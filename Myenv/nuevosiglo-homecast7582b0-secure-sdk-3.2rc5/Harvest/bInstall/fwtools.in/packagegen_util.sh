#!/bin/bash

print_header()
{
	echo -e "\x1b[32m$*\x1b[39m"
}

print_error()
{
	echo -e "\x1b[31mError: $*\x1b[39m"
}

print_info()
{
	echo -e "$*"
}

#
# Verifies if given signed image and nfsroot package match
#
# Parameters:
# $1 - nfsroot package name, must be in Cubiware name convention e.g. mmp-cp7584a1-fd-secure-cubitv-nfsroot-2.0rc8.tgz
# $2 - name of application image signed by Latens. Must be in Latens name convention. Acceptable is one of two Latens formats:
#      * <OEM_NAME>_<MODEL>_Application_<Type>_<Version>.signed e.g. Polsat_IPS3000MMP_Application_Production_1.6rc1.signed
#      * <OEM_NAME>_<MODEL>_Signed_<Type>_Application_<Version>.signed.bin e.g. Polsat_IPS3000MMP_Signed_FieldDebug_Application_1.6rc1.signed.bin
#
check_latens_package_version()
{
	PACKAGE_PATH=$1
	SIGNED_IMAGE_PATH=$2
	PACKAGE_NAME=$(basename $PACKAGE_PATH)
	SIGNED_IMAGE=$(basename $SIGNED_IMAGE_PATH)

	FORMAT1="^[[:alnum:]]+_[[:alnum:]]+_Application_[[:alnum:]]+_[[:alnum:]]+.*\.signed$"
	FORMAT2="^[[:alnum:]]+_[[:alnum:]]+_Signed_[[:alnum:]]+_Application_.*\.signed\.bin$"
	if [ -z "$(echo $SIGNED_IMAGE | grep -E $FORMAT1)" ] && [ -z "$(echo $SIGNED_IMAGE | grep -E $FORMAT2)" ]; then
		print_error "Wrong format of signed image. Should be: <OEM_NAME>_<MODEL>_Applicatione_<Type>_<Version>.signed or <OEM_NAME>_<MODEL>_Signed_<Type>_Application_<Version>.signed.bin"
		exit 1
	fi

	SECURE=0
	if [ -n "$(echo $PACKAGE_NAME | grep -- 'secure')" ]; then
		SECURE=1
	fi

	if [ -z "$(echo $PACKAGE_NAME | grep 'fd\|pd')" ]; then
		print_error "Wrong application built type. Only fd or pd images are allowed for Latens release!"
		exit 1
	fi

	TYPE="$(echo $PACKAGE_NAME | cut -d"-" -f3)"
	PACKAGE_VERSION="$(echo $PACKAGE_NAME | awk -F'nfsroot-' '{print $NF}' | sed 's/^\(.*\).tgz$/\1/')"
	if [ "$TYPE" = "fd" ]; then
		LATENS_APPTYPE=FieldDebug
	elif [ "$TYPE" = "pd" ]; then
		LATENS_APPTYPE=Production
	else
		print_error "Wrong image type = '$TYPE', only 'fd' and 'pd' are allowed"
		exit 1
	fi

	if [ -z "$(echo $SIGNED_IMAGE | grep $LATENS_APPTYPE)" ]; then
		print_error "Not matching Latens type of signed image, expected = $LATENS_APPTYPE"
		exit 1
	fi

	if [ $SECURE -eq 1 ] && [ -n "$(echo $SIGNED_IMAGE | grep "_debug")" ]; then
		print_error "Not matching application build, nfsroot package is secure but image is secure"
		exit 1
	fi

	if [ $SECURE -eq 0 ] && [ -z "$(echo $SIGNED_IMAGE | grep "_debug")" ]; then
		print_error "Not matching application build, nfsroot package is debug but image is secure"
		exit 1
	fi

	if [ "$(echo $SIGNED_IMAGE | cut -d'_' -f3)" = "Application" ]; then
		IMAGE_VERSION="$(echo $SIGNED_IMAGE | cut -d'_' -f5 | sed 's/^\(.*\).signed$/\1/' )"
	else
		IMAGE_VERSION="$(echo $SIGNED_IMAGE | cut -d'_' -f6 | sed 's/^\(.*\).signed.bin$/\1/' )"
	fi

	if [ "$IMAGE_VERSION" != "$PACKAGE_VERSION" ]; then
		print_error "Versions of signed image and nfsroot package differ. Using version from signed image for output package!"
	fi

	print_info "Latens image type = $TYPE, VERSION = $IMAGE_VERSION, SECURE = $SECURE"

}

