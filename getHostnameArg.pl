#!/usr/bin/perl

use strict;

# This script tests the command line parameters that hostname makes available
use IPC::Open3;

run("hostname --skldfj");
if(run("hostname --aldns")) { print "--as"; }
elsif(run("hostname --all-fqdns")) { print "--all-fqdns"; }
elsif(run("hostname --fqdn")) { print "--fqdn"; }
elsif(run("hostname")) { }
else {  die "hostname command not functional!"; }

# Runs the given command, returns true on success and false on failure
sub run
{
  my ($cmd) = @_;

  my $pid = open3(my $in, my $out, my $err, $cmd) || die "ERROR running command \"$cmd\"!"; 
  waitpid($pid, 0);
  my $child_exit_status = $? >> 8;
  #print "child_exit_status=$child_exit_status\n";
  return $child_exit_status==0;
}
