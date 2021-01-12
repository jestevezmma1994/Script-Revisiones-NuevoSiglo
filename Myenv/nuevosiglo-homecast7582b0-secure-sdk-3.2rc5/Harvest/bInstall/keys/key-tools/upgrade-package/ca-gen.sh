#!/bin/sh

if [ $# -lt 1 ]; then
	echo -e "$0 <Subject>" >&2
	echo -e "\texample: $0 \"/C=PL/L=WARSAW/O=SENTIVISION POLAND/CN=Test CA\"" >&2
	exit 1
fi

set -e
openssl ecparam -name secp224r1 -genkey -out ca.key.pem
openssl req -x509 -new -key ca.key.pem -out ca.cert.pem -days 3650 -subj "$*"
openssl x509 -in ca.cert.pem -out ca.cert.der -outform DER

exit 0
