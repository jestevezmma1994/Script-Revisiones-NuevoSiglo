#!/usr/bin/perl -w

my $command = $0;
my ($compile_root, $other_root);
my $environment;
my @subst = qw(usr lib include);
my @argv;
my $prefix;

$command=~s/.*\///;

sub substitute_paths($){
  my $arg = shift;
  if ($arg =~ m#^$compile_root#) { return $arg; }
  if ($other_root && $other_root ne '' && $arg =~ s#^$other_root##) {
      system("echo \"WARNING: `pwd`: $prefix$other_root$arg -> $prefix$arg\" >> $ENV{SRM_ROOT}/wrapper.warnings");  
  }
  for $path (@subst){
    if ($arg =~ /^(\/$path)(\/.*|)$/) {
      system("echo \"WARNING: `pwd`: $prefix$arg -> $prefix$compile_root$arg\" >> $ENV{SRM_ROOT}/wrapper.warnings");
      return $compile_root.$arg;
    }
  }
  return $arg;
}

if($command =~ /cross/){
  $compile_root = $ENV{CROSS_ROOT};
  $other_root = $ENV{BUILD_ROOT};
  $command =~ s/^.*-(.*-[\.\d]*)/$1/ or $command =~ s/^.*-//;
  $command = "$ENV{CROSS_PATH}/bin/$ENV{CROSS_ARCH}-$command";
  $environment = "$ENV{BUILD_ROOT}/bin/srm-cross-run";
} else {
  $compile_root = $ENV{BUILD_ROOT};
  $other_root = $ENV{CROSS_ROOT};
  $command =~ s/^.+?-//;
  $environment = "$ENV{BUILD_ROOT}/bin/srm-local-run";
}

my @params = ('I','L','idirafter ','include ','imacros ','iprefix ','isystem ');
while(@ARGV){
  my $arg = shift;
  # $arg =~ s/\"/\\\"/g;  
  for $word (@params) {
    if($arg =~ /^\s*(\-$word)/) {
      $prefix=$1; 
      $arg =~ s/^\s*$prefix\s*//;
      $arg = $prefix.substitute_paths($arg);
    }
  }
  $arg =~ s-^/usr-$compile_root/usr-;
  @argv = (@argv, $arg);
}
# print "$environment $command @argv\n";
# my $ret = system("$environment $command @argv");
# exit($ret>>8);
exec ($environment, $command, @argv);

