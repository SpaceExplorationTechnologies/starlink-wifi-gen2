#!/usr/bin/python3

import csv
import getopt
import shutil
import sys

helpmsg = """\
apply_eeprom_changes.py -i (--infile) <input-Factory-bin>
                        -o (--outfile) <output-Factory-bin>
                        -c (--changefile) <input-changes-csv>
                        -f (--offsetcol) <offset-col#>
                        -v (--valuecol) <value-col#>

All flags are required. Offset column and value column should be 0-indexed.

This program creates an output file, copies the input Factory binary into
it, and writes the appropriate changes from the changes CSV on top. The
input file is not edited. The first row of the CSV is skipped. All offsets
and values in the CSV must be in hex (with or without the leading '0x').

So, the CSV should be formatted as:

        Row 0:  <ignored header row>
        Row 1:  ..., <offset in hex>, ..., <value in hex>
        Row 2:  ..., <offset in hex>, ..., <value in hex>
        ...

where the <offset in hex> is at the 0-indexed column number offsetcol,
and the <value in hex> is at the 0-indexed column number valuecol.
"""

def main(argv):
    try:
        opts, _ = getopt.getopt(argv, "hi:o:c:f:v:",
            ["infile=", "outfile=", "changefile=", "offsetcol=", "valuecol="])
    except getopt.GetoptError:
        print(helpmsg)
        sys.exit()
    
    infile = ""
    outfile = ""
    changefile = ""
    offsetcol = 0
    valuecol = 0

    for opt, arg in opts:
        if opt == '-h':
            print(helpmsg)
            sys.exit()
        elif opt in ("-i, --infile"):
            infile = arg
        elif opt in ("-o, --outfile"):
            outfile = arg
        elif opt in ("-c, --changefile"):
            changefile = arg
        elif opt in ("-f, --offsetcol"):
            offsetcol = int(arg)
        elif opt in ("-v, --valuecol"):
            valuecol = int(arg)

    print(f"Applying {changefile} onto {infile}, outputting to {outfile}...")

    infileobj = open(infile, "rb")
    outfileobj = open(outfile, "wb")
    shutil.copyfileobj(infileobj, outfileobj)

    with open(changefile, newline='') as eeprom_file:
        eeprom_reader = csv.reader(eeprom_file)
        next(eeprom_reader) # Skip header row
        for row in eeprom_reader:
            offset_hex = row[offsetcol]
            value_hex = row[valuecol]
            offset_int = int(offset_hex, 16)
            value_int = int(value_hex, 16)
            outfileobj.seek(offset_int)
            outfileobj.write(bytes([value_int]))

    infileobj.close()
    outfileobj.close()

    print("Done.")


if __name__ == "__main__":
    main(sys.argv[1:])