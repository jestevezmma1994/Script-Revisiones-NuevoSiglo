. /bin/_upgrade_platform_spec.proc

version=$(cat /etc/VERSION)
rc=$(cat /etc/RC)
hash=$(_up_get_hash)

export QB_FIRMWARE_VERSION="$version $hash"
export QB_FIRMWARE_RC="$rc"
