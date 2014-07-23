#!/usr/apps/python2.7.3/bin/python

import sys
import os
from subprocess import call
import random
import subprocess
import re
import math
import argparse

SIGHT_ROOT = "/nfs/tmp2/bronevet/sight/sight"
CODE_ROOT = SIGHT_ROOT+"/apps/AMG2013"
procsPerNode = 16

def main(argv):
  parser = argparse.ArgumentParser(description='Experiments with AMG2013 configurations')
  parser.add_argument("--mtx",   dest="mtx",   action="store", nargs="+", help="Input matrix to be solved")
  args = parser.parse_args()
  
  os.environ["SIGHT_FILE_OUT"] = "1"
  
  runFI = True
  if(args.mtx and len(args.mtx)>0) : mtx = args.mtx
  elif("AMG_MTX" in os.environ) : mtx = os.environ["AMG_MTX"].split()
  else :                          mtx = ["default"]#, "laplace", "27pt", "jumps"];
  print "mtx="+str(mtx)
  solver = ["PCG_AMG", "PCG_Diag", "GMRES_AMG", "GMRES_Diag"];
  pooldist = ["0", "1"];
  power = ["25", "50", "75", "100"];
  refine = ["1", "6", "12"];
  ncube = ["10", "40", "100"];
  tol = ["1e-06", "1e-05", "1e-04"]
  # The distribution of processors among the x, y z dimensions. The last entry must correspond to 
  # the distribution with the largest values in each dimension. The problem size is pMult*px*py*pz of the largest entry.
  # px, py and pz must be powers of 2.
  pSize = [4,2,2]
  numprocs = [[2,2,2], [4,2,2], [4,4,2]]
  
  #os.system("rm -rf data")
  os.system("rm -rf data/*.core dbg.AMG2013* *.core")
  os.system("mkdir -p data")
  os.system("mkdir -p data/merge");
  os.chdir("data")
  os.system("cp "+CODE_ROOT+"/test/sstruct.in.MG.FD .");
  
  idx = 0
  
  for m in mtx :
   for s in solver :
    for pd in pooldist :
     for pw in power :
      for t in tol :
       for np in numprocs :
        if(m == "default") :
            n = "10"
            for r in refine :
                runApp(m, s, pd, pw, t, np, pSize, r, n, idx)
                merge (m, s, pd, pw, t, np, pSize, r, n, idx)
        else :
            r = "1"
            for n in ncube :
                runApp(m, s, pd, pw, t, np, pSize, r, n, idx)
                merge (m, s, pd, pw, t, np, pSize, r, n, idx)
  
  os.chdir("..")
  
  
  # Merge Logs
  #os.system("mkdir -p data/merge");
  #
  #def mergeLayout(m, s, pd, pw, r, n) :
  #    (path, args) = configuration(m, s, pd, pw, r, n)
  #        
  #    # Merge data from all ranks into a single log
  #    if(not os.path.exists("data/merge/"+path)) : 
  #        syscall(SIGHT_ROOT+"/hier_merge data/merge/"+path+" zipper data/"+path+".rank_*", True);
  #        
  #        # Lay out the html output
  #        #print "SLAYOUT"
  #        #syscall(SIGHT_ROOT+"/slayout data/merge/"+path, True)
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
  syscall(SIGHT_ROOT+"/hier_merge dbg.AMG2013 zipper data/merge/dbg.AMG2013.numprocs_*", True)
  
  # Lay out the html output
  print "SLAYOUT"
  print SIGHT_ROOT+"/slayout dbg.AMG2013\n"
  os.system(SIGHT_ROOT+"/slayout dbg.AMG2013")
  
  print "PACKAGING"
  #syscall("rm -f dbg.AMG2013.tar.gz", True);
  #syscall("rm -f dbg.AMG2013/structure data/merge/dbg.AMG2013.*/structure", True);
  #syscall("tar -cf dbg.AMG2013.tar dbg.AMG2013 data/merge/dbg.AMG2013.*", True);
  #syscall("gzip dbg.AMG2013.tar", True);
  
  # Remove the core files
  os.system("rm -f data/*.core data/*.yaml");

def syscall(command, verbose=True) :
    print command;
    os.system(command);
    return
  
def configuration(m, s, pd, pw, t, np, pSize, r, n) :
      args = []
      if(m != "default") :
          args = ["-"+m]
      args += ["-solver", s, "-pooldist", pd, "-r", r, r, r, "-n", n, n, n, "-tol", t]
     
      # Compute the total number of processors 
      numprocs = 1;
      for i in range(0, 3): numprocs *= np[i]
  
      # We choose px, py, pz so that the total amount of work is the same for all choices of np
      part = [0]*3;
      print "pSize="+str(pSize)+", np="+str(np)
      for i in range(0, 3): part[i] = np[i]
      # If pool_distribution is 0 we specify the allocation to be 8x larger (*2 in each dimension)
      if(pd=="1") : 
        for i in range(0, 3) : part[i] /= 2
      args += ["-P", str(part[0]), str(part[1]), str(part[2])]

      path="dbg.AMG2013.numprocs_"+str(numprocs)+".mtx_"+m+".solver_"+s+".P_"+str(part[0])+"_"+str(part[1])+"_"+str(part[2])+".r_"+r+"_"+r+"_"+r+".b_1_1_1.n_"+n+"_"+n+"_"+n+".pooldist_"+pd+".rhs_default.powercap_"+pw+".tol_"+t;
  
      return (path, args, numprocs)
  
def runApp(m, s, pd, pw, t, np, pSize, r, n, idx) :
      (path, args, numprocs) = configuration(m, s, pd, pw, t, np, pSize, r, n)
      print "path="+path
      if(not os.path.exists(path+".rank_0")) : 
          print "Path \""+path+".rank_0\" does not exist"
          if(pw != 0) : 
              args += ["-powercap", str(pw)]
              with open("rapl_config", "w") as f:
                f.write("[rapl numProperties=\"4\" key_0=\"CPUWatts\" val_0=\""+str(pw)+"\" key_1=\"CPUSeconds\" val_1=\"0.1\" key_2=\"DRAMWatts\" val_2=\""+str(pw)+"\" val_3=\"DRAMSeconds\" val_3=\"0.1\"][/rapl]")
              os.environ["SIGHT_STRUCTURE_CONFIG"] = "rapl_config"
           
          syscall("mpirun -n "+str(numprocs)+" "+CODE_ROOT+"/test/amg2013 "+" ".join(args)+" >"+path+".out", True);

          if(pw != 0) :
              del os.environ['SIGHT_STRUCTURE_CONFIG']
      idx+=1
      return;
  
def merge(m, s, pd, pw, t, np, pSize, r, n, idx) :
      (path, args, numprocs) = configuration(m, s, pd, pw, t, np, pSize, r, n)
          
      # Merge data from all ranks into a single log
      if(not os.path.exists("merge/"+path)) : 
          syscall(SIGHT_ROOT+"/hier_merge merge/"+path+" zipper "+path+".rank_*", True);
          
      return;
  


if __name__ == "__main__":
   print str(sys)
   main(sys.argv[1:])

