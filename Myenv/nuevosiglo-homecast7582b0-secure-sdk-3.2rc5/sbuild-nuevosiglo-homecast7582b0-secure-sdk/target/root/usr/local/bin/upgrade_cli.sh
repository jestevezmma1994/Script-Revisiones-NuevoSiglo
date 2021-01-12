#!/bin/sh
trap "" 1 2
force=""
if [ "$1" = "--force" ]; then
echo -n "Do you really want to force remote upgrade [y/n]? "
read ans
if [ "$ans" != "y" ]; then
echo "Aborting upgrade."
exit 0
fi
echo "Forcing remote upgrade..."
force="--force"
elif [ "$1" = "--auto" ]; then
force="--auto"
elif [ "$1" != "" ]; then
echo "Unknown parameter '$1' found."
exit 1
fi
/usr/local/bin/upgrade $force
retval=$?
if [ "$retval" = "8" ]; then
echo "Rebooting STB."
/usr/local/bin/soft-reboot.sh
fi
exit 0
