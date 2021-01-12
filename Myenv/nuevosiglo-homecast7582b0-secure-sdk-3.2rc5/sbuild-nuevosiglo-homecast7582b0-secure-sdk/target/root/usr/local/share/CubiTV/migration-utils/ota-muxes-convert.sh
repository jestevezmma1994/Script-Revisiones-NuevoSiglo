#!/bin/sh

if [ -z "$1" ]
then
  echo "error in $0: missing parameter with file name to be converted."
  echo "  USAGE: $0 filename"
  exit 1
fi

# clear output file
rm -f "$1.tmp"

cat $1 | sed -e 's/;/:/g'\
| sed -e 's/&/:/g'\
| sed -e 's/\bmodulation=/mod=/g'\
| sed -e 's/\bpolarization=/pol=/g'\
| sed -e 's/\bsymbol_rate=/symbol=/g'\
| sed -e 's/\btype=/tunType=/g'\
| sed -e 's/\bSID=/sid=/g'\
| sed -e 's/\bplp_id=/plp=/g'\
| sed -e 's/\bbandwidth=/band=/g' >"$1.tmp"

mv -f "$1.tmp" $1
