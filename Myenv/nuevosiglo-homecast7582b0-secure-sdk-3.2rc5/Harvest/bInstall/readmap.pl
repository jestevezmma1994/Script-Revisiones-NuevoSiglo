#!/usr/bin/perl

use strict;

if ( scalar @ARGV ne 3 ) {
	printf STDERR "
Usage:
	readmap.pl <file> <field> <attribute list>
![".join(" ",@ARGV)."]
";
	exit 1;
}

my @res;

open(FILE, $ARGV[0]) or die "failed to open file $ARGV[0]";
my $id=-1;
entry:while (<FILE>) {
	next if /^\s*#/;
	chomp;	
	$id++;
	/\s*(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s*([^#]*)#*(.*)/;
	my %e;
	$e{offset}=$1 if $2 ne '-';
	$e{size}=$2 if $2 ne '-';
	$e{src}=$3 if $3 ne '-';
	$e{dst}=$4 if $4 ne '-';
	$e{comment}=$6;
	my $attr=$e{attr}=$5; $attr=~s/\s*$//;
	my %attr=map { $_=>1 } split(/\s*,\s*/,$attr);
#	printf STDERR "$e{offset}:$e{size}:$e{src}:$e{dst}:".($attr?(join(",",keys %attr)):"")."\n";
	attribute:foreach my $a (split(/\s*,\s*/,$ARGV[2])) {
#		printf STDERR "checking $a\n";
		if ($attr{$a} || ($a=~/(.*)=(.*)/ && ($e{$1} eq $2 || ($1 eq 'id' && $2 eq $id)))) {
			next attribute;
		} else {
			next entry;
		}
	}
	my $res=$ARGV[1] eq 'id'?$id:$e{$ARGV[1]};
	push (@res,$res);
#	printf STDOUT "$res\n";
}
close(FILE);
print join("\n", reverse sort @res);
print "\n";
exit 0;
