#!/usr/bin/perl
$file = shift @ARGV;
$dflt = shift @ARGV;
exit 0 unless ($file && $dflt);
system ("chmod a+r,u+w $file");
open (F,"$file");
if ($dflt eq '-') {
  while (<F>) {
    for $en (@ARGV) {
      $path = $ENV{$en};
      s#$path#\$\{$en\}#g if ($path && $path ne '' && $en && $en ne '');
    }
    $content.=$_;
  }
} else {
  while (<F>) {
    s/(\s+|\-I|\-L|=|=\s*\'|=\s*\")(\/etc|\/var)/$1\$\{$dflt\}$2/g;
    s/(\-I|\-L)\s*(\/usr)/$1\$\{$dflt\}$2/g;
    s/(\s+|=|=\s*\'|=\s*\")(\/usr)(?!\/bin\/(install|\w*awk|perl\w*))/$1\$\{$dflt\}$2/g;
    for $en ($dflt,@ARGV) {
      $path = $ENV{$en};
      s#$path#\$\{$en\}#g if ($path && $path ne '' && $en && $en ne '');
    }
    $content.=$_;
  }
} 
close(F);
open (F,">$file"); print F $content; close F;
