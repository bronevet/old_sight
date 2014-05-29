#!/usr/bin/python

import os
from subprocess import call
import random
import subprocess
import re
import math

SIGHT_ROOT = "/home/greg/sight"
CODE_ROOT = SIGHT_ROOT+"/apps/AMG2013"

os.environ["SIGHT_FILE_OUT"] = "1"

runFI = True
mtx = ["default", "laplace", "27pt", "jumps"];
solver = ["PCG_AMG", "PCG_Diag", "GMRES_AMG", "GMRES_Diag"];
pooldist = ["0", "1"];
power = ["100"]#["25", "50", "75", "100"];
refine = ["1", "6", "12"];
ncube = ["10", "40", "100"];

#os.system("rm -rf data")
os.system("rm -rf data/*.core dbg.AMG2013* *.core")
os.system("mkdir -p data")
os.system("mkdir -p data/merge");
os.chdir("data")
os.system("cp "+CODE_ROOT+"/test/sstruct.in.MG.FD .");

def sys(command, verbose=True) :
    print command;
    os.system(command);
    return

def configuration(m, s, pd, pw, r, n) :
    args = []
    if(m != "default") :
        args = ["-"+m]
    args += ["-solver", s, "-pooldist", pd, "-r", r, r, r, "-n", n, n, n]
    if(pd=="0") :
      part = "2"
      args += ["-P", "2", "2", "2"]
    else :
      part = "1"

    path="dbg.AMG2013.mtx_"+m+".solver_"+s+".P_"+part+"_"+part+"_"+part+".r_"+r+"_"+r+"_"+r+".b_1_1_1.n_"+n+"_"+n+"_"+n+".pooldist_"+pd+".rhs_default.power_"+pw;

    return (path, args)

def runApp(m, s, pd, pw, r, n, idx) :
    (path, args) = configuration(m, s, pd, pw, r, n)
    if(not os.path.exists(path+".rank_0")) : 
        print "Path \""+path+".rank_0\" does not exit"
        os.environ['POWER'] = pw
        os.environ['SIGHT_CONFIG'] = '/home/greg/sight/apps/AMG2013/rapl_config.'+pw
    
        sys("mpirun -n 8 "+CODE_ROOT+"/test/amg2013 "+" ".join(args)+" >"+path+".out", True);
    idx+=1
    return;

def merge(m, s, pd, pw, r, n, idx) :
    (path, args) = configuration(m, s, pd, pw, r, n)
        
    # Merge data from all ranks into a single log
    if(not os.path.exists("merge/"+path)) : 
        del os.environ['SIGHT_CONFIG']
        sys(SIGHT_ROOT+"/hier_merge merge/"+path+" zipper "+path+".rank_*", True);
        
    return


idx = 0

for m in mtx :
 for s in solver :
  for pd in pooldist :
   for pw in power :
      if(m == "default") :
          n = "10"
          for r in refine :
              runApp(m, s, pd, pw, r, n, idx)
              merge (m, s, pd, pw, r, n, idx)
      else :
          r = "1"
          for n in ncube :
              runApp(m, s, pd, pw, r, n, idx)
              merge (m, s, pd, pw, r, n, idx)

os.chdir("..")


# Merge Logs
#os.system("mkdir -p data/merge");
#
#def mergeLayout(m, s, pd, pw, r, n) :
#    (path, args) = configuration(m, s, pd, pw, r, n)
#        
#    # Merge data from all ranks into a single log
#    if(not os.path.exists("data/merge/"+path)) : 
#        sys(SIGHT_ROOT+"/hier_merge data/merge/"+path+" zipper data/"+path+".rank_*", True);
#        
#        # Lay out the html output
#        #print "SLAYOUT"
#        #sys(SIGHT_ROOT+"/slayout data/merge/"+path, True)
#
#    return
#
#print "MERGE"
#for m in mtx :
# for s in solver :
#  for pd in pooldist :
#   for pw in power :
#      if(m == "default") :
#          n = "10"
#          for r in refine :
#              mergeLayout(m, s, pd, r, n)
#      else :
#          r = "1"
#          for n in ncube :
#              mergeLayout(m, s, pd, r, n)

# Merge all the runs
sys(SIGHT_ROOT+"/hier_merge dbg.AMG2013 zipper data/merge/dbg.AMG2013.mtx_*.rhs_default", True)

# Lay out the html output
print "SLAYOUT"
print SIGHT_ROOT+"/slayout dbg.AMG2013\n"
os.system(SIGHT_ROOT+"/slayout dbg.AMG2013")

print "PACKAGING"
#sys("rm -f dbg.AMG2013.tar.gz", True);
#sys("rm -f dbg.AMG2013/structure data/merge/dbg.AMG2013.*/structure", True);
#sys("tar -cf dbg.AMG2013.tar dbg.AMG2013 data/merge/dbg.AMG2013.*", True);
#sys("gzip dbg.AMG2013.tar", True);

# Remove the core files
os.system("rm -f data/*.core data/*.yaml");


