#!/usr/apps/python2.7.3/bin/python
# -*- coding: iso-8859-1 -*-

import sys
import argparse
import os

def main(argv):
  parser = argparse.ArgumentParser(description='Package programs for Swift commands')
  parser.add_argument('-sight_dir',  help='Directory where Sight is installed')
  parser.add_argument('-struct_dir',  help='Directory that contains the target structure file')
  parser.add_argument('-layout_dir',  help='Directory where we will place the laid out log representation')
  parser.add_argument('-environment', nargs='*', help='Environment of command')
  args = parser.parse_args()

  if(args.sight_dir):
   print "sight_dir "+args.sight_dir

  if(args.struct_dir):
   print "struct_dir "+args.struct_dir

  if(args.layout_dir):
   print "layout_dir "+args.layout_dir

  if(args.environment):
    # The length of the environment array must be even
    if(len(args.environment)%2 != 0) :
      print "ERROR: provided environment has an odd number of values! environment="+",".join(args.environment)
      sys.exit(-1)

    print "Environment"
    for i in range(0, len(args.environment), 2) :
      print args.environment[i]+" => "+args.environment[i+1]

  # If additional environment variables are provided, set them
  if(args.environment):
    # Add each pair of values in args.environment to os.environment
    for i in range(0, len(args.environment), 2) :
      os.environ[args.environment[i]] = args.environment[i+1];
 
  run(args.sight_dir+"/slayout "+args.struct_dir)

  run("mv "+args.struct_dir+" "+args.layout_dir);

  run("rm "+args.layout_dir+"/structure")


def run(cmd):
  print cmd
  os.system(cmd)

if __name__ == "__main__":
  main(sys.argv[1:])


