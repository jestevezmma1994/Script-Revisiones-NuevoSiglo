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

my $id=-1;
my @file;
open(FILE, $ARGV[0]) or die "failed to open file $ARGV[0]";
entry:while (<FILE>) {
	next if /^\s*#/;
	chomp;
#	printf STDERR "$e{offset}:$e{size}:$e{src}:$e{dst}:".($attr?(join(",",keys %attr)):"")."\n";
	$id++;
	/\s*(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s*([^#]*)#*(.*)/;
	my %e;
	$e{offset}=$1 if $2 ne '-';
	$e{size}=$2 if $2 ne '-';
	$e{src}=$3 if $3 ne '-';
	$e{dst}=$4 if $4 ne '-';
	$e{name}=$5 if $5 ne '-';
	$e{comment}=$7;
	$e{id}=$id;
	my $attr=$e{attr}=$6; $attr=~s/\s*$//;
	my %attr=map { $_=>1 } split(/\s*,\s*/,$attr);
	$e{attr_h}=\%attr;

	push(@file,\%e);
}
close(FILE);

entry:foreach my $e (@file) {
	attribute:foreach my $a (split(/\s*,\s*/,$ARGV[2])) {
#		printf STDERR "checking $a, $e\n";
		if ($e->{attr_h}->{$a} || ($a=~/(.*)=(.*)/ && ($e->{$1} eq $2))) {
			next attribute;
		} else {
			next entry;
		}
	}
	my $res = $ARGV[1] eq '*'?join("\n", map {"$_=\"$e->{$_}\";"} keys %$e):
		$e->{$ARGV[1]};
	printf STDOUT "$res\n";
}

exit 0;
