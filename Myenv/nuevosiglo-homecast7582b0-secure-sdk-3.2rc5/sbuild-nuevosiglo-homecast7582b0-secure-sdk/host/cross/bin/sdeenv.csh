set _opath = ($path)
set path = (/usr/bin /bin $path)

alias mdi 'eval `sh /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/bin/mdi.sh csh \!*`'

if ( -d /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/etc ) then
  set _ononomatch = $?nonomatch
  set nonomatch
  foreach _mdi (/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/etc/*.mdi)
    if ( -r $_mdi ) then
      mdi addfrag $_mdi
    endif
  end
  if ( ! $_ononomatch) unset nonomatch
  unset _ononomatch _mdi
endif

if ( ! $?MDI_LIB && -f /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/etc/default.mdi ) then
  mdi qswitch default
endif

if ($path[1] != "/usr/bin") then
    set path = ($path[1] /home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/bin $_opath)
else
    set path = (/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/bin $_opath)
endif
unset _opath 

if ( $?MANPATH ) then
  setenv MANPATH "${MANPATH}:/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/man"
else
  setenv MANPATH ":/home/lupus/rc/b1190/branch-1190-RC/sbuild-qb-bcm97552-toolchain/host/cross/man"
endif
