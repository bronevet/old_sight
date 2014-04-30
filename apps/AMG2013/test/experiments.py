#!/usr/bin/python

import os
from subprocess import call
import random
import subprocess
import re
import math

# Runs the application with no fault injection and returns the total number of 
# dynamic fault sites encountered in the application's execution.
def runAppNoFI(command):
    numFaultSites=-1
    proc = subprocess.Popen(command,stderr=subprocess.PIPE)
    while True:
      line = proc.stderr.readline()
      if line != '':
        #the real code does filtering here
        print "test:", line.rstrip()
        m = re.search("Total # fault sites enumerated : (\d+)", line.rstrip());
        if m:
            numFaultSites = int(m.groups()[0])
      else:
        break
    return numFaultSites;

SIZE = str(2);
NITERS = str(1);
numInjSites = 1;
injPerSite = 1;

SIGHT_ROOT = "/g/g15/bronevet/code/tmp/sight"
CODE_ROOT = SIGHT_ROOT+"/apps/AMG2013/test"

os.environ["SIGHT_FILE_OUT"] = "1"

runFI = True
mtx = ["laplace"]; #["27pt", "jumps"]
solver = ["PCG_AMG"]; #["PCG_Diag", "GMRES_AMG", "GMRES_Diag"];
#tols = ["1e-05", "1e-10", "1e-15"];

## Fault Injection
if(runFI) :
    os.system("rm -rf data dbg.AMG2013* *.core")
    os.system("mkdir -p data")
    os.chdir("data")

    idx = 0
    
    for m in mtx :
      for s in solver :
#        for tol in tols :
            args = ["-"+m, "-"+s, "-P", "2", "1", "1"]
    
            # Reference - No Injection
            os.environ["NEXT_FAULT_COUNTDOWN"] = "-1"
            os.environ["EXP_ID"] = "ref"
            print "mpirun -n 2 "+CODE_ROOT+"/amg2013.fi "+" ".join(args)+"\n";
            numFaultSites = runAppNoFI(["mpirun", "-n", "2", CODE_ROOT+"/amg2013.fi"] + args);
            assert numFaultSites>=0, "Number of fault sites not reported!";
    
            for i in range(numInjSites) :
                dynLoc = math.floor(random.random() * numFaultSites);
                os.environ["NEXT_FAULT_COUNTDOWN"] = str(dynLoc)
                print "NEXT_FAULT_COUNTDOWN="+str(dynLoc)
    
                for j in range(injPerSite) :
                    os.environ["EXP_ID"] = str(idx)
                    path="dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.exp_"+str(idx);
                    print "mpirun -n 2 "+CODE_ROOT+"/amg2013.fi "+" ".join(args)+" &>"+path+".out\n";
                    os.system(["mpirun", "-n", "2", CODE_ROOT+"/amg2013.fi"] + args + " &>"+path+".out");
                    idx+=1
    os.chdir("..")

else :
    os.system("rm -rf dbg.AMG2013*")


# Merge Logs

os.system("mkdir -p data/merge");

print "MERGE"
for m in mtx :
  for s in solver :
#    for tol in tols :
        args = ["-"+m, "-"+s, "-P", "2", "1", "1"]
      
        # Merge sub-sets of the runs
        for i in range(10) :
            print SIGHT_ROOT+"/hier_merge data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.branch_"+str(i)+" zipper data/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.exp_*["+str(i)+"]/structure"
            os.system(SIGHT_ROOT+"/hier_merge data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.branch_"+str(i)+" zipper data/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.exp_*["+str(i)+"]/structure")

        # Merge each sub-set into a single file
        print SIGHT_ROOT+"/hier_merge data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.allbranches zipper data/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.exp_ref/structure data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.branch_[0-9]/structure"
        os.system(SIGHT_ROOT+"/hier_merge data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.allbranches zipper data/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.exp_ref/structure data/merge/dbg.AMG2013.mtx_"+m+".solver_"+s+".P_2_1_1.r_1_1_1.b_1_1_1.pooldist_0.rhs_default.rank_0.branch_[0-9]/structure");
       

print SIGHT_ROOT+"/hier_merge dbg.AMG2013 zipper data/merge/dbg.AMG2013.*.allbranches/structure\n"
os.system(SIGHT_ROOT+"/hier_merge dbg.AMG2013 zipper data/merge/dbg.AMG2013.*.allbranches/structure")

# Project onto the different individual dimensions
#def mergeAndLayout(args):
#    print SIGHT_ROOT+"/hier_merge ../dbg.miniFE."+".".join(args)+" zipper dbg.miniFE."+".".join(args)+"/structure\n"
#    os.system(SIGHT_ROOT+"/hier_merge ../dbg.miniFE."+".".join(args)+" zipper dbg.miniFE."+".".join(args)+"/structure")
#
#    print SIGHT_ROOT+"/hier_merge ../dbg.miniFE zipper ../dbg.miniFE."+".".join(args)+"/structure\n"
#    os.system(SIGHT_ROOT+"/hier_merge ../dbg.miniFE zipper ../dbg.miniFE."+".".join(args)+"/structure")
#
#
#for n in nxyz :
#    args = ["nx_"+n, "ny_"+n, "nz_"+n, "tol_*", "mv_overlap_comm_comp_0", "use_locking_0", "load_imbalance_*", "mtxType_CSR"]
#    mergeAndLayout(args)
#
#for li in load_imbalances :
#    args = ["nx_*", "ny_*", "nz_*", "tol_*", "mv_overlap_comm_comp_0", "use_locking_0", "load_imbalance_"+li, "mtxType_CSR"]
#    mergeAndLayout(args)
#
#for tol in tols :    
#    args = ["nx_*", "ny_*", "nz_*", "tol_"+tol, "mv_overlap_comm_comp_0", "use_locking_0", "load_imbalance_*", "mtxType_CSR"]
#    mergeAndLayout(args)
        
# Lay out the html output
print "SLAYOUT"
print SIGHT_ROOT+"/slayout dbg.AMG2013/structure\n"
os.system(SIGHT_ROOT+"/slayout dbg.AMG2013/structure")

print "PACKAGING"
print "rm -f dbg.AMG2013.tar.gz; rm dbg.AMG2013/structure; tar -cf dbg.AMG2013.tar dbg.AMG2013; gzip dbg.AMG2013.tar";
os.system("rm -f dbg.AMG2013.tar.gz; rm dbg.AMG2013/structure; tar -cf dbg.AMG2013.tar dbg.AMG2013; gzip dbg.AMG2013.tar");

# Remove the core files
os.system("rm -f data/*.core data/*.yaml");


