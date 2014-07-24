#!/usr/apps/python2.7.3/bin/python
# -*- coding: iso-8859-1 -*-

import sys
import argparse
import os

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)

def main(argv):
  file = ''
  doUnpack = 0
  doPack = 0
  command = ''
  parser = argparse.ArgumentParser(description='Package programs for Swift commands')
  parser.add_argument('-inputs',  nargs='*', help='Input Files')
  parser.add_argument('-outputs', nargs='*', help='Output Files')
  parser.add_argument('-command', required=True, help='Command to execute')
  parser.add_argument('-environment', nargs='*', help='Environment of command')
  parser.add_argument('-work_dir', help='Working directory in which to run the command in')
  parser.add_argument('-pack_from_work', nargs='?', const="1", default="0", help="Indicates that we're packing and unpacking from the work directory rather than the root directory")
  args = parser.parse_args()

  if(args.inputs):
    print "inputs:"
    for i in args.inputs:
      print "    "+i
  
  if(args.outputs):
    print "outputs"
    for o in args.outputs:
      print "    "+o

  #if(args.command): 
  print "command "+args.command

  if(args.work_dir):
    print "work_dir "+args.work_dir

  print "pack_from_work="+args.pack_from_work

  if(args.environment):
    # The length of the environment array must be even
    if(len(args.environment)%2 != 0) :
      print "ERROR: provided environment has an odd number of values! environment="+",".join(args.environment)
      sys.exit(-1)

    print "Environment"
    for i in range(0, len(args.environment), 2) :
      print args.environment[i]+" => "+args.environment[i+1]

  origWD = os.getcwd()
  # Switch to the working directory if we need to do so before unpacking
  #switchToWork(bool(args.pack_from_work), args)
  run("pwd")
  run("ls -l *")
  run("du -h")

  # If input files were provided, unpack them
  if(args.inputs):
    os.system("echo 'Unpacking Inputs'")
    for i in args.inputs:
      for iFile in i.split(" "):
        if(args.pack_from_work) :
          iDir, iFName = os.path.split(iFile)
          switchToDir(True, iDir)
          run("ls -l *")
          run("mv "+iFName+" "+iFName+".tar"); 
          run("tar -xf "+iFName+".tar");
          switchToDir(True, origWD)
        else :
          run("mv "+iFile+" "+iFile+".tar");
          run("tar -xf "+iFile+".tar");
 
  # Switch to the working directory if we need to do so after unpacking
  #switchToWork(not bool(args.pack_from_work), args)
  switchToWork(True, args)
  
  # Run the provided command
  #run("pwd");
  #if(args.command):
  # If additional environment variables are provided, set them
  if(args.environment):
    # Add each pair of values in args.environment to os.environment
    for i in range(0, len(args.environment), 2) :
      os.environ[args.environment[i]] = args.environment[i+1];
  
  run("pwd")
  run("ls -l *")
  run("tree")
  os.system("echo 'Executing Command'")
  run(args.command);
  run("pwd")
  run("ls -l *")
  run("tree")
  #run("du -h")
  
  # If we're not unpacking from the work directory, switch back to root
  switchToDir(not bool(args.pack_from_work), origWD)
  
  run("ls -l *")
  run("du -h")

  # If the output files were specified, pack them
  if(args.outputs):
    os.system("echo 'Packing Outputs'")
    for o in args.outputs:
      for oFile in o.split(" "):
        run("ls -l "+oFile)
        run("tar -cf "+oFile+".tar "+oFile);
        #run("rm -rf "+oFile)
        run("mv "+oFile+" "+oFile+".back");
        run("mv "+oFile+".tar "+oFile);
#  
#  #sys.exit(-5);

def switchToWork(doNow, args) :
  if(doNow) :
     if(args.work_dir and (args.work_dir != "")) : 
       run("mkdir -p "+args.work_dir)
       #run("cd "+args.work_dir)
       os.chdir(args.work_dir)
     run("pwd")

def switchToDir(doNow, dir) :
  if(doNow) :
     if(dir != "") :
       run("mkdir -p "+dir)
       os.chdir(dir)
       #run("cd "+dir)
       run("pwd")

 
def run(cmd):
  os.system("echo '"+cmd+"'")
  os.system(cmd)

if __name__ == "__main__":
  main(sys.argv[1:])


