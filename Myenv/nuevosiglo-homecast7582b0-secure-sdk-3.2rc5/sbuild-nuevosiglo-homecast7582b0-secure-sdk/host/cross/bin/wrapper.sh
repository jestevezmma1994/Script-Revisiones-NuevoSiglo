#!/bin/sh
if [ -z $COMPILER_PATH ]; then
    echo WARNING: `pwd`: <CMD> $@ diverted to cross-toolchain >> ${SRM_ROOT}/cross.warnings
fi
exec ${SRM_BUILD_DIR}/bin/cross-<CMD> "$@"