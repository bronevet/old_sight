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

SIZE = str(250);
NITERS = str(5);
numInjSites = 1;
injPerSite = 1;

SIGHT_ROOT = "/g/g15/bronevet/code/tmp/sight"
CODE_ROOT = SIGHT_ROOT+"widgets/kulfi/apps/miniFE-2.0_ref/src"

os.environ["SIGHT_FILE_OUT"] = "1"

runFI = True
nxyz = ["5", "10", "20", "30", "40"];#["5", "10", "20"]; #["10", "20", "30", "40"]; #["10", "15", "20", "25", "30", "35", "40"]
load_imbalances = ["0"]; #["0", "1"];
tols = ["1e-05", "1e-10", "1e-15"];

## Fault Injection
if(runFI) :
    os.system("rm -rf data dbg.miniFE* *.core")
    os.system("mkdir -p data")
    os.chdir("data")

    idx = 0
    
    for n in nxyz :
      for li in load_imbalances :
        for tol in tols :
            args = ["nx="+n, "ny="+n, "nz="+n, "load_imbalance="+li, "tol="+tol]
    
            # Reference - No Injection
            os.environ["NEXT_FAULT_COUNTDOWN"] = "-1"
            os.environ["EXP_ID"] = "ref"
            print CODE_ROOT+"/miniFE.CSR "+" ".join(args)+"\n";
            numFaultSites = runAppNoFI([CODE_ROOT+"/miniFE.CSR"]+args);
            assert numFaultSites>=0, "Number of fault sites not reported!";
    
            for i in range(numInjSites) :
                dynLoc = math.floor(random.random() * numFaultSites);
                os.environ["NEXT_FAULT_COUNTDOWN"] = str(dynLoc)
                print "NEXT_FAULT_COUNTDOWN="+str(dynLoc)
    
                for j in range(injPerSite) :
                    os.environ["EXP_ID"] = str(idx)
                    path="dbg.miniFE.nx_"+n+".ny_"+n+".nz_"+n+".tol_1e"+tol+".mv_overlap_comm_comp_0.use_locking_0.load_imbalance_0.mtxType_CSR.exp_"+str(idx);
                    print CODE_ROOT+"/miniFE.CSR "+" ".join(args)+" &>"+path+".out\n";
                    os.system(CODE_ROOT+"/miniFE.CSR "+" ".join(args)+" &>"+path+".out");
                    idx+=1
    os.chdir("..")

else :
    os.system("rm -rf dbg.miniFE*")


# Merge Logs

os.system("mkdir -p data/merge");

print "MERGE"
for n in nxyz :
  for li in load_imbalances :
    for tol in tols :
        args = ["nx_"+n, "ny_"+n, "nz_"+n, "tol_"+tol, "mv_overlap_comm_comp_0", "use_locking_0", "load_imbalance_"+li, "mtxType_CSR"]
      
        # Merge sub-sets of the runs
        for i in range(10) :
            print SIGHT_ROOT+"/hier_merge data/merge/dbg.miniFE."+".".join(args)+".branch_"+str(i)+" zipper data/dbg.miniFE."+".".join(args)+".exp_*["+str(i)+"]/structure"
            os.system(SIGHT_ROOT+"/hier_merge data/merge/dbg.miniFE."+".".join(args)+".branch_"+str(i)+" zipper data/dbg.miniFE."+".".join(args)+".exp_*["+str(i)+"]/structure")

        # Merge each sub-set into a single file
        print SIGHT_ROOT+"/hier_merge data/merge/dbg.miniFE."+".".join(args)+".allbranches zipper data/dbg.miniFE."+".".join(args)+".exp_ref/structure data/merge/dbg.miniFE."+".".join(args)+".branch_[0-9]/structure"
        os.system(SIGHT_ROOT+"/hier_merge data/merge/dbg.miniFE."+".".join(args)+".allbranches zipper data/dbg.miniFE."+".".join(args)+".exp_ref/structure data/merge/dbg.miniFE."+".".join(args)+".branch_[0-9]/structure");
       

print SIGHT_ROOT+"/hier_merge dbg.miniFE zipper data/merge/dbg.miniFE.*.allbranches/structure\n"
os.system(SIGHT_ROOT+"/hier_merge dbg.miniFE zipper data/merge/dbg.miniFE.*.allbranches/structure")

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
print SIGHT_ROOT+"/slayout dbg.miniFE/structure\n"
os.system(SIGHT_ROOT+"/slayout dbg.miniFE/structure")

print "PACKAGING"
print "rm -f dbg.miniFE.tar.gz; rm dbg.miniFE/structure; tar -cf dbg.miniFE.tar dbg.miniFE; gzip dbg.miniFE.tar";
os.system("rm -f dbg.miniFE.tar.gz; rm dbg.miniFE/structure; tar -cf dbg.miniFE.tar dbg.miniFE; gzip dbg.miniFE.tar");

# Remove the core files
os.system("rm -f data/*.core data/*.yaml");


