PATH=/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/bin:$PATH
export PATH

MANPATH="${MANPATH}:/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/man"
export MANPATH

mdi () { eval `/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/bin/mdi.sh sh "$@"`; }
for _mdi in `ls /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/etc/*.mdi 2>/dev/null`; do
  mdi addfrag $_mdi
done
unset _mdi

if test "x$MDI_LIB" = "x" -a -f /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/etc/default.mdi; then
  mdi qswitch default
fi
