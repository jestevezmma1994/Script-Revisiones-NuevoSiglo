#!/bin/sh

. boot_config_tools.sh

if [ "`get_boot_config | grep "^u.bootstat" | cut -d'=' -f2`" != "\"OK\"" ]; then 
    set_boot_config u.bootstat "\"OK\""
    touch /var/lock/upgrade_ok
fi

rm -f /var/lock/upgrade_testing
