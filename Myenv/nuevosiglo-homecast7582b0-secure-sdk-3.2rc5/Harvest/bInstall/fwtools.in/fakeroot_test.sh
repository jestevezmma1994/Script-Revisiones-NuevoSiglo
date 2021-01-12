#!/bin/sh
export tmp_dir=$(mktemp -d)

ERROR_MSG="FAKEROOT TEST FAIL !\nBuildsystem requires a fakeroot which can handle both 32 and 64 bit processes properly.\nFor this reason fakeroot needs to be built from sources and installed manually.\nFor more information please contact with support@cubiware.com"

fakeroot_error() {
    echo -e $(tput setaf 1)$ERROR_MSG$(tput sgr0)
    exit 1
}

cat >$tmp_dir/check_file.sh <<EOF
\$1 \$2
if [ -c \$2 ]; then
    exit 0
else
    exit 1
fi
EOF

cat >$tmp_dir/main.c <<EOF
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    mknod(argv[1], 0755 | S_IFCHR, 0);
}
EOF

gcc $tmp_dir/main.c -o $tmp_dir/64bit
gcc $tmp_dir/main.c -o $tmp_dir/32bit -m32

if fakeroot sh $tmp_dir/check_file.sh $tmp_dir/64bit $tmp_dir/1; then
    echo "fakeroot 64-bit emulates mknod correctly"
else
    echo "fakeroot 64-bit is broken"
    fakeroot_error
fi

if fakeroot sh $tmp_dir/check_file.sh $tmp_dir/32bit $tmp_dir/2; then
    echo "fakeroot 32-bit emulates mknod correctly"
else
    echo "fakeroot 32-bit is broken"
    fakeroot_error
fi

cat >$tmp_dir/test.sh <<EOF
src_mode=640
mkdir -p $tmp_dir/src/some_directory
chmod \$src_mode $tmp_dir/src/some_directory
echo src mode \$src_mode
echo src dir mode \$(stat --format=%a $tmp_dir/src/some_directory)
echo umask is \$(umask)
cp -a $tmp_dir/src $tmp_dir/dest
echo dest dir mode \$(stat --format=%a $tmp_dir/dest/some_directory)
dest_mode=\$(stat --format=%a $tmp_dir/dest/some_directory)
[ \$dest_mode != \$src_mode ] && exit 1
exit 0
EOF

if fakeroot sh $tmp_dir/test.sh; then
    echo "fakeroot emulates permissions properly"
else
    echo "fakeroot is broken"
    fakeroot_error
fi
rm -rf $tmp_dir

