#!/usr/bin/python

import os
from subprocess import call
import random
import subprocess
import re
import math

SIGHT_ROOT = "/g/g15/bronevet/code/tmp/sight"
CODE_ROOT = SIGHT_ROOT+"/apps/lulesh"

os.environ["SIGHT_FILE_OUT"] = "1"

prec = [4, 8, 16]
spatial = [3, 7, 15, 31]
dtime = [1]
power = [25, 50, 100, 1000]
mem = ["soa", "aos"]
numExperiments=1

print "host="+os.environ["HOSTNAME"]

#os.system("rm -rf data")
os.system("rm -rf data dbg.Lulesh* *.core")
os.system("mkdir -p data")
os.system("mkdir -p data/merge");
cmdPrefix = ". /usr/local/tools/dotkit/init.sh; use silo-4.8; use hdf5-gnu-serial-1.8.10"
os.chdir("data")

def sys(command, verbose=True) :
    print command;
    os.system(command);
    return

def configuration(expID, p, s, d, pw, m, ref, idx) :
    os.environ["EXP_ID"] = str(expID)
    args = ["-s", str(s)]
    if(d != 1) : args += ["-dtfixed", str(d)]
    if(pw != 0) : 
        args += ["-powercap", str(pw)]
        with open("rapl_config", "w") as f:
          f.write("[rapl numProperties=\"4\" key_0=\"CPUWatts\" val_0=\""+str(pw)+"\" key_1=\"CPUSeconds\" val_1=\"0.1\" key_2=\"DRAMWatts\" val_2=\""+str(pw)+"\" val_3=\"DRAMSeconds\" val_3=\"0.1\"][/rapl]")
        os.environ["SIGHT_STRUCTURE_CONFIG"] = "rapl_config"
        #sys("cat rapl_config", True)
    if(ref == 1): args += ["-isReference"]
    
    execName = "lulesh2.0.real"+str(p)+"."+m

    outPath = "dbg.Lulesh.nx_"+str(s)+".prec_"+str(p)+".its_9999999.numReg_11.balance_1.cost_1"
    if(d == 1): outPath += ".dtfixed_-1e-06"
    else :       outPath += ".dtfixed_"+str(d)
    outPath += ".dthydro_1e+20.dtcourant_0.01"
    if(pw == 0): outPath += ".powercap_0"
    else:        outPath += ".powercap_"+str(pw)
    outPath += ".mem_"+m+".isReference_"+str(ref)

    return (execName, outPath, args)

def runApp(expID, p, s, d, pw, m, ref, idx) :
    (execName, outPath, args) = configuration(expID, p, s, d, pw, m, ref, idx)
    if(not os.path.exists(outPath+".rank_0")) : 
        print "Path \""+outPath+".rank_0\" does not exit"
        sys(cmdPrefix + "; " + CODE_ROOT+"/"+execName+" "+" ".join(args)+" >out."+outPath, True);
    idx+=1
    del os.environ["SIGHT_STRUCTURE_CONFIG"]
    return;

#def merge(m, s, pd, pw, r, n, idx) :
#    (path, args) = configuration(m, s, pd, pw, r, n)
#        
#    # Merge data from all ranks into a single log
#    if(not os.path.exists("merge/"+path)) : 
#        del os.environ['SIGHT_CONFIG']
#        sys(SIGHT_ROOT+"/hier_merge merge/"+path+" zipper "+path+".rank_*", True);
#        
#    return


idx = 0

for expID in range(numExperiments) :
 for ip in range(len(prec)) :
  p = prec[ip]
  for iS in range(len(spatial)) :
   s = spatial[iS]
   for id in range(len(dtime)) :
    d = dtime[id]
    for ipw in range(len(power)) :
     pw = power[ipw]
     for im in range(len(mem)) :
         m = mem[im]
         if (expID == 0) and (p == max(prec)) and (s == max(spatial)) and (d == min(dtime)) and (ipw==0) and (im==0): ref=1
         else : ref=0
         runApp(expID, p, s, d, pw, m, ref, idx)
#        merge (p, s, d, pw, m, idx)

os.chdir("..")

# Merge all the runs
sys(SIGHT_ROOT+"/hier_merge dbg.Lulesh zipper data/dbg.Lulesh.*", True)

# Lay out the html output
print "SLAYOUT"
print SIGHT_ROOT+"/slayout dbg.Lulesh\n"
os.environ["SIGHT_LAYOUT_CONFIG"] = SIGHT_ROOT+"/examples/emitObsDataTable.conf"
os.system(SIGHT_ROOT+"/slayout dbg.Lulesh")

print "PACKAGING"
#sys("rm -f dbg.AMG2013.tar.gz", True);
#sys("rm -f dbg.AMG2013/structure data/merge/dbg.AMG2013.*/structure", True);
#sys("tar -cf dbg.AMG2013.tar dbg.AMG2013 data/merge/dbg.AMG2013.*", True);
#sys("gzip dbg.AMG2013.tar", True);

# Remove the core files
os.system("rm -f data/*.core");



