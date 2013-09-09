#!/usr/bin/perl

#use warnings;
use strict;
use CGI;
use Cwd;
use IPC::Open3;
my $q = CGI->new;

require "dbglogDefines.pl";

#my $hostname = `hostname`; chomp $hostname;
#my $execFile = $q->param('execFile'); if($execFile eq "") { missingParam("execFile"); die; }
#my $args     = $q->param('args');     #if($args eq "") { missingParam("args"); return; }

# Create a new xstartup for the vnc server

# First, back up the original one
my $vncXStartup = "$ENV{HOME}/.vnc/xstartup";
if(-e $vncXStartup)
{ system "cp $vncXStartup ${vncXStartup}.dbglog_backup"; }

# Make sure that the vnc configuration directory exists
system "mkdir -p $ENV{HOME}/.vnc";
open(my $xstartup, ">$vncXStartup") || "ERROR opening file \"$vncXStartup\" for writing! $!";
  print $xstartup "#!/bin/sh\n";
  print $xstartup "[ -r /etc/sysconfig/i18n ] && . /etc/sysconfig/i18n\n";
  print $xstartup "export LANG\n";
  print $xstartup "export SYSFONT\n";
  #print $xstartup "vncconfig -iconic &\n";
  print $xstartup "unset SESSION_MANAGER\n";
  print $xstartup "unset DBUS_SESSION_BUS_ADDRESS\n";
  print $xstartup "OS=`uname -s`\n";
  print $xstartup "[ -r \$HOME/.Xresources ] && xrdb \$HOME/.Xresources\n";
  print $xstartup "xsetroot -solid grey\n";
  #print $xstartup "xterm -geometry 80x24+10+10 -ls -title \"\$VNCDESKTOP Desktop\" &\n";
  #print $xstartup "mwm &\n";
  #print $xstartup "$execFile $args&\n";
  print $xstartup "$main::dbglogPath/widgets/glvis-2.0/glvis -m $main::dbglogPath/widgets/mfem-2.0/data/ball-nurbs.mesh&\n";
  #print $xstartup "ps -ef\n";
  #print $xstartup "echo 'before sleep'\n";
  #print $xstartup "sleep 1\n";
  print $xstartup "~/code/dbglog/maximizeWindow.pl GLVis&\n";
  #print $xstartup "echo 'after sleep'\n";
  #print $xstartup "echo 'after max'\n";
close($xstartup);

system "chmod 700 $vncXStartup";

# Start a vnc server
my $vncserverPID = open3(my $vncServerIn, my $vncServerOut, my $vncServerErr, "vncserver") || die "ERROR running command \"vncserver\"! $!";
# Read the output of vnc until we get the ID of the created desktop
my $desktopID;
while(my $line=<$vncServerOut>) {
  chomp $line;
  #print "line=\"$line\"\n";
  if($line =~ /^\s*New '[^']+' desktop is [^:]+:([0-9]+)\s*$/) { 
    $desktopID=$1;
    print "DesktopID = $desktopID\n";
    waitpid($desktopID, 0);
    last;
  }
}
#close($vnc) || die "ERROR closing pipe to command \"vncserver\"! $!";

# Spawn a separate process to replace the xstartup script after a short delay to allow vncserver to read
# the one we just wrote
if(fork()==0) {
  sleep(5);

  # If there was an xstartup script before we started, replace it
  if(-e "${vncXStartup}.dbglog_backup")
  { system "mv ${vncXStartup}.dbglog_backup $vncXStartup"; }
	
  exit(0);
}

# Run noVNC and connect it to the just-started vncserver
my $noVNCPID = open3(my $noVNCIn, my $noVNCOut, my $noVNCErr, "$main::dbglogPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID") || die "ERROR running command \"$main::dbglogPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID\"! $!";

# The URL at which noVNC will host the VNC session
my $vncURL="";
while(my $line = <$noVNCOut>) {
  chomp $line;
  #print "line=\"$line\"\n";
  if($line =~ /^\s*(http:\/\/[^\/:]+:[0-9]+\/vnc.html\?host=[^&]+&port=[0-9]+)\s*$/)
  {
    $vncURL = $1; 
    print "vncURL=$vncURL\n";
    my $pid = fork();
    # We've now learned the URL to point the browser to
    #
    # Leave a process to wait for noVNC to complete
    if($pid == 0)
    { waitpid($noVNCPID, 0); }
    # The main process returns a redirection to the browser
    else {
      print $q->redirect($vncURL);
      exit(0);
    }
  }
}
#close($noVNCOut) ||  die "ERROR closing pipe to command \"$main::dbglogPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID\"! $!";

die "Finished reading output of noVNC but did not find url to point browser to!";

sub missingParam
{
  my ($paramName) = @_;

  print $q->header('text/html');
  print $q->start_html(-title=>'Missing parameter \"$paramName\"!');
  print "ERROR: Missing parameter \"$paramName\"!";
  print $q->end_html;
}

sub list2Str
{
        my ($list) = @_;

        if(not defined $list) { return "()"; }
        if(ref $list ne "ARRAY") { confess("[common] list2Str() ERROR: list is not an array!"); }

        my $out = "(";
        my $i=1;
        foreach my $val (@$list)
        {
                if(defined $val) { $out .= "$val"; }

                if($i < scalar(@$list))
                { $out .= ", "; }
                $i++;
        }
        $out.=")";
        return $out;
}

