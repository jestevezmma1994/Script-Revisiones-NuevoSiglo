#! /bin/sh

# Where this file, and the other downloaded files live
bindir="`dirname $0`"

nw=`echo ${bindir} | wc -w`
if test $nw -gt 1; then
    echo "Cannot install in directory with space in name, please move files."
    exit 1
fi

# Convert relative name to absolute
case "${bindir}" in
/*)	;;
*)	bindir="`pwd`/${bindir}"
	;;
esac

target="`dirname ${bindir}`"

# Name of product
product="MIPS SDE"
version=5.03

echo ""
echo "${product} ${version} Installation"
echo ""

UNAME_MACHINE=`(uname -m) 2>/dev/null` || UNAME_MACHINE=unknown
UNAME_RELEASE=`(uname -r) 2>/dev/null` || UNAME_RELEASE=unknown
UNAME_SYSTEM=`(uname -s) 2>/dev/null` || UNAME_SYSTEM=unknown
UNAME_VERSION=`(uname -v) 2>/dev/null` || UNAME_VERSION=unknown

msimid=
case "${UNAME_MACHINE}:${UNAME_SYSTEM}:${UNAME_RELEASE}:${UNAME_VERSION}" in
    i?86:BSD/386:*:* | *:BSD/OS:*:*)
	needmaj=1 needmin=1 needpatch=
	system=bsdi ;;
    i?86:Linux:*:* | x86_64:Linux:*:**:*)
	system=linux
	needmaj=2 needmin=2 needpatch=
	msimid=L ;;
    i?86:CYGWIN*:*:*)
	system=win32 
	needmaj=1 needmin=5 needpatch=3
	msimid=M
	# Backspace is not default erase character - fix it for
	# the duration of this script.
	STTY=`stty -g`
	trap "stty $STTY; exit" 0 1 2 15
	stty erase ^h
	;;
    sun4*:SunOS:5*:* | tadpole*:SunOS:5*:*)
	system=sparc 
	needmaj=5 needmin=6 needpatch=
	msimid=S ;;
    #sun*:*:4.2BSD:*)
    #	system=sparc ;;
    9000/[678]??:HP-UX:*:*)
	needmaj=10 needmin=20 needpatch=
	system=hpux ;;
    *)
	echo "Unable to guess system type for:"
        echo " ${UNAME_MACHINE}:${UNAME_SYSTEM}:${UNAME_RELEASE}:${UNAME_VERSION}"
	exit 1 ;;
esac

# Work out how not do an echo without a final line-feed
if test "`/bin/echo 'foo\c'`" = 'foo\c'; then
    ECHON="/bin/echo -n"
    ECHOE=""
else
    ECHON="/bin/echo"
    ECHOE='\c'
fi
GZIP=""

# Take OS release name, strip off leading non-digits, trailing
# non-digit or '.', and extract the major, minor and patch number.
eval `echo ${UNAME_RELEASE} | \
      sed -e 's/^[^0-9][^0-9]*//' -e 's/[^.0-9].*$//' | \
      awk -F. '{printf "maj=%s min=%s patch=%s",$1,$2,$3}'`

fail=
needrel=$needmaj.$needmin
test $maj -lt $needmaj && fail=y
test -z "$fail" -a $min -lt $needmin && fail=y
if test -n "$needpatch"; then
    needrel=$needrel.$needpatch
    test -z "$fail" -a $patch -lt $needpatch && fail=y
fi
 
if test -n "$fail"; then
    echo "Your $UNAME_SYSTEM kernel version $UNAME_RELEASE is too old,"
    echo "you need at least version ${needrel} to run"
    echo "${product} ${version}."
    echo ""
    echo "Please upgrade your system and then rerun this script."
    exit 1
fi

if test "$system" = "linux"; then
    # Make sure that Linux has libc.so.6 installed
    if test -x /sbin/ldconfig; then
	/sbin/ldconfig -p | fgrep libc.so.6 >/dev/null 2>&1
	if test $? -ne 0; then
	    echo "Version 6 of the Linux C library (libc6) is not"
	    echo "installed - please install libc6 first and then"
	    echo "rerun this installation script."
	    exit 1
	fi
	/sbin/ldconfig -p | fgrep libncurses.so.5 >/dev/null 2>&1
	if test $? -ne 0; then
	    echo "Version 5 of the ncurses library (libncurses.so.5) is not"
	    echo "installed - please install the ncurses4 package and then"
	    echo "rerun this installation script."
	    echo ""
	    echo "You should be able to download this from your Linux"
	    echo "provider\'s packages collection."
	    exit 1
	fi
    else
	echo "I cannot find the /sbin/ldconfig program on this system,"
        echo "so this version of Linux is probably way too old to be able"
	echo "to run ${product} ${version}."
	echo ""
	echo "Please upgrade to a more modern Linux with ELF support"
	echo "and libc6, and then try again."
	exit 1
    fi
fi

for var in GCC_EXEC_PREFIX LIBRARY_PATH COMPILER_PATH; do
    x=`eval echo \$\{${var}:+set\}`
    if test "x$x" = "xset"; then
      val=`eval echo \$\{${var}\}`
      echo "The ${var} environment variable is already defined"
      echo "as ${var}=\"${val}\"."
      echo ""
      echo "Which means that this installation may conflict with another"
      echo "copy of GCC or anold SDE."
      echo ""
      ${ECHON} "Do you want to stop and fix this now? (y/n) [y] ${ECHOE}"
      read check
      case "${check}" in
      [nN]*) ;;
      *) 	exit 1 ;;
      esac
      break
    fi
done

echo ""
echo ""
echo ""
echo "If you have already installed MIPSsim, you can tell SDE how"
echo "to find it, by generating an \"MDI fragment\". You can do"
echo "this for each version of MIPSsim you have installed."

flag=1
a="a"

while test ${flag} -ne 0; do
    echo ""
    echo ""
    echo ""
    ${ECHON} "Do you want to create $a MIPSsim MDI fragment? (y/n) [n] ${ECHOE}"
    read check
    case "${check}" in
    [yY]*)
	sh ${target}/bin/gen-mdifrag.sh mipssim ;;
    *) 	
	flag=0 ;;
    esac
    a="another"
    MIPSARCHROOT=
done

if test "$system" = "win32"; then
    echo ""
    echo ""
    echo ""
    echo ""
    echo ""
    echo ""
    echo ""
    echo "If you have already installed an EJTAG probe with an MDI interface"
    echo "you can now generate an \"MDI fragment\" for it."
    echo ""
    echo "For the FS2 probe, for example, you will need to enter only the"
    echo "name of the FS2 DLL \"fs2mips.dll\"."
    echo ""

    ${ECHON} "Do you want to create a basic probe fragment? (y/n) [n] ${ECHOE}"
    read check
    case "${check}" in
    [yY]*)
	sh ${target}/bin/gen-mdifrag.sh cutdown ;;
    *) 	
	flag=0 ;;
    esac
fi


echo ""
echo ""
echo ""
echo "Creating ${product} environment"
echo ""

cat <<EOF 					>$target/bin/sdeenv.sh
PATH=${target}/bin:\$PATH
export PATH

MANPATH="\${MANPATH}:${target}/man"
export MANPATH

mdi () { eval \`${target}/bin/mdi.sh sh "\$@"\`; }
for _mdi in \`ls ${target}/etc/*.mdi 2>/dev/null\`; do
  mdi addfrag \$_mdi
done
unset _mdi

if test "x\$MDI_LIB" = "x" -a -f ${target}/etc/default.mdi; then
  mdi qswitch default
fi
EOF

cat <<EOF 					>$target/bin/sdeenv.csh
set _opath = (\$path)
set path = (/usr/bin /bin \$path)

alias mdi 'eval \`sh ${target}/bin/mdi.sh csh \\!*\`'

if ( -d ${target}/etc ) then
  set _ononomatch = \$?nonomatch
  set nonomatch
  foreach _mdi (${target}/etc/*.mdi)
    if ( -r \$_mdi ) then
      mdi addfrag \$_mdi
    endif
  end
  if ( ! \$_ononomatch) unset nonomatch
  unset _ononomatch _mdi
endif

if ( ! \$?MDI_LIB && -f ${target}/etc/default.mdi ) then
  mdi qswitch default
endif

if (\$path[1] != "/usr/bin") then
    set path = (\$path[1] ${target}/bin \$_opath)
else
    set path = (${target}/bin \$_opath)
endif
unset _opath 

if ( \$?MANPATH ) then
  setenv MANPATH "\${MANPATH}:${target}/man"
else
  setenv MANPATH ":${target}/man"
endif
EOF

chmod a+x $target/bin/sdeenv.sh
chmod a+x $target/bin/sdeenv.csh

global=0
manual=1

case "${SHELL}" in
*tcsh)  rccmd=source
	rctype=csh
	if test -w ${HOME}/.tcshrc; then
	    rcfile=.tcshrc
	elif test -w ${HOME}/.cshrc; then
	    rcfile=.cshrc
	else
	    rcfile=.login
	fi ;;
*csh)   rccmd=source
	rctype=csh
	if test -w ${HOME}/.cshrc; then
	    rcfile=.cshrc
	else
	    rcfile=.login
	fi ;;
*)      rccmd=. 
	rctype=sh
	rcfile=.profile ;;
esac

if test "$system" = "win32"; then
    # Create /etc/profile.d now, if no-one else has
    mkdir -p /etc/profile.d
fi

# Try to set environment globally (this only works on Linux)
if test -d /etc/profile.d && test -w /etc/profile.d; then
    ${ECHON} "Set environment globally in /etc/profile.d? (y/n) [y] ${ECHOE}"
    read check
    case "${check}" in
    [nN]*) ;;
    *)	rm -f /etc/profile.d/sdeenv.sh /etc/profile.d/sdeenv.csh
	cp ${target}/bin/sdeenv.*sh /etc/profile.d/ && global=1 ;;
    esac
fi

if test ${global} -eq 0; then
    ${ECHON} "Set private environment in ${HOME}/${rcfile}? (y/n) [y] ${ECHOE}"
    read check
    case "${check}" in
    [nN]*)  ;;
	*)  echo "${rccmd} ${target}/bin/sdeenv.${rctype}" >>${HOME}/${rcfile}
	    manual=0 ;;
    esac
fi

echo ""
echo "Setup of ${product} complete"
echo ""

echo "Enter the following command now to start using ${product}."
echo "  ${rccmd} ${target}/bin/sdeenv.${rctype}"
echo ""

if test ${global} -eq 0; then
    echo "Don\'t forget to add one of these lines to the .cshrc and/or"
    echo ".profile file of each ${product} user too."
    echo "    csh/tcsh:  source ${target}/bin/sdeenv.csh"
    echo "    sh/bash:   . ${target}/bin/sdeenv.sh"
    echo ""
fi

exit 0
