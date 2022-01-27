#!/bin/sh

# register hook with hwnat driver.
wifi down
insmod /lib/modules/ralink/hw_nat.ko
modprobe mt_whnat.ko
wifi up
