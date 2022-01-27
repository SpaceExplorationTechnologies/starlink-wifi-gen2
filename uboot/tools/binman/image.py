# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Class for an image, the output of binman
#

from collections import OrderedDict
import fnmatch
from operator import attrgetter
import os
import re
import sys

from binman.entry import Entry
from binman.etype import fdtmap
from binman.etype import image_header
from binman.etype import section
from dtoc import fdt
from dtoc import fdt_util
from patman import tools
from patman import tout

class Image(section.Entry_section):
    """A Image, representing an output from binman

    An image is comprised of a collection of entries each containing binary
    data. The image size must be large enough to hold all of this data.

    This class implements the various operations needed for images.

    Attributes:
        filename: Output filename for image
        image_node: Name of node containing the description for this image
        fdtmap_dtb: Fdt object for the fdtmap when loading from a file
        fdtmap_data: Contents of the fdtmap when loading from a file
        allow_repack: True to add properties to allow the image to be safely
            repacked later

    Args:
        copy_to_orig: Copy offset/size to orig_offset/orig_size after reading
            from the device tree
        test: True if this is being called from a test of Images. This this case
            there is no device tree defining the structure of the section, so
            we create a section manually.
        ignore_missing: Ignore any missing entry arguments (i.e. don't raise an
            exception). This should be used if the Image is being loaded from
            a file rather than generated. In that case we obviously don't need
            the entry arguments since the contents already exists.
    """
    def __init__(self, name, node, copy_to_orig=True, test=False,
                 ignore_missing=False):
        super().__init__(None, 'section', node, test=test)
        self.copy_to_orig = copy_to_orig
        self.name = 'main-section'
        self.image_name = name
        self._filename = '%s.bin' % self.image_name
        self.fdtmap_dtb = None
        self.fdtmap_data = None
        self.allow_repack = False
        self._ignore_missing = ignore_missing
        if not test:
            self.ReadNode()

    def ReadNode(self):
        super().ReadNode()
        filename = fdt_util.GetString(self._node, 'filename')
        if filename:
            self._filename = filename
        self.allow_repack = fdt_util.GetBool(self._node, 'allow-repack')

    @classmethod
    def FromFile(cls, fname):
        """Convert an image file into an Image for use in binman

        Args:
            fname: Filename of image file to read

        Returns:
            Image object on success

        Raises:
            ValueError if something goes wrong
        """
        data = tools.ReadFile(fname)
        size = len(data)

        # First look for an image header
        pos = image_header.LocateHeaderOffset(data)
        if pos is None:
            # Look for the FDT map
            pos = fdtmap.LocateFdtmap(data)
        if pos is None:
            raise ValueError('Cannot find FDT map in image')

        # We don't know the FDT size, so check its header first
        probe_dtb = fdt.Fdt.FromData(
            data[pos + fdtmap.FDTMAP_HDR_LEN:pos + 256])
        dtb_size = probe_dtb.GetFdtObj().totalsize()
        fdtmap_data = data[pos:pos + dtb_size + fdtmap.FDTMAP_HDR_LEN]
        fdt_data = fdtmap_data[fdtmap.FDTMAP_HDR_LEN:]
        out_fname = tools.GetOutputFilename('fdtmap.in.dtb')
        tools.WriteFile(out_fname, fdt_data)
        dtb = fdt.Fdt(out_fname)
        dtb.Scan()

        # Return an Image with the associated nodes
        root = dtb.GetRoot()
        image = Image('image', root, copy_to_orig=False, ignore_missing=True)

        image.image_node = fdt_util.GetString(root, 'image-node', 'image')
        image.fdtmap_dtb = dtb
        image.fdtmap_data = fdtmap_data
        image._data = data
        image._filename = fname
        image.image_name, _ = os.path.splitext(fname)
        return image

    def Raise(self, msg):
        """Convenience function to raise an error referencing an image"""
        raise ValueError("Image '%s': %s" % (self._node.path, msg))

    def PackEntries(self):
        """Pack all entries into the image"""
        super().Pack(0)

    def SetImagePos(self):
        # This first section in the image so it starts at 0
        super().SetImagePos(0)

    def ProcessEntryContents(self):
        """Call the ProcessContents() method for each entry

        This is intended to adjust the contents as needed by the entry type.

        Returns:
            True if the new data size is OK, False if expansion is needed
        """
        return super().ProcessContents()

    def WriteSymbols(self):
        """Write symbol values into binary files for access at run time"""
        super().WriteSymbols(self)

    def BuildImage(self):
        """Write the image to a file"""
        fname = tools.GetOutputFilename(self._filename)
        tout.Info("Writing image to '%s'" % fname)
        with open(fname, 'wb') as fd:
            data = self.GetPaddedData()
            fd.write(data)
        tout.Info("Wrote %#x bytes" % len(data))

    def WriteMap(self):
        """Write a map of the image to a .map file

        Returns:
            Filename of map file written
        """
        filename = '%s.map' % self.image_name
        fname = tools.GetOutputFilename(filename)
        with open(fname, 'w') as fd:
            print('%8s  %8s  %8s  %s' % ('ImagePos', 'Offset', 'Size', 'Name'),
                  file=fd)
            super().WriteMap(fd, 0)
        return fname

    def BuildEntryList(self):
        """List the files in an image

        Returns:
            List of entry.EntryInfo objects describing all entries in the image
        """
        entries = []
        self.ListEntries(entries, 0)
        return entries

    def FindEntryPath(self, entry_path):
        """Find an entry at a given path in the image

        Args:
            entry_path: Path to entry (e.g. /ro-section/u-boot')

        Returns:
            Entry object corresponding to that past

        Raises:
            ValueError if no entry found
        """
        parts = entry_path.split('/')
        entries = self.GetEntries()
        parent = '/'
        for part in parts:
            entry = entries.get(part)
            if not entry:
                raise ValueError("Entry '%s' not found in '%s'" %
                                 (part, parent))
            parent = entry.GetPath()
            entries = entry.GetEntries()
        return entry

    def ReadData(self, decomp=True):
        tout.Debug("Image '%s' ReadData(), size=%#x" %
                   (self.GetPath(), len(self._data)))
        return self._data

    def GetListEntries(self, entry_paths):
        """List the entries in an image

        This decodes the supplied image and returns a list of entries from that
        image, preceded by a header.

        Args:
            entry_paths: List of paths to match (each can have wildcards). Only
                entries whose names match one of these paths will be printed

        Returns:
            String error message if something went wrong, otherwise
            3-Tuple:
                List of EntryInfo objects
                List of lines, each
                    List of text columns, each a string
                List of widths of each column
        """
        def _EntryToStrings(entry):
            """Convert an entry to a list of strings, one for each column

            Args:
                entry: EntryInfo object containing information to output

            Returns:
                List of strings, one for each field in entry
            """
            def _AppendHex(val):
                """Append a hex value, or an empty string if val is None

                Args:
                    val: Integer value, or None if none
                """
                args.append('' if val is None else '>%x' % val)

            args = ['  ' * entry.indent + entry.name]
            _AppendHex(entry.image_pos)
            _AppendHex(entry.size)
            args.append(entry.etype)
            _AppendHex(entry.offset)
            _AppendHex(entry.uncomp_size)
            return args

        def _DoLine(lines, line):
            """Add a line to the output list

            This adds a line (a list of columns) to the output list. It also updates
            the widths[] array with the maximum width of each column

            Args:
                lines: List of lines to add to
                line: List of strings, one for each column
            """
            for i, item in enumerate(line):
                widths[i] = max(widths[i], len(item))
            lines.append(line)

        def _NameInPaths(fname, entry_paths):
            """Check if a filename is in a list of wildcarded paths

            Args:
                fname: Filename to check
                entry_paths: List of wildcarded paths (e.g. ['*dtb*', 'u-boot*',
                                                             'section/u-boot'])

            Returns:
                True if any wildcard matches the filename (using Unix filename
                    pattern matching, not regular expressions)
                False if not
            """
            for path in entry_paths:
                if fnmatch.fnmatch(fname, path):
                    return True
            return False

        entries = self.BuildEntryList()

        # This is our list of lines. Each item in the list is a list of strings, one
        # for each column
        lines = []
        HEADER = ['Name', 'Image-pos', 'Size', 'Entry-type', 'Offset',
                  'Uncomp-size']
        num_columns = len(HEADER)

        # This records the width of each column, calculated as the maximum width of
        # all the strings in that column
        widths = [0] * num_columns
        _DoLine(lines, HEADER)

        # We won't print anything unless it has at least this indent. So at the
        # start we will print nothing, unless a path matches (or there are no
        # entry paths)
        MAX_INDENT = 100
        min_indent = MAX_INDENT
        path_stack = []
        path = ''
        indent = 0
        selected_entries = []
        for entry in entries:
            if entry.indent > indent:
                path_stack.append(path)
            elif entry.indent < indent:
                path_stack.pop()
            if path_stack:
                path = path_stack[-1] + '/' + entry.name
            indent = entry.indent

            # If there are entry paths to match and we are not looking at a
            # sub-entry of a previously matched entry, we need to check the path
            if entry_paths and indent <= min_indent:
                if _NameInPaths(path[1:], entry_paths):
                    # Print this entry and all sub-entries (=higher indent)
                    min_indent = indent
                else:
                    # Don't print this entry, nor any following entries until we get
                    # a path match
                    min_indent = MAX_INDENT
                    continue
            _DoLine(lines, _EntryToStrings(entry))
            selected_entries.append(entry)
        return selected_entries, lines, widths

    def LookupImageSymbol(self, sym_name, optional, msg, base_addr):
        """Look up a symbol in an ELF file

        Looks up a symbol in an ELF file. Only entry types which come from an
        ELF image can be used by this function.

        This searches through this image including all of its subsections.

        At present the only entry properties supported are:
            offset
            image_pos - 'base_addr' is added if this is not an end-at-4gb image
            size

        Args:
            sym_name: Symbol name in the ELF file to look up in the format
                _binman_<entry>_prop_<property> where <entry> is the name of
                the entry and <property> is the property to find (e.g.
                _binman_u_boot_prop_offset). As a special case, you can append
                _any to <entry> to have it search for any matching entry. E.g.
                _binman_u_boot_any_prop_offset will match entries called u-boot,
                u-boot-img and u-boot-nodtb)
            optional: True if the symbol is optional. If False this function
                will raise if the symbol is not found
            msg: Message to display if an error occurs
            base_addr: Base address of image. This is added to the returned
                image_pos in most cases so that the returned position indicates
                where the targeted entry/binary has actually been loaded. But
                if end-at-4gb is used, this is not done, since the binary is
                already assumed to be linked to the ROM position and using
                execute-in-place (XIP).

        Returns:
            Value that should be assigned to that symbol, or None if it was
                optional and not found

        Raises:
            ValueError if the symbol is invalid or not found, or references a
                property which is not supported
        """
        entries = OrderedDict()
        entries_by_name = {}
        self._CollectEntries(entries, entries_by_name, self)
        return self.LookupSymbol(sym_name, optional, msg, base_addr,
                                 entries_by_name)
