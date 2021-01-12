#!/bin/sh

UPGRADE_MARKER=/etc/vod/http-upgrade-available
PID_FILE=/var/run/upgrade_monitor.pid
CHECK_PERIOD=600 #10m

check_upgrade_monitor_status() {
    STATUS=0
    if [ -e "$PID_FILE" ]; then
        PID=$(cat $PID_FILE)
        if [ -n "$PID" ]; then
            STATUS=1
        fi
    fi
}

start_upgrade_monitor() {
    check_upgrade_monitor_status
    if [ $STATUS -ne 0 ]; then
        exit 0
    fi
    if [ -e $UPGRADE_MARKER ]; then
        exit 0 #we already marked a pending upgrade
    fi

    #do this in background
    {
        RANDOM=$(( 0x`xxd -l 4 -p /dev/urandom` )) #returns random value
        RANDOM_SLEEP=$(( $RANDOM % $CHECK_PERIOD )) #this has to be in a separate evaluation block
        sleep $RANDOM_SLEEP #sleep 0..CHECK_PERIOD seconds before first check for load balancing purposes

        while true; do
            upgrade --check-only --auto 1> /dev/null 2>&1
            if [ $? -eq 6 ]; then
                touch $UPGRADE_MARKER
                exit 0
            fi
            sleep $CHECK_PERIOD
        done
    } &
    echo $! > $PID_FILE
}

stop_upgrade_monitor() {
    check_upgrade_monitor_status
    if [ $STATUS -eq 0 ]; then
        exit 0
    fi

    kill $(cat $PID_FILE)
    rm $PID_FILE
}

case "$1" in
    start)
        start_upgrade_monitor
    ;;
    stop)
        stop_upgrade_monitor
    ;;
    restart)
        stop_upgrade_monitor
        start_upgrade_monitor
    ;;
    *)
        echo "Usage: $0 [start|stop]"
        exit 1
        ;;
esac
