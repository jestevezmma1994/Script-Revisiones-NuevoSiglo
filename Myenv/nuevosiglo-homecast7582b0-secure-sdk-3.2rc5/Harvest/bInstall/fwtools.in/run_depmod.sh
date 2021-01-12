#!/bin/bash

DEPMOD=`which depmod`

if [ -x ${SRM_ROOT}/host/local/usr/bin/depmod ]; then
    echo "Running built-in depmod."

    # Will need to link (temporarily) stapi modules to /lib/modules/$KERNELVER so they can be found by depmod..
    if [ -L ${SRM_DESTDIR}/modules ]; then
        ln -sfT  ../../../modules ${SRM_DESTDIR}/lib/modules/${KERNELVER}/stapi-modules
    fi

    OUTPUT=`${SRM_ROOT}/host/local/usr/bin/depmod  -b ${SRM_DESTDIR}/ ${KERNELVER} -e -F ${KERNELDIR}/System.map 2>&1`
    echo $OUTPUT | grep "unknown symbol" >/dev/null
    if [ "$?" == 0 ]; then
        echo "$OUTPUT"
        echo " ***"
        echo " *** Found unknown symbols in your modules. Inspect warnings from depmod above."
        echo " *** Common issue: you may be mixing 'debug' kernel modules with a 'secure' sbuild/kernel. This may happen e.g. for externally-delivered modules, like third-party QBConax*.ko"
        echo " ***"
        exit 1
    fi
else
    echo "Running system-wide depmod. Will not use -e flag which is a bad thing. Please make sure to update your base-system."
    PATH=$$PATH:/sbin depmod -b ${SRM_DESTDIR}/ ${KERNELVER}
fi
