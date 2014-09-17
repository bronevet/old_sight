#!/usr/bin/perl

sub connect_be {
        my $conn_str = shift;
	my $be_host_name = shift;
	my $sight_app = shift;
	my $app_params = shift ;
        
  	#my $params_w_n = $app_params ;
	my $sight_mrnet_env ;
	my $cmd ;
        if($be_host_name == "localhost" || $be_host_name == "127.0.0.1" ){
		#local BE
		$sight_mrnet_env="$conn_str";
		delete $ENV{'SIGHT_FILE_OUT'};
		$ENV{'MRNET_MERGE_EXEC'} = $sight_mrnet_env ;
		$cmd = "$sight_app $app_params";		
                print "$cmd -- $sight_mrnet_env -- $be_host_name  \n" ;  
  		system("$cmd");
	}else {
   		#remote BE - connect via ssh
		#unset SIGHT_FILE_OUT;
		#export MRNET_MERGE_EXEC="./smrnet_be 127.0.0.1 $PARENT_PORT $PARENT_RANK 127.0.0.1 $BE_RANK";
		
	}
}

my $TOP_FNAME = $ARGV[0] || 'connection.params' ;
my $PARAM_FNAME = $ARGV[1] || 'app.params' ;

print "taking parameters from $TOP_FNAME : $PARAM_FNAME \n";

open FILE, $TOP_FNAME or die $!;
open P_FILE, $PARAM_FNAME or die $!;

my @lines = <FILE> ;
my @params = <P_FILE> ;

my $i = 0;
my @children;
my $rank = 10200;

foreach my $line(@lines){
  @words = split(/ /, $line);
  @line_w_n = split(/\n/, $line);

  @app_params = split(/ /, $params[$i]);

  #parse application specific params
  $bin_name = shift @app_params;
  $be_hname = shift @app_params;

  $conn_str = "smrnet_be $line_w_n[0] $be_hname $rank";
#  system($cmd);
  my $pid = fork();
  if ($pid) {
	#parent
	push(@children, $pid); 
  } elsif($pid == 0){
 	#child
 	@tmp = split(/\n/,"@app_params") ;
	#print "$tmp[0]\n";
        connect_be($conn_str, $be_hname, $bin_name, $tmp[0]);
        exit 0 ;   
  } else{
        die "couldnt fork: $!\n";
  } 
  $i = $i + 1;
  $rank = $rank + 1 ;
}

foreach (@children) {
        my $tmp = waitpid($_, 0);
 
}

print "done...\n";

close $FILE;
close $P_FILE;



