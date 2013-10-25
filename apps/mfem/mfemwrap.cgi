#!/usr/bin/perl

#use warnings;
use strict;
use CGI;
use Cwd;
use IPC::Open3;
my $q = CGI->new;

require "../../sightDefines.pl";

my $hostname = `hostname`; chomp $hostname;
my @params = $q->param;
foreach my $param (@params)
{ $ENV{$param} = $q->param($param); }

my $execFile = $ENV{execFile}; if($execFile eq "") { missingParam("execFile"); die; }
my $mesh     = $ENV{mesh};     if($mesh eq "") { missingParam("mesh"); return; }
my $soln     = $ENV{soln};     if($soln eq "") { missingParam("soln"); return; }
if(not defined $ENV{USER}) { missingParam("USER"); return; }
if(not defined $ENV{HOME}) { missingParam("HOME"); return; }

# Check if a noVNC session is already running
my $allProcsStr = `ps -ef`;
my @allProcs = split(/^/, $allProcsStr);
#print "Content-Type: text/plain\n\n";
foreach my $proc (@allProcs) {
  #print "proc=\"$proc\"\n";
  # If noVNC is running, reuse the same VNC session
  if($proc =~ /^.*noVNC\/utils\/launch\.sh --vnc localhost:([0-9]+)$/) {
    system "$main::sightPath/apps/mfem/meshFile2Socket $mesh $soln";
    print $q->redirect("http://$hostname:6080/vnc.html?host=$hostname&port=6080");
    exit(0);
  }
}

# If noVNC is not currently running

# Create a new xstartup for the vnc server

# The size of the vnc session window
my $vncWidth = 1024;
my $vncHeight = 768;

# First, back up the original one
my $vncXStartup = "$ENV{HOME}/.vnc/xstartup";
if(-e $vncXStartup)
{ system "cp $vncXStartup ${vncXStartup}.sight_backup"; }

# Make sure that the vnc configuration directory exists
system "mkdir -p $ENV{HOME}/.vnc";
open(my $xstartup, ">$vncXStartup") || "ERROR opening file \"$vncXStartup\" for writing! $!";
system "/usr/bin/whoami";
  print $xstartup "#!/bin/sh\n";
  print $xstartup "[ -r /etc/sysconfig/i18n ] && . /etc/sysconfig/i18n\n";
  print $xstartup "export LANG\n";
  print $xstartup "export SYSFONT\n";
  #print $xstartup "vncconfig -iconic &\n";
  print $xstartup "unset SESSION_MANAGER\n";
  print $xstartup "unset DBUS_SESSION_BUS_ADDRESS\n";
  print $xstartup "OS=`uname -s`\n";
  print $xstartup "[ -r $ENV{HOME}/.Xresources ] && xrdb $ENV{HOME}/.Xresources\n";
  print $xstartup "xsetroot -solid grey\n";
  print $xstartup "xterm -geometry 80x24+10+10 -ls -title \"\$VNCDESKTOP Desktop\" &\n";
  #print $xstartup "mwm &\n";
  print $xstartup "$execFile&\n";
  #print $xstartup "$main::sightPath/apps/mfem/glvis/glvis -m $main::sightPath/mfem/mfem/mfem/data/ball-nurbs.mesh&\n";
  #print $xstartup "ps -ef\n";
  #print $xstartup "echo 'before sleep'\n";
  #print $xstartup "sleep 1\n";
  print $xstartup "$main::sightPath/apps/mfem/maximizeWindow.pl $main::sightPath $vncWidth $vncHeight GLVis\n";
  #print $xstartup "echo 'after sleep'\n";
  #print $xstartup "echo 'after max'\n";
close($xstartup);

system "chmod 700 $vncXStartup";
#print "Content-Type: text/plain\n\n";
#print "cat $vncXStartup\n";
#system "cat $vncXStartup";
#die;

# Start a vnc server
my $vncserverPID = open3(my $vncServerIn, my $vncServerOut, my $vncServerErr, "vncserver -geometry ${vncWidth}x${vncHeight}") || die "ERROR running command \"vncserver\"! $!";
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
  if(-e "${vncXStartup}.sight_backup")
  { system "mv ${vncXStartup}.sight_backup $vncXStartup"; }
	
  exit(0);
}

# Run noVNC and connect it to the just-started vncserver
my $noVNCPID = open3(my $noVNCIn, my $noVNCOut, my $noVNCErr, "$main::sightPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID") || die "ERROR running command \"$main::sightPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID\"! $!";

# The URL at which noVNC will host the VNC session
my $vncURL="";
my $noVNCOutput = "";
while(my $line = <$noVNCOut>) {
  $noVNCOutput .= $line; # Record noVNC's output in case we need to emit an error message
  chomp $line;
  #print "line=\"$line\"\n";
  if($line =~ /^\s*(http:\/\/[^\/:]+:[0-9]+\/vnc.html\?host=[^&]+&port=[0-9]+)\s*$/)
  {
    $vncURL = $1; 
    #print "vncURL=$vncURL\n";
    my $pid = fork();
    # We've now learned the URL to point the browser to
    #
    # Leave a process to wait for noVNC to complete
    if($pid == 0) {
      # Close the streams that connect the child process to the parent's console
      close STDOUT;
      close STDERR;
      close STDIN;
      waitpid($noVNCPID, 0);
    # The main process returns a redirection to the browser
    } else {
      sleep(1);

      # Inform the currently-running instance of GLVis that it should display the given pair of mesh and solution
      #print "Content-Type: text/plain\n\n";
      #print  "$main::sightPath/widget/mfem/meshFile2Socket $mesh $soln\n";
      system "$main::sightPath/apps/mfem/meshFile2Socket $mesh $soln";
      
      print $q->redirect($vncURL);
      exit(0);
    }
  }
}
#close($noVNCOut) ||  die "ERROR closing pipe to command \"$main::sightPath/widgets/noVNC/utils/launch.sh --vnc localhost:590$desktopID\"! $!";

print $q->header;
print "Finished reading output of noVNC but did not find url to point browser to!<br>\n";
print $noVNCOutput;

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

