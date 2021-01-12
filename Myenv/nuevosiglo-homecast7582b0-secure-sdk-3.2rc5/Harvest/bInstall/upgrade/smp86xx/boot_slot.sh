#!/bin/sh

. boot_config_tools.sh

# getting slot number
SLOT=`get_boot_config | grep "^u.bootslnr=" | tail -n 1 | cut -d'=' -f2-`
[ -z "$SLOT" ] && exit 2;
exit $SLOT
