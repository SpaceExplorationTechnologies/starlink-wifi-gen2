import os
import sys
import re
import getopt


topdir = ""
linuxdir = ""
quilt = "/usr/bin/quilt"
verbose = False # more log

def execute(cmd, ignore_error=False):
	if verbose: print(cmd)
	ret = os.system(cmd)
	if not ignore_error and 0 != ret:
		raise Exception("\n{head}\n{cmd}\nCommands failed with error {ret}!\n{tail}".format(
			head="="*64, cmd=";\n".join(cmd.split(";")), ret=ret, tail="="*64))


def patch_delete(patch):
	cmd = """cd {linuxdir};{quilt} delete {patch};""".format(
		linuxdir=linuxdir, quilt=quilt, patch=patch)
	execute(cmd)

def patch_clear():

	if verbose: print("delete patches from quilt")

	cmd = """cd {linuxdir};{quilt} pop -a;""".format(
		linuxdir=linuxdir, quilt=quilt)
	execute(cmd, True)

	series = os.path.join(linuxdir, "patches/series")
	if os.path.exists(series):
		with open(series) as fd:
			for patch in reversed(fd.read().split()):
				if re.match(r"(mt76\w{2,3}|mt_wifi)/.+patch", patch):
					patch_delete(patch)

	cmd = """cd {linuxdir};{quilt} push -a;""".format(
		linuxdir=linuxdir, quilt=quilt)
	execute(cmd, True)

	if verbose: print("remove patch folders")
	driversdir = os.path.join(topdir, "package", "mtk", "drivers")
	drivers = os.listdir(driversdir)
	for driver in drivers:
		if not re.match(r"mt76.{2,3}|mt_wifi", driver): continue
		cmd = """cd {linuxdir}; rm -rf {patchpath};""".format(
			linuxdir=linuxdir,
			patchpath=os.path.join("patches", driver))
		execute(cmd)

def patch_apply(driver):
	patchdir = os.path.join(topdir, "package", "mtk", "drivers", driver, "patches")
	if verbose: print(patchdir)
	if not os.path.exists(patchdir) or not os.path.isdir(patchdir):
		print("{0} not exists or is not dir!".format(patchdir))
		return

	cmd = "mkdir -p {0}".format(os.path.join(linuxdir,"patches",driver))
	execute(cmd)

	for patch in sorted(os.listdir(patchdir)):
		# check if a patch is already applied
		_target = os.path.join(linuxdir,"patches",driver, patch)
		if os.path.exists(_target):
			print("{0} alread applied".format(os.path.join(driver, patch)))
			continue
		patch_fullpath = os.path.join(patchdir, patch)
		if verbose: print(patch_fullpath)
		if os.path.isfile(patch_fullpath) and re.match(r".+\.patch", patch):
			cmd = "cd {linuxdir};{quilt} import {patch}".format(
				linuxdir=linuxdir, quilt=quilt, patch=patch_fullpath)
			execute(cmd)
			cmd="cd {linuxdir};{quilt} next".format(linuxdir=linuxdir, quilt=quilt)
			fp = os.popen(cmd)
			oldpath = fp.read().strip()
			fp.close()
			print("oldpath", oldpath)
			try:
				cmd = "cd {linuxdir};{quilt} push".format(
					linuxdir=linuxdir, quilt=quilt)
				execute(cmd)
			except Exception as e:
				# to terminate the script
				raise e
			finally:
				cmd = "cd {linuxdir};{quilt} rename -P {oldpath} {newpath}".format(
					linuxdir=linuxdir, quilt=quilt,
					oldpath=oldpath,
					newpath=os.path.join(driver, patch))
				execute(cmd)


def init_drivers_from_gerrit():
	drivers_install_scripts = {}
	drivers_install_scripts["mt_wifi"] = """
		echo "install source code";
		cp -rf {TOPDIR}/../ko_module/wlan_driver/jedi/bin {LINUX_DIR}/drivers/net/wireless/mediatek/;
		cp -rf {TOPDIR}/../ko_module/wlan_driver/jedi/wifi_driver {LINUX_DIR}/drivers/net/wireless/mediatek/;
		cp -rf {TOPDIR}/../ko_module/wlan_driver/jedi/wlan_service {LINUX_DIR}/drivers/net/wireless/mediatek/;
		cp -rf {TOPDIR}/../ko_module/wlan_driver/jedi/wifi_driver/embedded/tools/auto_build_kernel_4_4.sh {LINUX_DIR}/drivers/net/wireless/mediatek/;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mediatek/wifi_driver/.git;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mediatek/wlan_service/.git;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mediatek/bin/.git;
		echo "install makefile & kconfig";
		cd {LINUX_DIR}/drivers/net/wireless/mediatek && sh auto_build_kernel_4_4.sh AP;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)
	'''
	drivers_install_scripts["mt7663"] = """
		echo "prepare mt7663";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7663/ {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/.git;
		echo "install makefile & kconfig";
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_ap;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/os/linux/Makefile.mt_wifi_ap {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_ap/Makefile;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/os/linux/Kconfig.mt_wifi_ap {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_ap/Kconfig;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/os/linux/Kconfig.mt_wifi {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/embedded/Kconfig;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_sta;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/os/linux/Makefile.mt_wifi_sta {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_sta/Makefile;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi/os/linux/Kconfig.mt_wifi_sta {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/mt_wifi_sta/Kconfig;
		echo "update firmware && eeprom binary";
		cp -rf {TOPDIR}/../wifi/bin/ {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/bin;
		cp -rf {TOPDIR}/package/mtk/drivers/mt7663/auto_build_update_fw.sh {LINUX_DIR}/drivers/net/wireless/mtk/mt7663/auto_build_update_fw.sh;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7663 && sh auto_build_update_fw.sh;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt7615"] = """
		echo "prepare mt7615";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7615/ {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/.git;
		echo "install makefile & kconfig";
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_ap;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/os/linux/Makefile.mt_wifi_ap {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_ap/Makefile;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/os/linux/Kconfig.mt_wifi_ap {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_ap/Kconfig;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/os/linux/Kconfig.mt_wifi {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/embedded/Kconfig;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_sta;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/os/linux/Makefile.mt_wifi_sta {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_sta/Makefile;
		cp -rfp {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi/os/linux/Kconfig.mt_wifi_sta {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/mt_wifi_sta/Kconfig;
		echo "update firmware && eeprom binary";
		cp -rf {TOPDIR}/../wifi/bin/ {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/bin;
		cp -rf {TOPDIR}/package/mtk/drivers/mt7615/auto_build_update_fw.sh {LINUX_DIR}/drivers/net/wireless/mtk/mt7615/auto_build_update_fw.sh;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7615 && sh auto_build_update_fw.sh;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt7603"] = """
		echo "prepare mt7603";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7603/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7603/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7603/* {LINUX_DIR}/drivers/net/wireless/mtk/mt7603/;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7603 && sh mt7603*_driver_local_build.sh;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7603/.git;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt76x2"] = """
		echo "prepare mt76x2";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt76x2/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt76x2/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt76x2/* {LINUX_DIR}/drivers/net/wireless/mtk/mt76x2/;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt76x2 && sh mt76x2*_driver_local_build.sh;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt76x2/.git;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt7610"] = """
		echo "prepare mt7610";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7610/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7610/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7610/* {LINUX_DIR}/drivers/net/wireless/mtk/mt7610/;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7610 && sh mt7610*_driver_local_build.sh;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7610/.git;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt7628"] = """
		echo "prepare mt7628";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7628/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7628/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7628/* {LINUX_DIR}/drivers/net/wireless/mtk/mt7628/;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7628 && sh mt7628*_driver_local_build.sh;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7628/.git;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)

	drivers_install_scripts["mt7620"] = """
		echo "prepare mt7620";
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7620/;
		mkdir -p {LINUX_DIR}/drivers/net/wireless/mtk/mt7620/;
		echo "install source code";
		cp -rf {TOPDIR}/../wifi/mt7620/* {LINUX_DIR}/drivers/net/wireless/mtk/mt7620/;
		cd {LINUX_DIR}/drivers/net/wireless/mtk/mt7620 && sh mt7620*_driver_local_build.sh;
		rm -rf {LINUX_DIR}/drivers/net/wireless/mtk/mt7620/.git;
		""".format(LINUX_DIR=linuxdir,TOPDIR=topdir)
	'''
	return drivers_install_scripts

def init_drivers_from_tarball(driver,pkgsrc):
	print("\n*****init_driver_from_tarball!!*****")
	'''
	cmd = """echo "prepare {driver}";
		rm -rf {linuxdir}/drivers/net/wireless/mtk/{driver};
		mkdir -p {linuxdir}/drivers/net/wireless/mtk/{driver}""".format(
		linuxdir=linuxdir, driver=driver)
	execute(cmd)
	'''
	if re.search(".gz",pkgsrc):
		tarcmd = "-zxf"
	elif re.search(".bz2",pkgsrc):
		tarcmd = "-jxf"
	elif re.search(".xz",pkgsrc):
		tarcmd = "-Jxf"
	else:
		assert False, "File format not supported!"
	'''
	cmd = """tar {tarcmd} {topdir}/dl/{pkgsrc} -C {destdir}""".format(
		tarcmd=tarcmd, topdir=topdir , pkgsrc=pkgsrc ,
		destdir=os.path.join(linuxdir,"drivers","net","wireless","mtk",))
	'''
	cmd = """tar {tarcmd} {topdir}/dl/{pkgsrc} -C {destdir}""".format(
	tarcmd=tarcmd, topdir=topdir , pkgsrc=pkgsrc ,
	destdir=os.path.join(linuxdir,"drivers","net","wireless","mediatek",))
	execute(cmd)
        copy_sh = """cp -rf {destdir}/mt_wifi/embedded/tools/release_build_in_4.4.sh {destdir};
                cd {destdir} && sh release_build_in_4.4.sh AP""".format(
                destdir= os.path.join(linuxdir,"drivers","net","wireless","mediatek",))
        execute(copy_sh)
	patch_apply(driver)

def init_drivers():
	file = os.path.join(topdir, "target/linux/ramips/Makefile")
	mtk_gerrit = False
	with open(file) as fd:
		for line in reversed(fd.read().split()):
			if re.match("MTK_RELEASE_VERSION:=trunk",line): #mtk release flag
				mtk_gerrit = True
				break

	driversdir = os.path.join(topdir, "package", "mtk", "drivers")
	drivers = os.listdir(driversdir)
	for driver in drivers:
		makefile = os.path.join(driversdir, driver, "Makefile")
		if not re.match(r"mt76.{2,3}|mt_wifi", driver) or not os.path.exists(makefile):
			print ("match fail" ,driver)
			continue
		found = 0
		with open(makefile) as fd:
			for pkgsrc in reversed(fd.read().split()):
				result = re.match(r"PKG_SOURCE:=(.*)", pkgsrc) #mtk driver tarball path
				if result:
					found = 1
					pkgsrc = result.group(1)
					break
		if mtk_gerrit:
			#driver tarball not exist and not release, init driver from gerrit
			driver_scripts = init_drivers_from_gerrit()
			for mtkdriver, script in driver_scripts.items():
				if driver == mtkdriver:
					print("\n*****init_driver_from_gerrit!!*****")
					execute(script)
					patch_apply(driver)
					break
		else:
			if found == 1 and pkgsrc != "":
				#driver source tarball path exist, init driver from tarball
				init_drivers_from_tarball(driver,pkgsrc)

def clean():
	patch_clear()

def prepare():
	init_drivers()


def usage():
	print("help info!")


def main():
	global linuxdir
	global topdir
	global quilt
	global verbose

	try:
		opts, args = getopt.getopt(sys.argv[1:],
			None, ["help", "verbose", "linuxdir=", "topdir=", "quilt="])
	except getopt.GetoptError as e:
		print(e)
		usage()
		sys.exit(2)

	for k, v in opts:
		if k in ("-v", "--verbose"):
			verbose = True
		elif k in ("-h", "--help"):
			usage()
			sys.exit()
		elif k in ("-l", "--linuxdir"):
			linuxdir = os.path.abspath(v) if v else ""
		elif k in ("-t", "--topdir"):
			topdir = os.path.abspath(v) if v else ""
		elif k in ("-q", "--quilt"):
			quilt = v
		else:
			assert False, "unhandled option"

	if os.path.exists(topdir) and not os.path.exists(quilt):
		quilt = topdir+'/staging_dir/host/bin/quilt'

	assert os.path.exists(quilt), "quilt not specified! ({0})".format(quilt)
	assert os.path.exists(linuxdir), "linux dir not exists! ({0})".format(linuxdir)
	assert os.path.exists(topdir), "topdir not exists! ({0})".format(topdir)

	for arg in args:
		if arg == "clean":
			clean()
		elif arg == "prepare":
			prepare()
		elif arg == "patch_apply":
			patch_apply()
		elif arg == "patch_clear":
			patch_clear()
		else:
			print("ignore unsupported cmd {0}".format(arg))


if __name__ == "__main__":
	main()
