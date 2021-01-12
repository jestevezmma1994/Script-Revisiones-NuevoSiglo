#!/bin/sh

if [ "$1" = "--manual" ]; then
    if [ "$2" = "" ]; then
	echo -en "Please specify new date as the argument.\nDate should have the following format: YYYY.MM.DD-HH:MM:SS\n"
	exit 1
    fi
    date -s $2
    ret=$?
else
  if [ -f /etc/profile ]; then
      . /etc/profile
  fi

  if [ "$NTPSTATUS" = "disabled" -o "$NTP_STATUS" = "disabled" ]; then
      exit 0
  fi

  if [ "$NTPSERVER" ]; then
      NTP_SERVER=$NTPSERVER
  fi
  
  /usr/bin/ntpdate $NTP_SERVER
  ret=$?
  if [ $ret -eq 0 ]; then
    date +"%s" > /var/run/ntpsync
  fi
fi

if [ $ret -eq 0 -a -x /usr/local/bin/svhwclock ]; then
    /usr/local/bin/svhwclock --sys2hw
    ret=$?
fi
exit $ret
