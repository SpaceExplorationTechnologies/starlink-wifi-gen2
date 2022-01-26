# Factory Calibration

## apply\_eeprom\_changes.py

This script creates copies of the factory calibration binary with provided changes
to the binary file. These changes are provided in a CSV containing offsets and
the values those offsets should be set to. Use `python3 apply\_eeprom\_changes.py --help`
for more information. This file should be copied to `openwrt/package/base-files/lib/firmware/e2p`
where the driver will find the file and use it at runtime.
