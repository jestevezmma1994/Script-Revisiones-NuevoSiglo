#!/bin/sh

###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
# use only plaform specific functions for your implementation #
###############################################################

[ -f /var/lock/upgrade_testing ] && exit 1
touch /var/lock/upgrade_testing

do_job () {
. /bin/_upgrade_platform_spec.proc
[ -z "$_UP_PLATFORM_NAME" ] && { echo "upable to load platform specific procedures!"; exit 101; }

UPGRADE_STATUS=`_up_get_status`;

if [ "$UPGRADE_STATUS" != "OK" ]; then
    sleep _UPGRADE_CONFIRMATION_DELAY_
    _up_set_status "OK"
    # Create marker with upgrade status for upgrade pop-up
    echo -n "OK" > /etc/vod/upgrade.stat
fi
touch /var/lock/upgrade_ok

rm -f /var/lock/upgrade_testing
if [ -f /etc/vod/nfsroot ]; then
    rm /etc/vod/nfsroot
    /etc/init.d/config save
fi
}

case "$1" in
	start) do_job &
		;;
	stop)
		;;
	*)
		echo "Usage: $0 [start|stop]"
		exit 255
		;;
esac
exit 0
