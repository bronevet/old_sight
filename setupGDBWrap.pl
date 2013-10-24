#!/usr/bin/perl

use warnings;
use strict;

# This script points gdbwrap.cgi to the line within dbglog.C at which gdb should break.
# This is done by scanning dbglog.C for the line that contains a special comment and then
# writing this line number into gdbLineNum.pl, which is required by gdbwrap.cgi

open(my $fin, "<dbglog_structure.C") || die "ERROR opening file dbglog.C for reading $!";
my $lnum=1;
my $found=0;
while(my $line = <$fin>) {
  chomp $line;
  if($line =~ /THIS COMMENT MARKS THE SPOT IN THE CODE AT WHICH GDB SHOULD BREAK/) {
    $found = 1;
    last;
  }
  $lnum++;
}
close($fin);

if(!$found) { die "ERROR: cannot find the line within dbglog.C where gdb should break!"; }

open(my $fout, ">gdbLineNum.pl") || die "ERROR opening file gdbLineNum.pl for writing! $!";
print $fout "\$main::gdbLineNum = $lnum;\n";
close($fout);
