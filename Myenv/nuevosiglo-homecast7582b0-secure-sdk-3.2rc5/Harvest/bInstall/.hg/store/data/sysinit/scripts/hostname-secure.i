         m   l       �������������U�0�)ԭ���lʵ�<            u#!/bin/sh

HOSTNAME=`ifconfig eth0 | grep HW | sed -e 's/.*HWaddr \([0-9A-F:]*\).*/\1/'`
hostname $HOSTNAME
