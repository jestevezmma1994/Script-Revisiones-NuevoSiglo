#!/bin/sh

. /etc/profile

killall -STOP gui_start >/dev/null 2>&1
killall -9 stella >/dev/null 2>&1
killall -9 lstv >/dev/null 2>&1
cd /tmp

# kill off remote connections
trap "" 1 2
( 
  killall dropbear >/dev/null 2>&1 
  killall telnetd >/dev/null 2>&1
  killall in.telnetd >/dev/null 2>&1
  /etc/init.d/bluez stop >/dev/null 2>&1
  umount -a >/dev/null 2>&1
  hdparm -f /dev/hda >/dev/null 2>&1
  usleep 100000
  sync
  reboot -f
) &

sleep 5
sync
reboot -f

