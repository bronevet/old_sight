#!/usr/bin/perl

use warnings;
use Cwd;
use Cwd 'abs_path';
use File::Basename;

my $workDir = getcwd;

my $scriptPath = abs_path($0);
my ($exFilename, $exDir, $exSuffix) = fileparse($scriptPath);

my $verbose = 1;
sys("rm -fr dbg.MFEM.ex1*", $verbose);

#my @meshes = ("square-disc", "square-disc-p2", "square-disc-p3");
my @meshes
 = ("fichera", "fichera-q2", "fichera-q3");
my @ref_levels = (0, 1, 2, 3);
my @finElements = ("linear", "h1:1", "h1:2", "h1:3");

# Disable automatic generation of html output since we need to generate structure files for each sub-run of ex1
# so that their information can be merged
$ENV{SIGHT_FILE_OUT} = 1;
foreach my $mesh (@meshes) {
foreach my $ref_level (@ref_levels) {
foreach my $finElement (@finElements) {
foreach my $exactSoln (0, 1) {
#  chdir $exDir;
  sys("$exDir/ex1 $exDir/../data/${mesh}.mesh $ref_level $finElement $exactSoln", $verbose);
} } } }

#chdir $workDir;

# Merge the output of the individual runs
sys("../hier_merge dbg.MFEM.ex1 zipper dbg.MFEM.ex1.meshFile_*.ref_levels_*/structure", $verbose);

# Visualize the merged data
sys("../slayout dbg.MFEM.ex1/structure", $verbose);

# Remove the directories of the individual runs
#sys("rm -fr dbg.MFEM.ex1.*");

sub sys {
  my ($cmd, $verbose) = @_;

  print "$cmd\n";
  system $cmd;
}
