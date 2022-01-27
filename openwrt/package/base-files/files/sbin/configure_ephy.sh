#!/bin/ash

# set Marvell PHY registers
mii_mgr -s -p 1 -r 22 -v 0x12   # Set page to 18, where config registers live per MRVL datasheet
mii_mgr -s -p 1 -r 20 -v 0x1    # Set page 18 reg 20 to 0x1, which sets to SGMII/Copper mode
mii_mgr -s -p 1 -r 20 -v 0x8001 # reset page 18
mii_mgr -s -p 1 -r 22 -v 0x1    # set the page back to 1
mii_mgr -s -p 1 -r 0 -v 0x9140  # reset page 1, keeping autonegotiation
mii_mgr -s -p 1 -r 22 -v 0x0    # set the page back to 0
sleep 3

# Reset Marvell PHY
mii_mgr -s -p 1 -r 22 -v 0x2    # set page to 2
mii_mgr -s -p 1 -r 16 -v 0x444a # disable 125MHz clock output to pin
mii_mgr -s -p 1 -r 22 -v 0x0    # set the page back to 0
mii_mgr -s -p 1 -r 0 -v 0x9140  # Reset the device
mii_mgr -s -p 1 -r 0 -v 0x1340  # Un-reset the device
