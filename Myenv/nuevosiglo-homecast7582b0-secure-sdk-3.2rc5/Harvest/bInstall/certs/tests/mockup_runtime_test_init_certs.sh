#!/bin/sh

S_INSTALL_CERT_CRYPTO_VERSION=2

if [ "$S_INSTALL_CERT_CRYPTO_VERSION" == "2" ]; then
    S_MOCKUP_PREFIX=$PWD/result1
    PRIMARY_REFERENCE_CERT=delivery/cert.pem
    PRIMARY_REFERENCE_KEY=delivery/key.pem
else
    S_MOCKUP_PREFIX=$PWD/result2-1
    PRIMARY_REFERENCE_CERT=delivery/cert-v1.pem
    PRIMARY_REFERENCE_KEY=delivery/key-v1.pem
    USE_DMCRYPT=no
    S_INSTALL_CERT_CRYPTO_VERSION=1
fi

export S_MOCKUP_PREFIX
export S_INSTALL_CERT_CRYPTO_VERSION

GLOBAL_ERROR=0

prepare_dirs()
{
    echo "Preparing dirs"

    if [ -d $S_MOCKUP_PREFIX ]; then
        chmod +w $S_MOCKUP_PREFIX -R
        rm -rf $S_MOCKUP_PREFIX
    fi
    echo "Done preparing dirs"

    ./test.sh >/dev/null

    mkdir -p $S_MOCKUP_PREFIX/tmp
    mkdir -p $S_MOCKUP_PREFIX/dev

    mkdir -p $S_MOCKUP_PREFIX/proc
    cp delivery/fake_proc_mtd $S_MOCKUP_PREFIX/proc/mtd

    # Simulate the presence of unsquashfs to not use dmcrypt
    if [ $USE_DMCRYPT != "yes" ]; then
        echo "Using unsquashfs"
        mkdir -p $S_MOCKUP_PREFIX/usr/bin
        touch $S_MOCKUP_PREFIX/usr/bin/unsquashfs
        chmod +x $S_MOCKUP_PREFIX/usr/bin/unsquashfs
    fi

    # V1
    if [ "$S_INSTALL_CERT_CRYPTO_VERSION" == "1" ]; then
        cp delivery/000000000000.certs $S_MOCKUP_PREFIX/dev/mtdblock10
        cp delivery/000000000000.sign $S_MOCKUP_PREFIX/dev/mtdblock11

        mkdir -p $S_MOCKUP_PREFIX/sys/block/mtdblock10/
        echo 128 > $S_MOCKUP_PREFIX/sys/block/mtdblock10/size
    else
        # V2
        cp delivery/D74000000000003.certs $S_MOCKUP_PREFIX/dev/mtd10
        mkdir -p $S_MOCKUP_PREFIX/sys/class/mtd/mtd10
        echo 65536 > $S_MOCKUP_PREFIX/sys/class/mtd/mtd10/size
    fi

    sed -i -e "s#chown \(.*\):\(.*\) $S_MOCKUP_PREFIX#chown $USER:$USER $S_MOCKUP_PREFIX#g" $S_MOCKUP_PREFIX/etc/init.d/init_certs
}

print_result()
{
    res=$1
    expected=$2
    if [ "$expected" == "$res" ]; then
        echo "SUCCESS"
    else
        echo "ERROR: expected $expected and got $res"
        GLOBAL_ERROR=1
    fi
}

run_init()
{
    expected=$1
    /bin/sh $S_MOCKUP_PREFIX/etc/init.d/init_certs start
    print_result $? $expected
}

check_primary()
{
    # /etc/ssl/private points to /tmp/ssl/private
    cmp $S_MOCKUP_PREFIX/tmp/ssl/private/cert.pem $PRIMARY_REFERENCE_CERT
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi
 
    cmp $S_MOCKUP_PREFIX/tmp/ssl/private/key.pem $PRIMARY_REFERENCE_KEY
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi

    cmp $S_MOCKUP_PREFIX/etc/ssl/private/cert.pem $PRIMARY_REFERENCE_CERT
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi

    cmp $S_MOCKUP_PREFIX/etc/ssl/private/key.pem $PRIMARY_REFERENCE_KEY
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi

    # openssl shall verify those certs

    return 0
}

assert_primary()
{
    check_primary
    RET=$?
    if [ "$RET" != 0 ]; then
        echo "[PRIMARY] File test failed! Inspect errors above"
        GLOBAL_ERROR=1
        return 1
    else
        echo "[PRIMARY] File test OK"
        return 0
    fi
}

check_backup()
{
    # /etc/ssl/private points to /tmp/ssl/private
    cmp $S_MOCKUP_PREFIX/tmp/ssl/private/cert.pem $S_MOCKUP_PREFIX/etc/ssl/backup/cert.pem
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi
 
    cmp $S_MOCKUP_PREFIX/tmp/ssl/private/key.pem $S_MOCKUP_PREFIX/etc/ssl/backup/key.pem
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi

    cmp $S_MOCKUP_PREFIX/etc/ssl/private/cert.pem $S_MOCKUP_PREFIX/etc/ssl/backup/cert.pem
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi

    cmp $S_MOCKUP_PREFIX/etc/ssl/private/key.pem $S_MOCKUP_PREFIX/etc/ssl/backup/key.pem
    RET=$?
    if [ "$RET" != 0 ]; then
        return $RET
    fi
}

assert_backup()
{
    check_backup
    RET=$?
    if [ "$RET" != 0 ]; then
        echo "[BACKUP] File test failed! Inspect errors above"
        GLOBAL_ERROR=1
        return 1
    else
        echo "[BACKUP] File test OK"
        return 0
    fi
}

assert_stopped()
{
    if [ -d $S_MOCKUP_PREFIX/tmp/ssl ]; then
        echo "Certificate system not stopped properly!"
        GLOBAL_ERROR=1
        return 1
    else
        echo "Certificate system stopped OK"
    fi
}

start_test()
{
    echo ""
    echo ">>>>>>>"
    echo ">>>" $*
    echo ">>>>>>>"
}

do_exit()
{
    if [ "$GLOBAL_ERROR" != 0 ]; then
        echo "******** TEST RESULT: " FAILED
    else
        echo "******** TEST RESULT: " SUCCESS
    fi
    exit 0

}
# TODO: 
# check exit conditions, if everything checks out, perhaps run openssl in the same manner
# as stb will do

##############################
## Positive test, shall work
##############################
start_test "Positive, just start the certs"
prepare_dirs
run_init 0
assert_primary

start_test "Start again w/o stopping in the middle"
prepare_dirs
run_init 0
assert_primary

start_test "Negative test, remove something (do the full execution tree testing for all the cases)"
/bin/sh $S_MOCKUP_PREFIX/etc/init.d/init_certs stop
print_result $? 0
assert_stopped

start_test "Stop for the second time, should be success as well"
/bin/sh $S_MOCKUP_PREFIX/etc/init.d/init_certs stop
print_result $? 0
assert_stopped

start_test "Positive after stopped"
prepare_dirs
run_init 0
assert_primary

start_test "Remove /dev/mtd completely"
prepare_dirs
rm $S_MOCKUP_PREFIX/proc/mtd
run_init 4
assert_backup

start_test "Modify /dev/mtd so it is missing cert partition"
prepare_dirs
sed -i $S_MOCKUP_PREFIX/proc/mtd -e "s#cert#fakefake#g#"
run_init 4
assert_backup

start_test "Missing /dev/mtd10 partition"
prepare_dirs
rm $S_MOCKUP_PREFIX/dev/mtd10
rm $S_MOCKUP_PREFIX/dev/mtdblock10
run_init 4
assert_backup

start_test "Messed up /dev/mtd10 partition"
prepare_dirs
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1k count=5
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtdblock10 bs=1k count=5
run_init 4
assert_backup

start_test "Missing $S_MOCKUP_PREFIX/sys/class/mtd/mtd$CERT_DEVNUM/size"
prepare_dirs
rm $S_MOCKUP_PREFIX/sys/class/mtd/mtd10/size
rm $S_MOCKUP_PREFIX/sys/block/mtdblock10/size
run_init 4
assert_backup

start_test "Missing $S_MOCKUP_PREFIX/sys/class/mtd/mtd$CERT_DEVNUM/size and missing dev/mtd10 partition "
prepare_dirs
rm $S_MOCKUP_PREFIX/sys/class/mtd/mtd10/size
rm $S_MOCKUP_PREFIX/dev/mtd10
rm $S_MOCKUP_PREFIX/sys/block/mtdblock10/size
rm $S_MOCKUP_PREFIX/dev/mtdblock10
run_init 4
assert_backup

if [ "$S_INSTALL_CERT_CRYPTO_VERSION" == "1" ]; then
    start_test "Mess up signature: first 256 bytes"
    prepare_dirs
    dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtdblock11 bs=1 count=1 seek=0
    run_init 4
    assert_backup

    start_test "Mess up data, starts at 608"
    prepare_dirs
    dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtdblock10 bs=1 count=1 seek=608
    run_init 4
    assert_backup

    start_test "Truncate /dev/mt10 partition to 256-1 bytes"
    prepare_dirs
    dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtdblock10 bs=1 count=255
    run_init 4
    assert_backup
else
start_test "Mess up sign_header: first 256 bytes"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=1 seek=0
run_init 4
assert_backup

start_test "Mess up sign_data: first 256 bytes"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=1 seek=256
run_init 4
assert_backup

start_test "Mess up header: first 512 up to 576 bytes"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=1 seek=512
run_init 4
assert_backup

start_test "Mess up iv: 576 up tp 608"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=1 seek=576
run_init 4
assert_backup

start_test "Mess up data, starts at 608"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=1 seek=608
run_init 4
assert_backup

start_test "Truncate /dev/mt10 partition to 256-1 bytes"
prepare_dirs
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=255
run_init 4
assert_backup

start_test "Truncate /dev/mt10 partition to 512-1 bytes"
prepare_dirs
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=511
run_init 4
assert_backup

start_test "Truncate /dev/mt10 partition to 576-1 bytes"
prepare_dirs
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=575
run_init 4
assert_backup

start_test "Truncate /dev/mt10 partition to 608-1 bytes"
prepare_dirs
dd if=/dev/zero of=$S_MOCKUP_PREFIX/dev/mtd10 bs=1 count=607
run_init 4
assert_backup
fi

start_test "Mess up transport key"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/etc/ssl/transport/certs.key bs=1 count=1
run_init 4
assert_backup

start_test "Mess up RSA key"
prepare_dirs
dd conv=notrunc if=/dev/zero of=$S_MOCKUP_PREFIX/etc/ssl/transport/rsa.pub bs=1 count=256
run_init 4
assert_backup

start_test "Use incorrect RSA key"
prepare_dirs
cp delivery/fake_rsa.pub $S_MOCKUP_PREFIX/etc/ssl/transport/rsa.pub
run_init 4
assert_backup

do_exit

