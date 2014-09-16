#!/usr/bin/perl

sub connect_be {
        my $conn_str = shift;
	my $sight_app ;
	my $be_rank ;
	my $sight_mrnet_env ;
	my $be_host_name ;
	my $cmd ;
        if($be_host_name == "localhost" || $be_host_name == "127.0.0.1" ){
		#local BE
		$sight_mrnet_env="smrnet_be $conn_str";
		delete $ENV{'SIGHT_FILE_OUT'};
		$ENV{'MRNET_MERGE_EXEC'} = $sight_mrnet_env ;
		$cmd = $sight_app;		
	}else {
   		#remote BE - connect via ssh
		#unset SIGHT_FILE_OUT;
		#export MRNET_MERGE_EXEC="./smrnet_be 127.0.0.1 $PARENT_PORT $PARENT_RANK 127.0.0.1 $BE_RANK";
		
	}
  	system($cmd);
}

my $TOP_FNAME = $ARGV[0] || 'connection.params' ;
my $PARAM_FNAME = $ARGV[1] || 'file1.txt' ;

print "$TOP_FNAME $PARAM_FNAME \n";

open FILE, $TOP_FNAME or die $!;
open P_FILE, $PARAM_FNAME or die $!;

my @lines = <FILE> ;
my @params = <P_FILE> ;

my $i = 0;
my @children;

foreach my $line(@lines){
  @words = split(/ /, $line);
  @line_w_n = split(/\n/, $line);

  $cmd = "./my_program.sh $line_w_n[0] $params[$i]";
#  system($cmd);
  my $pid = fork();
  if ($pid) {
	#parent
	push(@children, $pid); 
  } elsif($pid == 0){
 	#child
        connect_be($cmd);
        exit 0 ;   
  } else{
        die "couldnt fork: $!\n";
  } 

  $i = $i + 1;
}

foreach (@children) {
        my $tmp = waitpid($_, 0);
 
}

print "done...\n";

close $FILE;
close $P_FILE;



