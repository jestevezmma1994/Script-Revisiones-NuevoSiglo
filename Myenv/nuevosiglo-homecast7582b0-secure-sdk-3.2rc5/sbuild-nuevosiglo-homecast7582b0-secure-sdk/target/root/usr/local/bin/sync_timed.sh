#!/bin/sh

# wait for network for a while
if [ ! -f /var/lock/network ]; then
    logger -t "sync_timed.sh" "Network seems to be not initialized yet. Waiting for network initialization"
    i=0
    while [ ! -f /var/lock/network ]; do
	i=$(($i + 1))
	if [ $i -gt 300 ]; then
	    break
	fi
	sleep 1
    done
    if [ -f /var/lock/network ]; then
	logger -t "sync_timed.sh" "Network is up!"
    fi
fi

while true; do
    unset NTP_POLL
    if [ -f /etc/profile ]; then
	. /etc/profile
    fi
    if [ "$LOG_LEVEL" = "" ]; then
	LOG_LEVEL=3
    fi
    if [ "$NTP_STATUS" = "disabled" -o "$NTPSTATUS" = "disabled" ]; then
	if [ "$LOG_LEVEL" -gt 1 ]; then
	    logger -t sync_timed.sh "NTP time synchronization is disabled. Giving up."
	fi
	exit 1
    fi

    if [ "$NTPSERVER" != "" ]; then
	NTP_SERVER=$NTPSERVER
    fi
    
    if [ "$NTP_SERVER" = "" ]; then
	if [ "$LOG_LEVEL" -gt 1 ]; then
	    if [ "$warned" != "yes" ]; then
		logger -t sync_timed.sh "NTP server is not specified, aborting time synchronization"
	    fi
	fi
    else
	if [ "$LOG_LEVEL" -gt 1 ]; then
	    logger -t sync_timed.sh "Synchronizing time with $NTP_SERVER"
	    /usr/local/bin/sync_time.sh 2>&1 | logger -t sync_timed.sh
	else
	    /usr/local/bin/sync_time.sh 2>&1 >/dev/null
	fi
    fi
    if [ "$NTP_POLL" = "" ]; then NTP_POLL=1140; fi
    if [ ! -f /var/run/ntpsync ]; then
	if [ "$warned" != "yes" ]; then
 	  logger -t sync_timed.sh "Warning: time is not synchronized with NTP server yet!"
	  warned=yes
	fi
	sl=1
    else
	sl=$(($NTP_POLL * 60))
	warned=no
    fi
    if [ "$LOG_LEVEL" -gt 1 ]; then
	if [ "$warned" != "yes" ]; then
    	    logger -t sync_timed.sh "Next time synchronization in $sl seconds"
	fi
    fi
    sleep $sl
done
