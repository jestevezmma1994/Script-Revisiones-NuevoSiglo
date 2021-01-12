#!/bin/sh

# MDI library manager...

# Input arguments:
#  1. shell mode to output for
#  2. arguments to the mdi function

CURRENTMDILIB="${MDI_LIB:-none}"
CURRENTMDIFRAGPATH="$MDI_FRAG_PATH"
SHELLMODE="$1"
shift

# Find the directory containing this script
FRAGDIR=`dirname $0`
# That should be the SDE .../bin directory
SDEBINDIR=${SDEBINDIR:-$FRAGDIR}
# Derive SDEBASE from SDEBINDIR
SDEBASEPATH=`dirname "${SDEBINDIR}"`

# First up, check the shellmode is usable.

if test "x$SHELLMODE" != "xsh" -a "x$SHELLMODE" != "xcsh"; then
   echo >&2 Shell mode supplied was \'$SHELLMODE\'
   echo >&2 Sorry, unable to comply. mdi.sh only understands sh and csh
   exit 1;
fi

if test "x$PATH" = "x"; then
   echo >&2 Your PATH appears to be empty. mdi.sh aborting
   exit 1;
fi

# Now, check for mdi fragments and source them if available

# Default fragment
MDILIBS="none"
none_DESC="Select this to clean your MDI environment"
none_SETVARS=""
none_PATHVARS=""

OLDIFS="$IFS"

IFS=":"
for FRAG in $CURRENTMDIFRAGPATH; do
  IFS="$OLDIFS"
  test -r $FRAG && eval `sh $FRAG`
  IFS=":"
done
IFS="$OLDIFS"

# Some functions needed for later

remove_if_in ()
{
   NEWVAL=""
   BITS=`eval echo \$\{$1\}`
   OLDIFS="$IFS"
   IFS=":"
   for BIT in $BITS; do
     if test "x$BIT" != "x$2"; then
        if test "x$NEWVAL" = "x"; then
	   NEWVAL="$BIT"
	else
	   IFS="$OLDIFS"
	   NEWVAL="$NEWVAL:$BIT"
	   IFS=":"
	fi
     fi
   done
   IFS="$OLDIFS"
   eval $1="\"$NEWVAL\""
}

add_to ()
{
   NEWVAL=`eval echo \$\{$1\}`;
   if test "x$NEWVAL" = "x"; then
      NEWVAL="$2"
   else
#     NEWVAL="$NEWVAL:$2";
      NEWVAL="$2:$NEWVAL";
   fi
   eval $1="\"$NEWVAL\""
}

output_variable ()
{
   OLDIFS="$IFS"
   IFS=""
   if test "x$SHELLMODE" = "xsh"; then
      if test "x"`eval echo \$\{$1\}` = "x"; then
         echo "unset $2;"
      else
	 echo "$2=\""`eval echo \$\{$1\}`"\";"
	 echo "export $2;"
      fi
   else
      if test "x"`eval echo \$\{$1\}` = "x"; then
         echo "unsetenv $2;"
      else
         # double up backslashes for csh
	 val="`eval echo \$\{$1\} | sed -e 's/\\\\/\\\\\\\\/g'`"
	 echo "setenv $2 \"$val\";"
      fi
   fi
   IFS="$OLDIFS"
}

# See what command we need to run...

case "x$1" in
xhelp|xusage)
    ;;
x|xavail|xavailable)
    ;;
xaddfrag)
    ;;
xswitch)
    ;;
xqswitch)
    ;;
*)
    # unrecognised command, assume fragment name
    if test "x$2" = "x"; then
      set switch "$1" 
    fi
    ;;
esac

COMMAND="$1"

if test "x$COMMAND" = "xhelp" -o "x$COMMAND" = "xusage"; then
   echo >&2 "Usage:"
   echo >&2 "   mdi help|usage                  This message"
   echo >&2 "   mdi addfrag <path to frag>      Add an MDI fragment to the system"
   echo >&2 "   mdi [avail|available]           List available MDI libraries"
   echo >&2 "   mdi [switch] <mdiname>          Switch to this MDI fragment"
   echo >&2 "   mdi qswitch <mdiname>           Switch to this MDI fragment, quietly"
   echo >&2 "   mdi none                        Clean out any MDI environment variables"
   exit 0;
fi

if test "x$COMMAND" = "x" -o "x$COMMAND" = "xavail" -o "x$COMMAND" = "xavailable"; then
   # Iterate through the MDILIBS, listing their availability
   OLDIFS="$IFS"
   IFS=":"
   for LIB in $MDILIBS; do
      IFS="$OLDIFS"
      DESC="`eval echo \$\{${LIB}_DESC\}`"
      CURR=" "
      test "x$LIB" = "x$CURRENTMDILIB" && CURR="*"
      printf >&2 "%s %-12s -- %s\\n" "$CURR" "$LIB" "$DESC"
   done
   IFS="$OLDIFS"
   exit 0;
fi

if test "x$COMMAND" = "xaddfrag"; then
   shift;
   FRAGTOADD=$1;
   if test "x$FRAGTOADD" = "x"; then
      echo >&2 "Missing fragment name"
      exit 1;
   fi

   if test -r "$FRAGTOADD"; then
     :
   elif test -r "$SDEBASEPATH/etc/$FRAGTOADD"; then
    FRAGTOADD="$SDEBASEPATH/etc/$FRAGTOADD"
   elif test -r "$SDEBASEPATH/etc/$FRAGTOADD.mdi"; then
    FRAGTOADD="$SDEBASEPATH/etc/$FRAGTOADD.mdi"
   else
    echo >&2 "MDI fragment $FRAGTOADD: not found"
    exit 1;
   fi

   # The path exists and is readable, assume it's a frag and add it
   remove_if_in CURRENTMDIFRAGPATH "$FRAGTOADD"
   add_to CURRENTMDIFRAGPATH "$FRAGTOADD"
   output_variable CURRENTMDIFRAGPATH MDI_FRAG_PATH
   exit 0;
fi

if test "x$COMMAND" = "xswitch" -o "x$COMMAND" = "xqswitch"; then
  NEWLIB=$2
  if test "x$NEWLIB" = "x"; then
     echo >&2 Sorry, can\'t switch to an missing fragment name, use \'none\' for that.
     exit 1;
  fi
  if test "x$NEWLIB" = "x$CURRENTMDILIB"; then
     if test "$COMMAND" != "qswitch"; then
       echo >&2 Already selected: $NEWLIB
     fi
     exit 0;
  fi

  if echo "$NEWLIB" | egrep -i -s '^[a-z_][a-z0-9_]*$' >/dev/null; then
    :
  else
     echo >&2 Invalid MDI fragment name: $NEWLIB
     exit 1
  fi

  if test "x`eval echo \$\{${NEWLIB}_DESC\}`" = "x"; then
     echo >&2 Unknown MDI fragment: $NEWLIB
     echo >&2 "Do you need to load it using 'mdi addfrag'?"
     exit 1;
  fi

  # _PATHVARS and _SETVARS
  # First up, check for the current lib
  PATHS_TO_CHANGE=""
  if test "x$CURRENTMDILIB" != "x" -a "x$CURRENTMDILIB" != "xnone"; then
     if test "$COMMAND" != "qswitch"; then
       echo >&2 Undoing settings for: `eval echo \$\{${CURRENTMDILIB}_DESC\}`
     fi

     # Unset all the _SETVARS...
     EMPTY=""

     OLDIFS="$IFS"
     VARS=`eval echo \$\{${CURRENTMDILIB}_SETVARS\}`
     IFS=":"
     for VAR in $VARS; do
        IFS="$OLDIFS"
        output_variable EMPTY $VAR
        IFS=":"
     done
     IFS="$OLDIFS" 

     # Remove the values in all of the PATH and LD_LIBRARY_PATH entries
     VARS=`eval echo \$\{${CURRENTMDILIB}_PATHVARS\}`
     IFS=":"
     for VAR in $VARS; do
        IFS="$OLDIFS"
	remove_if_in $VAR `eval echo \$\{${CURRENTMDILIB}_${VAR}\}`
	add_to PATHS_TO_CHANGE $VAR
        IFS=":"
     done
     IFS="$OLDIFS"
  fi

  # Next up, add in the values for the new library...
  if test "x$NEWLIB" != "xnone"; then
     if test "$COMMAND" != "qswitch"; then
       echo >&2 Initialising environment for: `eval echo \$\{${NEWLIB}_DESC\}`
     fi

     OLDIFS="$IFS"
     VARS=`eval echo \$\{${NEWLIB}_SETVARS\}`
     IFS=":"
     for VAR in $VARS; do
        IFS="$OLDIFS"
        output_variable ${NEWLIB}_${VAR} $VAR
        IFS=":"
     done
     IFS="$OLDIFS"

     # Add the values in all of the PATH and LD_LIBRARY_PATH entries
     VARS=`eval echo \$\{${NEWLIB}_PATHVARS\}`
     IFS=":"
     for VAR in $VARS; do
        IFS="$OLDIFS"
	add_to $VAR `eval echo \$\{${NEWLIB}_${VAR}\}`
	remove_if_in PATHS_TO_CHANGE $VAR
	add_to PATHS_TO_CHANGE $VAR
        IFS=":"
     done
     IFS="$OLDIFS"
  fi

  OLDIFS="$IFS"
  IFS=":"
  for VAR in $PATHS_TO_CHANGE; do
     IFS="$OLDIFS" 
     output_variable $VAR $VAR
     IFS=":"
  done
  IFS="$OLDIFS"

  # Finally set the MDI_LIB appropriately
  output_variable NEWLIB MDI_LIB
  exit 0;
fi

echo >&2 Unable to understand: mdi "$@"
exit 1;
