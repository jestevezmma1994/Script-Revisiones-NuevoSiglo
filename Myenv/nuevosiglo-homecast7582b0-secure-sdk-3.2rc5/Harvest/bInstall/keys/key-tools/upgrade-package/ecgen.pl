#!/usr/bin/perl

my $openssl;
if(defined $ENV{OPENSSL}) {
	$openssl = $ENV{OPENSSL};
} else {
	$openssl = "openssl";
	$ENV{OPENSSL} = $openssl;
}

$SSLEAY_CONFIG=$ENV{"SSLEAY_CONFIG"};
$DAYS="3650";	# 10 years
#$SUBJ="/C=PL/L=WARSAW/O=SENTIVISION POLAND/CN=";
unless ( $ARGV[0] ) {
	printf STDERR "$0 <Subject>\n";
	printf STDERR "\texample: $0 \"/C=PL/L=WARSAW/O=SENTIVISION POLAND/CN=Test\"\n";
	exit 1;
}
$SUBJ=$ARGV[0];

$REQ="$openssl req $SSLEAY_CONFIG";
$X509="$openssl x509";
$ECP="$openssl ecparam";
$EC="$openssl ec";
$VERIFY="$openssl verify";

$CAKEY="ca.key.pem";
$CACERT="ca.cert.pem";
$CURVE="secp224r1";

# create EC parameters
system ("$ECP -name $CURVE -out $CURVE.pem") and die "$ECP failed\n";
print "Curve parameters created\n";

my $FILE="supplier";
# create a certificate request
system ("$REQ -new -nodes -subj \"$SUBJ\" -keyout $FILE.key.pem ". 
                    "-newkey ec:$CURVE.pem ".
                    "-new -out $FILE.req.pem") and die "$REQ failed\n";
print "Certificate request created\n";

system ("$X509 -req -days $DAYS -in $FILE.req.pem -inform PEM ".
     "-CA $CACERT -CAkey $CAKEY -out $FILE.cert.pem -set_serial ".int(time()))
     and die "$X509 failed\n";
print "Certificate request signed\n";

system ("$VERIFY -CAfile $CACERT $FILE.cert.pem") and die "$VERIFY failed\n";
print "Certificate verified\n";

system ("$EC -in $FILE.key.pem -inform PEM -out $FILE.key.der -outform DER")
     and die "$EC failed\n"; 
system ("$X509 -in $FILE.cert.pem -inform PEM -out $FILE.cert.der -outform DER")
     and die "$X509 failed\n"; 
system ("rm -f $FILE.req.pem $FILE.key.pem $FILE.cert.pem $CURVE.pem");
print "Done\n";

exit $RET;
