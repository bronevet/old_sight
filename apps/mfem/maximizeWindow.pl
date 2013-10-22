#!/usr/bin/perl

use warnings;
use strict;
#require "../../dbglogDefines.pl";

if(scalar(@ARGV)!=4) { die "Usage: maximizeWindow.pl dbglogPath vncWidth vncHeight winIDString"; }
# The path of the dbglog root directory
my $dbglogPath  = $ARGV[0];
# The size of the vnc session window
my $vncWidth    = $ARGV[1];
my $vncHeight   = $ARGV[2];
# String that uniquely identifies the window to be maximized.
my $winIDString = $ARGV[3];
#my $vncDesktop = $ARGV[1]; # This argument is only used by other processes to help identify the VNC session that this script is running in
#print "maximizeWindow: winIDString=$winIDString\n";

# Get the current screen resolution
#my $curResolution = `xrandr -q | awk -F'current' -F',' 'NR==1 {gsub("( |current)","");print \$2}'`;
#print "curResolution=$curResolution\n";
#my ($width, $height) = split(/x/, $curResolution);

# Add the xdotool directory to the library path to enable xdotool to be called to manipulate window properties
$ENV{LD_LIBRARY_PATH} = "$dbglogPath/widgets/xdotool:$ENV{LD_LIBRARY_PATH}";

# Keep setting the target app's window to fill the entire desktop, even if the user tries to change this
while(1) {
  print "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString\n";
  my $winsStr = `$dbglogPath/widgets/xdotool/xdotool search --name $winIDString`;
  print "winsStr=$winsStr\n";
  my @wins = split(/\s+/, $winsStr);

  if(scalar(@wins) > 0) {
    # Kill all windows but the first one
    for(my $i=0; $i<scalar(@wins)-1; $i++) {
      #print  "$dbglogPath/widgets/xdotool/xdotool windowkill $wins[$i]\n";
      `$dbglogPath/widgets/xdotool/xdotool windowkill $wins[$i]`;
    }

    # Maximize the most recently opened window
    splice(@wins, 0, scalar(@wins)-1);
    #print  "$dbglogPath/widgets/xdotool/xdotool windowmove $wins[0] 0 0\n";
    `$dbglogPath/widgets/xdotool/xdotool windowmove $wins[0] 0 0`;
    #print  "$dbglogPath/widgets/xdotool/xdotool windowsize $wins[0] $width $height\n";
    `$dbglogPath/widgets/xdotool/xdotool windowsize $wins[0] $vncWidth $vncHeight`;
  }

#  system "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString windowmove 0 0";
  #print "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString windowsize $vncWidth $vncHeight\n";
#  system "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString windowsize $vncWidth $vncHeight";
  #system "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString windowfocus";
  #system "$dbglogPath/widgets/xdotool/xdotool search --name $winIDString windowraise";
 
  #foreach my $var (keys %ENV) {
  #  print "::::$var => $ENV{$var}\n";
  #}
 
  sleep(1);
}
