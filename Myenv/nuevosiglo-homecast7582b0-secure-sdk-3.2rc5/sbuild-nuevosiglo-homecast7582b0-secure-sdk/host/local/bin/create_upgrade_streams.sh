#!/bin/sh

if [ -z "$1" ] && [ -z "$SRM_ROOT" ]; then
    echo "Usage: $(basename "$0") sbuild_path"
    exit 0
fi

#UPGRADE_CATALOG_SBUILD_PATH - internal relative path to upgrade catalog in sbuild directory
UPGRADE_CATALOG_SBUILD_PATH=host/local/usr/local/share/upgrade_catalog

if [ -z "$SRM_ROOT" ]; then
    #Use provided path to sbuild directory
    $1/$UPGRADE_CATALOG_SBUILD_PATH/create_upgrade_streams.py $1
else
    #SRM_ROOT has been set up by silo shell
    #Use srm-host-run for correct library path
    srm-host-run $SRM_ROOT/$UPGRADE_CATALOG_SBUILD_PATH/create_upgrade_streams.py $SRM_ROOT
fi

