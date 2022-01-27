#!/bin/sh
# register hook with hwnat driver.

wifi down
rmmod mt_whnat.ko
rmmod hw_nat.ko
rmmod wifi_forward
rmmod mt_wifi
insmod mt_wifi
insmod wifi_forward
wifi up
