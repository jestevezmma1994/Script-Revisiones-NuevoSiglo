#!/bin/sh

while sleep 1; do 
	echo -n 1 > /dev/gpio27
	sleep 1
	echo -n 0 > /dev/gpio27
done

exit 0
