#!/bin/sh

ERROR_MSG="UMASK TEST FAIL !\nBuildsystem requires the umask to be set to u=rwx,g=rwx,o=rx (0002 octal) or u=rwx,g=rx,o=rx (0022 octal).\nOther umask values are not supported and might cause broken permission in firmware images.\nFor more information please contact with support@cubiware.com"

umask_error() {
    echo -e $(tput setaf 1)$ERROR_MSG$(tput sgr0)
    exit 1
}

if [ "x$(umask -S)" = "xu=rwx,g=rwx,o=rx" ] || [ "x$(umask -S)" = "xu=rwx,g=rx,o=rx" ]; then
    echo "umask set properly"
else
    echo "umask is wrong"
    umask_error
fi
rm -rf $tmp_dir

