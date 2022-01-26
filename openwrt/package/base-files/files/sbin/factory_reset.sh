#!/bin/sh

echo "FACTORY RESET" > /dev/console

# TODO: Implement BLUE LED for factory reset
# Turn LED blue to give user some feedback.

jffs2reset -y && reboot -f
