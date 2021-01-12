#!/bin/sh
# this script is used to export values of configuration variables
# usage:
# . export_settings.sh [V1 [V2 [...]]]
#
# - if no arument is passed, all configuration variables will be exported
# - otherwise, only Vi variables (or Vi's sub-trees if Vi is a struct object)
#   will be exported

OUT=/tmp/conf.sh
rm -f $OUT
if [ "$1" != "" ]; then
    while [ "$1" != "" ]; do
	/usr/local/bin/svconf_ctl --export $1 >> $OUT
	shift
    done
else
    /usr/local/bin/svconf_ctl --export >> $OUT
fi
. $OUT
