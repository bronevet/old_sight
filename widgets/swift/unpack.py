#!/usr/apps/python2.7.3/bin/python

import sys
import argparse
import os

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)

def main(argv):
  abspaths = []
  for i in range(0, len(argv)) :
    abspaths += [os.path.abspath(argv[i])]; 

  print str(abspaths)
 
  for i in range(0, len(argv)):
    run("mkdir tmp")
    os.chdir("tmp")
    run("cp "+abspaths[i]+" "+os.path.basename(abspaths[i])+".tar");
    run("tar -xf "+os.path.basename(abspaths[i])+".tar");
    run("mv "+os.path.basename(abspaths[i])+" ../"+os.path.basename(abspaths[i])+".unpacked");
   
def run(cmd):
  print cmd
  os.system(cmd)

if __name__ == "__main__":
  main(sys.argv[1:])


