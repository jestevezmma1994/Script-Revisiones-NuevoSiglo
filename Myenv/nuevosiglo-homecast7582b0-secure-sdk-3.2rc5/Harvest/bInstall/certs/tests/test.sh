#!/bin/bash
#PWD=`pwd`
#echo "tutaj chmoduje do ciezkiej pizdy: $PWD/result"
chmod +w $PWD/result1 $PWD/result2 $PWD/result3 $PWD/result4 -R -f
rm -rf $PWD/result1 $PWD/result2 $PWD/result3 $PWD/result4

if [ x"$S_INSTALL_CERT_CRYPTO_VERSION" == x"" ]; then
    S_INSTALL_CERT_CRYPTO_VERSION=2
    echo NOT SET
fi

echo "Using S_INSTALL_CERT_CRYPTO_VERSION=[$S_INSTALL_CERT_CRYPTO_VERSION]"

export S_INSTALL_CERT_CRYPTO_VERSION

echo ""
echo "Test1"
S_MOCKUP_PREFIX=$PWD/result1 S_INSTALL_BSPEC=$PWD/bspec-new SRM_DESTDIR=$PWD/result1 S_INSTALL_CERT_LOCATION=PART make -C .. install
RES=$?
if [ "$RES" != 0 ]; then
    echo "Test failed with RESULT $RES"
    exit 1
else
    echo "*** RESULT $RES"
fi

echo ""
echo "Test2"
S_MOCKUP_PREFIX=$PWD/result2 S_INSTALL_BSPEC=$PWD/bspec-old SRM_DESTDIR=$PWD/result2 S_INSTALL_CERT_LOCATION=PART make -C .. install
RES=$?
if [ "$RES" != 0 ]; then
    echo "Test failed with RESULT $RES"
    exit 1
else
    echo "*** RESULT $RES"
fi

echo ""
echo "Test2-1"
S_MOCKUP_PREFIX=$PWD/result2-1 S_INSTALL_BSPEC=$PWD/bspec-old-v1 SRM_DESTDIR=$PWD/result2-1 S_INSTALL_CERT_LOCATION=PART S_INSTALL_CERT_CRYPTO_VERSION=1 make -C .. install
RES=$?
if [ "$RES" != 0 ]; then
    echo "Test failed with RESULT $RES"
    exit 1
else
    echo "*** RESULT $RES"
fi


echo ""
echo "Test3"
S_MOCKUP_PREFIX=$PWD/result3 S_INSTALL_BSPEC=$PWD/bspec-new SRM_DESTDIR=$PWD/result3 S_INSTALL_CERT_LOCATION=ROOTFS make -C .. install
RES=$?
if [ "$RES" != 0 ]; then
    echo "Test failed with RESULT $RES"
    exit 1
else
    echo "*** RESULT $RES"
fi

echo ""
echo "Test4"
S_MOCKUP_PREFIX=$PWD/result4 S_INSTALL_BSPEC=$PWD/bspec-old SRM_DESTDIR=$PWD/result4 S_INSTALL_CERT_LOCATION=ROOTFS make -C .. install
RES=$?
if [ "$RES" != 0 ]; then
    echo "Test failed with RESULT $RES"
    exit 1
else
    echo "*** RESULT $RES"
fi

