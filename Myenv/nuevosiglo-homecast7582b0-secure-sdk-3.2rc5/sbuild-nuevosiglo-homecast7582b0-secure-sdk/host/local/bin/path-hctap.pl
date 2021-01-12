#!/usr/bin/perl
$file = shift @ARGV;
exit 0 unless ($file);
system ("chmod a+r,u+w $file");
open (F,"$file");
while (<F>) {
  for $en ($dflt,@ARGV) {
    $path = $ENV{$en};
    s#\$\{$en\}#$path#g if ($path && $path ne '' && $en && $en ne '');
  }
  $content.=$_;
} 
close(F);
open (F,">$file"); print F $content; close F;
