#!/usr/bin/perl

#use warnings;
use strict;
use CGI;
use Cwd;

require "sightDefines.pl";
require "gdbLineNum.pl";

my $q = CGI->new;

#my $hostname = $q->param('hostname'); if($hostname eq "") { missingParam("hostname"); die; }
#my $username = $q->param('username'); if($username eq "") { missingParam("username"); die; }
my $hostname = `hostname`; chomp $hostname;
my $execFile = $q->param('execFile'); if($execFile eq "") { missingParam("execFile"); die; }
my $tgtCount = $q->param('tgtCount'); if($tgtCount eq "") { missingParam("tgtCount"); die;  }
my $args     = $q->param('args');     #if($args eq "") { missingParam("args"); return; }

#if(scalar(@ARGV)<=3) { die "Usage: gdbwrap.pl execFile tgtCount args"; }
#my $execFile = $ARGV[0];
#my $tgtCount = $ARGV[1];
#splice(0, 2, @ARGV); # Remove the first two parameters from @ARGV
#my $args = list2StrSep(@ARGV);

open(my $f, ">gdbscript") || die "ERROR opening file \"gdbscript\" for writing! $!";
print $f "set pagination off\n";
#print $f "set logging file gdb.txt\n";
print $f "set logging on\n";
print $f "set breakpoint pending on\n";
print $f "file $execFile\n";
#print $f "sharedlibrary sight.o\n";
#print $f "break advanceBlockCount\n";
print $f "break sight_structure.C:$main::gdbLineNum\n";
print $f "cond 1 sight::structure::block::blockID==$tgtCount\n";
print $f "r $args\n";
print $f "finish\n";
print $f "finish\n";
print $f "finish\n";
print $f "echo \\n\n";
print $f "echo \\n\n";
print $f "echo \\n\n";
print $f "bt\n";
close($f);

my ($username, $pass, $uid, $gid, $quota, $comment, $gcos, $dir, $shell, $expire) = getpwuid( $< );
#my ($groupname, $grouppasswd, $gid2, $groupmembers) = getgrgid($gid);
#my $cmd = "/g/g15/bronevet/code/sight/widgets/shellinabox-2.14/shellinaboxd --cgi -t -s \"/:$username:$gid:".getcwd.":gdb -x gdbscript\"";
#my $cmd = "/g/g15/bronevet/code/sight/widgets/shellinabox-2.14/shellinaboxd --cgi -t -s \"/:SSH:bijisan.lojik.net\"";
#my $cmd = "ssh -t -t $username\@".`hostname`." /g/g15/bronevet/code/sight/widgets/shellinabox-2.14/shellinaboxd --cgi -t -s \"/:SSH:bijisan.lojik.net\"";
my $cmd = "$main::sightPath/widgets/shellinabox/bin/shellinaboxd --cgi -t -s \"/:$username:$gid:".getcwd.":ssh -o PreferredAuthentications=keyboard-interactive -o PubkeyAuthentication=no $username\@$hostname gdb -x ".getcwd."/gdbscript\"";
#print $q->header('text/html');
#print "$cmd\n";
#foreach my $key (keys %ENV) { print "$key => $ENV{$key}<br>\n"; }
system $cmd;

sub missingParam
{
  my ($paramName) = @_;

  print $q->header('text/html');
  print $q->start_html(-title=>'Missing parameter \"$paramName\"!');
  print "ERROR: Missing parameter \"$paramName\"!";
  print $q->end_html;
}


#sub signal{
#  my ($sig) = @_;
#  my (@args) = @_;
#  print "got ".list2Str(@args)."\n";
#  open(my $f, ">>log");
#  print $f "got sig=$sig args=".list2Str(@args).", $$\n";
#  close($f);
#  die;
#};
#$SIG{HUP} = &signal;
#$SIG{TERM} = &signal;
#$SIG{STOP} = &signal;
#$SIG{INT} = &signal;
#$SIG{QUIT} = &signal;
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

