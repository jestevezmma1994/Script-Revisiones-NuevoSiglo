#!/bin/sh

# Find the directory that we live in
FRAGDIR=`dirname $0`
SDEBINDIR=${SDEBINDIR:-$FRAGDIR}

# We should be running from the SDEBASE/bin directory, do find SDEBASE
SDEBASEPATH=`dirname "${SDEBINDIR}"`

# This is where we find fragments

FRAGPATH=$SDEBASEPATH/etc/fragbuilder
# System for generating MDI fragments...

FRAGNAME="$1"

# Work out how not do an echo without a final line-feed
if test "`/bin/echo 'foo\c'`" = 'foo\c'; then
    ECHON="/bin/echo -n"
    ECHOE=""
else
    ECHON="/bin/echo"
    ECHOE='\c'
fi
export ECHON ECHOE

UNAME_SYSTEM=`(uname -s) 2>/dev/null` || UNAME_SYSTEM=unknown
case "$UNAME_SYSTEM" in
CYGWIN*)
    # Backspace is not default erase character - fix it for
    # the duration of this script.
    STTY=`stty -g`
    trap "stty $STTY; exit 1" 1 2 15
    stty erase ^h
    ;;
esac

echo "MIPS Software Toolkit MDI fragment generator"
echo "--------------------------------------------"
echo ""

if test ! -r "$FRAGPATH/${FRAGNAME}.fragbuild"; then

echo "     Type     /\\   Creates fragments for"
echo "--------------||-------------------------------------------------------"

for FRAG in "$FRAGPATH"/*.fragbuild; do
   sh $FRAG describe
done

echo "              \\/"
echo ""
echo "Please select the type of fragment you wish to create..."
echo ""

FRAGNAME=""

while test ! -r "$FRAGPATH/${FRAGNAME}.fragbuild"; do
    ${ECHON} "Type: ${ECHOE}"
    read FRAGNAME
    if test ! -r "$FRAGPATH/${FRAGNAME}.fragbuild"; then
       echo ""
       echo "Unknown fragment type: \"$FRAGNAME\"";
       echo ""
       echo "Please select the type of fragment you wish to create..."
       echo ""
    fi
done

fi

FRAG="$FRAGPATH/${FRAGNAME}.fragbuild do"

TEMPFILE="$HOME/.gen-mdifrag.$$"

sh $FRAG banner || exit 1;

echo ""
echo ""
echo "First, the standard questions..."
echo ""
echo "What shortname would you like to give your MDI fragment?"
echo "   o Examples include: default  mipssim4  fs2probe  greenhills"
echo ""
echo "   o If you use the name \"default\" this this will be selected"
echo "     automatically when you first login or open a shell window."

SHORTNAME=
while test -z "$SHORTNAME"; do
    echo ""
    ${ECHON} "Name: ${ECHOE}"
    read SHORTNAME
    if echo "$SHORTNAME" | egrep -i -s '^[a-z_][a-z0-9_]*$' >/dev/null; then
	:
    else
	echo ""
	echo "Name must start with an alphabetic character, and contain"
	echo "only alphanumerics or '_'. Please try again."
	SHORTNAME=
    fi
done

if test -f "$SDEBASEPATH/etc/${SHORTNAME}.mdi"; then
  echo ""
  echo "Fragment $SDEBASEPATH/etc/${SHORTNAME}.mdi already exists."
  echo ""
  ${ECHON} "Are you sure you want to overwrite it? (y/n) [n] ${ECHOE}"
  read check
  case "${check:-no}" in 
  y|Y|yes|YES)
 	;;
  *) 
	echo ""
	echo "OK, giving up, please try again later."
	echo ""
	exit 1 ;;
  esac
fi


echo ""
echo ""
echo "Secondly, enter a short (less than 50 characters if possible)"
echo "  description of the fragment. This is presented to users of the"
echo "  mdi command so that they can choose the correct fragment to"
echo "  select to access their debugger properly."
echo "    o Example: \"MIPSsim Version 4.0.8a\""
echo ""
echo "Note: do not enter the quote marks (\") in your description"
echo ""

DESCRIPTION=
while test -z "$DESCRIPTION"; do
    echo ""
    ${ECHON} "Description: ${ECHOE}"
    read DESCRIPTION
done

echo ""
echo ""

sh $FRAG ask $TEMPFILE || exit 1;

NEWTMP="$TEMPFILE.2"

sed \
  -e 's/"/\\"/g' \
  -e "s/^tmp_/echo \"${SHORTNAME}_/" \
  -e "s/$/\"/" \
  < $TEMPFILE > $NEWTMP

echo echo \"MDILIBS=\\\"\\\$MDILIBS\:${SHORTNAME}\\\"\" >> $NEWTMP
echo echo \"${SHORTNAME}_DESC=\\\""$DESCRIPTION"\\\"\" >> $NEWTMP


echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""

FAILED=0

cp $NEWTMP "$SDEBASEPATH/etc/${SHORTNAME}.mdi" || FAILED=1

if test "x$FAILED" = "x1"; then
    echo "Unable to copy your new fragment into place, it can be found as"
    echo "$TEMPFILE and should be copied as ${SHORTNAME}.mdi to"
    echo "$SDEBASEPATH/etc if you want it to be automatically detected"
    echo "by the SDE startup scripts"
    rm $TEMPFILE; mv $NEWTMP $TEMPFILE
else
    echo "Success."
    echo ""
    echo "Your fragment was saved as $SDEBASEPATH/etc/${SHORTNAME}.mdi"
    rm -f $TEMPFILE $NEWTMP
fi

case "$UNAME_SYSTEM" in
CYGWIN*)
    # Reset erase character
    stty $STTY
    ;;
esac

echo ""
echo "Please enter the command \"mdi addfrag ${SHORTNAME}\" to add this"
echo "new fragment to your current mdi command's configuration."
