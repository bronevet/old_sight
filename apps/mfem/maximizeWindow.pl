#!/usr/bin/perl

use warnings;
use strict;
require "../../dbglogDefines.pl";

if(scalar(@ARGV)!=1) { die "Usage: maximizeWindow.pl winIDString"; }
my $winIDString = $ARGV[0];
#my $vncDesktop = $ARGV[1]; # This argument is only used by other processes to help identify the VNC session that this script is running in
print "maximizeWindow: winIDString=$winIDString\n";

# Get the current screen resolution
my $curResolution = `xrandr -q | awk -F'current' -F',' 'NR==1 {gsub("( |current)","");print \$2}'`;
print "curResolution=$curResolution\n";
my ($width, $height) = split(/x/, $curResolution);
$ENV{LD_LIBRARY_PATH} = "$main::dbglogPath/widgets/xdottool:$ENV{LD_LIBRARY_PATH}";

# Keep setting the target app's window to fill the entire desktop, even if the user tries to change this
while(1) {
  print "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString\n";
  my $winsStr = `$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString`;
  print "winsStr=$winsStr\n";
  my @wins = split(/\s+/, $winsStr);

  if(scalar(@wins) > 0) {
    # Kill all windows but the first one
    for(my $i=0; $i<scalar(@wins)-1; $i++) {
      print  "$main::dbglogPath/widgets/xdottool/xdotool windowkill $wins[$i]\n";
      system "$main::dbglogPath/widgets/xdottool/xdotool windowkill $wins[$i]";
    }

    # Maximize the most recently opened window
    splice(@wins, 0, scalar(@wins)-1);
    print  "$main::dbglogPath/widgets/xdottool/xdotool windowmove $wins[0] 0 0\n";
    system "$main::dbglogPath/widgets/xdottool/xdotool windowmove $wins[0] 0 0";
    print  "$main::dbglogPath/widgets/xdottool/xdotool windowsize $wins[0] $width $height\n";
    system "$main::dbglogPath/widgets/xdottool/xdotool windowsize $wins[0] $width $height";
  }

#  system "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString windowmove 0 0";
  #print "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString windowsize $width $height\n";
#  system "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString windowsize $width $height";
  #system "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString windowfocus";
  #system "$main::dbglogPath/widgets/xdottool/xdotool search --name $winIDString windowraise";
 
  #foreach my $var (keys %ENV) {
  #  print "::::$var => $ENV{$var}\n";
  #}
 
  sleep(1);
}
