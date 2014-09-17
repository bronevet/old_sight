#!/usr/bin/perl

use strict;
use warnings;

my $numSteps=256;
my $dt = 1;
my $verbose = 1;
my $dir = "..";

$ENV{SIGHT_FILE_OUT}=1;

sys("rm -rf dbg.CoMD.lat_*.dt_*", $verbose);

while($dt<($numSteps>32?32:$numSteps)) {
foreach my $lat (3.615, 10, 20) {
  sys("rm -fr dbg.CoMD", $verbose);
  sys("$dir/apps/CoMD/bin/CoMD-mpi.compmodules --lat=$lat --dt=$dt --nSteps=".($numSteps/$dt), $verbose);

  $dt *=2;
} }

sys("$dir/hier_merge dbg.CoMD.CompModules zipper dbg.CoMD.CompModules.dt_*.lat_*/structure");
sys("$dir/slayout dbg.CoMD.CompModules/structure");

sys("rm *.yaml", $verbose);

sub sys {
  my ($cmd, $verbose) = @_;

  print "$cmd\n";
  system $cmd;
}

