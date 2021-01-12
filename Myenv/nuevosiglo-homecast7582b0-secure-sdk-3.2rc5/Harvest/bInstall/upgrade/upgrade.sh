#!/bin/sh

###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
# use only plaform specific functions for your implementation #
###############################################################

# exit codes:
# 0 - upgrade not needed
# 1 - download failed
# 2 - upgrade is locked
# 3 - downgrade rejected
# 4 - package verification failed
# 6 - new version present (if only checking for new version)
# 8 - done
# 101 - platform specific procedures are not found

# exit codes affect the behavior of exit_upgrade() !!
#========================
# 0. prepare environment
#========================
if [ -x /usr/bin/logger ]; then
        LOGGER="/usr/bin/logger -s -p user.notice -t upgrade.sh "
else
        LOGGER=echo
fi
WORK_DIR=/tmp/UPGRADE
LOCK_FILE=/var/lock/upgrade
if which curl > /dev/null; then
	WGET="curl -L -f -v -O -g --speed-time 20 --speed-limit 1 --retry 5 --retry-delay 10 --retry-max-time _UPGRADE_RETRY_TIMEOUT_ --connect-timeout _UPGRADE_CONNECT_TIMEOUT_"
	WGET_STDOUT="curl -L -f -g --speed-time 20 --speed-limit 1 --retry 5 --retry-delay 10 --retry-max-time _UPGRADE_RETRY_TIMEOUT_ --connect-timeout _UPGRADE_CONNECT_TIMEOUT_"
	if [ ! -z $USERAGENT ]; then
		WGET="$WGET --user-agent $USERAGENT"
		WGET_STDOUT="$WGET_STDOUT --user-agent $USERAGENT"
	fi
else
	WGET="wget"
	WGET_STDOUT="wget -O -"
fi
if ! which checksign > /dev/null; then
	#use default dummy checksign if not found
checksign() {
	cat $2 > /dev/null
}
fi

# exit procedures
find_bairns_of() { #find all background proces
    for i in $@; do
        b=`pgrep -P $i`;
        [ -z "$b" ] && continue
        echo $b
        find_bairns_of $b
    done
}
kill_bairns_of() { #fill all background process
    pids=`find_bairns_of $$`
    [ -z "$pids" ] && return
    kill $pids 2>/dev/null
    kill $pids 2>/dev/null
    sleep 1
    kill -9 $pids 2>/dev/null
}
exit_upgrade()
{
    [ ! -z "$1" ] && RETVAL=$1 || RETVAL=-1
    [ ! -z "$2" ] && $LOGGER "E:$RETVAL: $2"
    if [ "$AUTO" != "1" ] && [ $RETVAL -ne 0 ] && [ $RETVAL -ne 6 ] && [ $RETVAL -ne 8 ]; then
        $LOGGER "!!!!! ABORTING UPGRADE !!!!!"
        /etc/init.d/config save
    fi
    kill_bairns_of $$
    # what now? reboot? not really... restart gui?
    if [ "$INITIATED" = "1" ]; then
        killall progress >/dev/null 2>&1
        killall blink > /dev/null 2>&1
        if [ -f /etc/init.d/codec ]; then
            /etc/init.d/codec restart > /dev/null 2>&1
        fi
        if [ "$WITH_GUI" != 1 ]; then
            if [ -e /usr/local/bin/start_gui ]; then
                /usr/local/bin/start_gui  > /dev/null 2>&1
            fi
        else
            #running in GUI context
            echo "GUI still running"
        fi
    fi
    # if RETVAL is "locked", it's locked for a reason.
    if [ $RETVAL -ne 2 ] && [ $RETVAL -ne 8 ]; then
        rm -f $FW_PACK_NAME $FW_PACK_NAME.md5 $FW_PACK_NAME.sig 2>/dev/null
        rm -f /var/run/upgrading 2>/dev/null
        rm -rf $WORK_DIR 2>/dev/null

        rm -f $LOCK_FILE
    fi
    exit $RETVAL
}

#parameters
PROGRESS_FILE=/var/run/progress
while [ ! -z "$1" ]; do
	[ "$1" = "--force" ] && FORCE=1
	[ "$1" = "--auto" ] && AUTO=1
	[ "$1" = "--with-gui" ] && WITH_GUI=1
	[ "$1" = "--check-only" ] && CHECK_ONLY=1
	if [ "$1" = "--progress-file" -a ! -z "$2" ]; then
		shift
		PROGRESS_FILE="$1"
	fi
	if [ "$1" = "--upgrade-server" -a ! -z "$2" ]; then
		shift
		FORCED_UPGRADE_SERVER="$1"
	fi
	shift
done
#dont run me twice
$LOGGER "I:0000: trying to lock..."
[ -e $LOCK_FILE ] && exit_upgrade 2 "upgrade is already locked!"
touch $LOCK_FILE

#wait for firmware affirmation
while [ ! -f /var/lock/upgrade_ok ]; do
	#file created by upgrade_ok.sh
	[ "$WAITING_AFFIRMATION" != "yes" ] && $LOGGER "I:0000: waiting for previos f/w upgrade affirmation"
	WAITING_AFFIRMATION=yes;
	sleep 3
done
[ "$WAITING_AFFIRMATION" = "yes" ] && WAITING_AFFIRMATION=no;
#import configuration settings
[ -e /etc/profile ] && . /etc/profile
SRM_PROJECT=`cat /etc/SRM_PROJECT`
[ ! -z "$SRM_PROJECT" ] && UPGRADE_SERVER="$UPGRADE_SERVER/$SRM_PROJECT"
[ ! -z "$FORCED_UPGRADE_SERVER" ] && UPGRADE_SERVER="$FORCED_UPGRADE_SERVER"
URI_SCHEME="$(echo $UPGRADE_SERVER | cut -d: -f1)"

#set upgrade over HTTPS if needed
if echo $UPGRADE_SERVER | grep -i "https://" >/dev/null; then
    if which curl > /dev/null; then
        WGET="$WGET --cacert /etc/cacert.pem --cert /etc/cert.pem --key /etc/certkey.pem"
        WGET_STDOUT="$WGET_STDOUT --cacert /etc/cacert.pem --cert /etc/cert.pem --key /etc/certkey.pem"
    else
        WGET="$WGET --ca-certificate /etc/cacert.pem"
        WGET_STDOUT="$WGET_STDOUT --ca-certificate /etc/cacert.pem"
    fi
fi

#add MAC address as custom HTTP header
if echo $UPGRADE_SERVER | grep -i -E "http(s)?://" >/dev/null; then
    if [ -f /sys/class/net/eth0/address ]; then
        MAC_ADDRESS=$(cat /sys/class/net/eth0/address)
        if which curl > /dev/null; then
            WGET="$WGET --header X-Client-MAC-Address:$MAC_ADDRESS"
            WGET_STDOUT="$WGET_STDOUT --header X-Client-MAC-Address:$MAC_ADDRESS"
        else
            WGET="$WGET --header=X-Client-MAC-Address:$MAC_ADDRESS"
            WGET_STDOUT="$WGET_STDOUT --header=X-Client-MAC-Address:$MAC_ADDRESS"
        fi
    fi
fi

#import strings
[ -e /etc/strings.txt ] && . /etc/strings.txt
#import platform specific procedures
[ -z "$UPGRADE_PLATFORM_SPEC" ] && UPGRADE_PLATFORM_SPEC=_upgrade_platform_spec.proc
. /bin/$UPGRADE_PLATFORM_SPEC
[ -z "$_UP_PLATFORM_NAME" ] && exit_upgrade 101 "unable to load platform specific procedures!"
# export proxy settings
if [ "$NETWORK_PROXY_STATUS" = "enabled" -a "$NETWORK_PROXY_ADDRESS" != "" ]; then
	if [ ! -z "$NETWORK_PROXY_USER" ]; then
		u="$NETWORK_PROXY_USER"
		[ ! -z "$NETWORK_PROXY_PASS" ] && u="$u:$NETWORK_PROXY_PASS";
		u="$u@"
	fi
	[ "$NETWORK_PROXY_PORT" != "" ] && p=":$NETWORK_PROXY_PORT"
	export http_proxy=http://$u$NETWORK_PROXY_ADDRESS$p
	WGET="$WGET -x $http_proxy"
	WGET_STDOUT="$WGET_STDOUT -x $http_proxy"
fi
DEFAULT_ARCH=SRM_SV_ARCH
DEFAULT_IMG=SRM_SV_IMG
DEFAULT_PROJECT=SRM_SV_PROJECT
for i in ARCH IMG PROJECT; do
	if [ -e /etc/vod/$i ]; then export $i=`cat /etc/vod/$i`;
	elif [ -e /etc/$i ]; then export $i=`cat /etc/$i`;
	else export $i=DEFAULT_$i; fi
done
#FW:
FW_NAME=$ARCH-$IMG-$PROJECT
FW_PACK_NAME=$FW_NAME.tgz
#initialize storage:
rm -rf $WORK_DIR
mkdir -p $WORK_DIR
cd $WORK_DIR
# internal procedures
cnt=0
step_progress()
{
	echo $cnt $* >$PROGRESS_FILE
	cnt=$(($cnt+1))
	#echo "P:"$(( 100 * $cnt / $STEP_CNT ))
}
track_download_progress()
{
	while read line; do
		progress=$(echo $line | cut -d: -f2)
		if [ "x_${URI_SCHEME}" = "x_file" ]; then
			echo -e "$((cnt-1)) writing\n$progress" >$PROGRESS_FILE
		else
			echo -e "$((cnt-1)) download\n$progress" >$PROGRESS_FILE
		fi
		echo "$line" >&2
	done
}
#=========================
# 1. check remote version
#=========================
$LOGGER "I:0200: downloading $UPGRADE_SERVER/$FW_PACK_NAME.md5"
_up_before_version_check
$WGET $UPGRADE_SERVER/$FW_PACK_NAME.md5 > wget.log 2>&1
[ "$?" != "0" -o -z  "`cat $FW_PACK_NAME.md5`" ] && exit_upgrade 1 "download failed: `cat wget.log`"
$LOGGER "I:0800: version verification"
VERSION=`cat $FW_PACK_NAME.md5 | grep 'RELEASE' | cut -d' ' -f2`
OLD_VERSION=`cat /etc/VERSION`
HASH=`grep -v "^#" $FW_PACK_NAME.md5 | cut -d' ' -f1 | head -n 1`
OLD_HASH=`_up_get_hash`
if [ "$HASH" = "$OLD_HASH" ]; then
	if [ "$FORCE" = "1" ]; then
		$LOGGER "I:0810: upgrade is not needed, forced"
	else
		exit_upgrade 0 "not needed"
	fi
fi
ver_to_num() {
# conversion from version to number:
# version is x-x'.y-y'.z-z'
# x<<50 + x'<<40 + y<<30 + y'<<20 + z<<10 + z'
	if [ "$1" = "CURRENT" ]; then
		echo 0
	else
		V="$1.";VD=0;
		while [ ! -z "$V" ]; do
			N=`echo "$V" | cut -d'.' -f1`"-0"
			NV=`echo $N | cut -d'-' -f1`
			NP=`echo $N | cut -d'-' -f2`
			V=`echo "$V" | cut -d'.' -f2-`
			VD=$(( ( $VD * 1024 * 1024 ) + ( $NV * 1024 + $NP ) ))
		done
	echo $VD;
	fi
}
if [ "$VERSION" != "CURRENT" -a "$OLD_VERSION" != "CURRENT" ]; then
	#check against downgrade
	D_VER=`ver_to_num "$VERSION"`
	D_OLD_VER=`ver_to_num "$OLD_VERSION"`
	if [ "$D_VER" != "0" -a "$D_OLD_VER" != "0" -a "$D_OLD_VER" -gt "$D_VER" ]; then
		if [ "$FORCE" = "1" ]; then
			$LOGGER "I:08E0: downgrade forced"
		else
			exit_upgrade 3 "downgrade rejected"
		fi
	fi
fi
_up_check_version $VERSION
rm -f *.log
$LOGGER "I:08F0: new release version is $VERSION"
_up_after_version_check
$LOGGER "I:08FF: done"
if [ "$CHECK_ONLY" = 1 ]; then
	exit_upgrade 6 "server has new version"
fi

#=====================
# 2. initiate upgrade
#=====================
$LOGGER "I:0001: creating file /var/run/upgrading"
_up_before_initiate_upgrade
trap "" 1
INITIATED=1
touch /var/run/upgrading
echo -n "UPGRADE_IN_PROGRESS" > /etc/vod/upgrade.stat
export PREVIOUS_VERSION=`cat /etc/VERSION`
export FIRMWARE_VERSION=$VERSION
STEP_CNT=3
[ ! -z "$_UP_PLATFORM_STEP_CNT" ] && STEP_CNT=$(( $STEP_CNT + $_UP_PLATFORM_STEP_CNT ))
if [ "$WITH_GUI" != 1 ]; then
	#no GUI running, call progress (GUI)
	stop_gui >/dev/null 2>&1
	stop_gui >/dev/null 2>&1
	if [ -z "$LANG" ] || [ "$LANG" = "POSIX" ]; then
		# set language for progress GUI
		export LANG=es_MX.UTF-8
		export LC_ALL=es_MX.UTF-8
	else
		export LC_ALL=$LANG
	fi
	progress >/dev/null 2>&1 &
else
	#running in GUI context
	$LOGGER "GUI still running"
fi
blink >/dev/null 2>&1 &
_up_after_initiate_upgrade
$LOGGER "I:00FF: done"


#==========================
# 3. getting upgrade files
#==========================
$LOGGER "I:0100: getting FIRMWARE from $UPGRADE_SERVER/$FW_PACK_NAME"
if [ "x_${URI_SCHEME}" = "x_file" ]; then
	step_progress "writing"
else
	step_progress "download"
fi
_up_before_getting_firmware
_up_before_writting_firmware
rm -f *.? *.log #cleanup
#prepare for secure upgrade, get supplier certificate
if [ "_SECURE_UPGRADE_" = "yes" ]; then
	# get public key
	$LOGGER "I:0300: downloading public key certificate, $UPGRADE_SERVER/server.cert.der... "
	$WGET_STDOUT $UPGRADE_SERVER/$FW_NAME.cert.der 2> wget.log > supplier.cert.der || \
	$WGET_STDOUT $UPGRADE_SERVER/server.cert.der 2> wget.log > supplier.cert.der || {
		exit_upgrade 1 "download $UPGRADE_SERVER/server.cert.der failed (tried both $FW_PACK_NAME.cert.der and server.cert.der): "`cat wget.log`
	}
	$LOGGER "I:03FF: done"
	#get signature
	$LOGGER "I:0201: downloading firmware signature, $UPGRADE_SERVER/$FW_PACK_NAME.sig... "
	$WGET $UPGRADE_SERVER/$FW_PACK_NAME.sig > wget.log 2>&1 || {
		exit_upgrade 1 "download $UPGRADE_SERVER/$FW_PACK_NAME.sig failed: "`cat wget.log`
	}
	$LOGGER "I:02FF: done"
fi
if [ "_USE_IMAGE_SIZES_" = "yes" ]; then
$LOGGER "I:0400: downloading image sizes, $UPGRADE_SERVER/$FW_NAME.size... "
$WGET_STDOUT $UPGRADE_SERVER/$FW_NAME.size 2> wget.log >image.sizes || exit_upgrade 1 "download $UPGRADE_SERVER/$FW_NAME.size failed: "`cat wget.log`
$LOGGER "I:04FF: done"
fi
_up_before_main_fifo_create
#unpack job moved to platform specific
TAR_FIFOS="$_UP_PLATFORM_TAR_PIPES md5 sig" #fifos for tar stream (gunzippped)
mkfifo tee_fifo $FW_PACK_NAME $TAR_FIFOS
#run secure verification (if needed)
if [ "_SECURE_UPGRADE_" = "yes" ]; then
	WAIT_FOR="verification gunzip" #add verification and gunzip task to list
    # stream original tgz to verification pipe named $FW_PACK_NAME
    # gunzip original tgz and stream it in pipes $TAR_FIFOS
	{ tee $FW_PACK_NAME <tee_fifo | \
        { gunzip -c -; echo $? > gunzip.?;} | tee $TAR_FIFOS >/dev/null; \
        	echo $? > tee.?;}&
    # run check sign process
	{ checksign _SECURE_SCRATCH_ $FW_PACK_NAME >/dev/null 2>&1; \
    	echo $? > verification.?;}&
else
	WAIT_FOR="gunzip" #add gunzip task to waitting list
    # gunzip original tgz and stream it in pipes $TAR_FIFOS
	{ cat tee_fifo | \
	    { gunzip -c -; echo $? >gunzip.?;} | tee $TAR_FIFOS >/dev/null; echo $? > tee.?;}&
fi
#extend list of waiting processes with download and md5 sum file extracting
WAIT_FOR="$WAIT_FOR $TAR_FIFOS download"
#run task writting upgrade files to flash
_up_write_firmware $HASH &
#run task extracting signature files for write verification
{ cat sig | tar x $FW_NAME.sig 2>/dev/null; \
    echo 0 >  sig.?;}&
#run task extracting md5 files for write verification
{ cat md5 | tar x $FW_NAME.md5; \
    echo $? >  md5.?;}&
#feed on pipe with source (download task)
#do not redirect stderr, the progress will be printed on it
{ $WGET_STDOUT $UPGRADE_SERVER/$FW_PACK_NAME 2>&1 >tee_fifo | tr '\r' '\n' | track_download_progress; \
    echo $? > download.?;}&
#JOIN TASKS (background jobs) HERE
while [ ! -z "$WAIT_FOR" ]; do #while waiting for something
	sleep 1 && sync #be nice and sync
	for i in $WAIT_FOR; do #for each waiting task
		if [ -e "$i.?" ]; then #check is result exists
			if [ "`cat $i.?`" = "0" ]; then #check result
                #result is OK (no error, success)
				# do not wait for the $i job anymore
				WAIT_FOR=`echo $WAIT_FOR | sed "s/\<$i\>//"`
			else
                #result is error
				ret=102
				[ "$i" = "verification" ] && ret=4
				[ "$i" = "download" ] && ret=1
				[ "$i" = "unpack" ] && ret=4
                #exit with error here (all background processes will be killed
				exit_upgrade $ret "$i failed: "`cat $i.?`
			fi
		fi
	done
done
# all waiting tasks are complete
rm -f *.? *.log
_up_after_writting_firmware
_up_after_getting_firmware
$LOGGER "I:01FF: done"

#=================================
# x. verifying firmware signature
#=================================
# moved to download procedure

#=======================
# x. unpacking firmware
#=======================
# moved to download procedure

#===============================
# x. verifying downloaded files
#===============================
$LOGGER "I:0B00: verication of in-package files"
step_progress "verify"
_up_before_firmware_verification
# generic verification expects all files for md5sum exists or waiting in pipes from flash devices
for i in  $_UP_PLATFORM_TAR_FILES; do
	[ ! -e "$i" ] && continue
	grep "\b$i\b" $FW_NAME.md5 | md5sum -c >/dev/null 2>&1
	[ "$?" != "0" -o -z "`cat $FW_NAME.md5`" ] && exit_upgrade 4 "MD5-sum of $i does not match!"
done
#platform specific verification
_up_verify_firmware
_up_after_firmware_verification
$LOGGER "I:0BFF: done"

#======================
# x. writting firmware
#======================
# moved to download section

#=============
# 6. complete
#=============
$LOGGER "I:0A00: completing and cleanup"
step_progress "done"
_up_before_complete
_up_confirm_firmware #this confirms upgrade but changing bootcfg
/etc/init.d/config save
#do not remove $LOCK_FILE here! upgrade is succeeded and the STB should be reboot now
_up_after_complete
$LOGGER "I:0AFF: done"

exit_upgrade 8 "done"
