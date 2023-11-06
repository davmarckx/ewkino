######################################################
# Simple folder merger for the output of runanalysis #
######################################################
# Use case: merge one (source) output folder into another (target) output folder.
# Note that if there are duplicate files, 
# those in the target folder will be overwritten.

import sys
import os

if __name__=='__main__':

  # settings
  sourcedir = sys.argv[1]
  targetdir = sys.argv[2]

  # make the commands
  cmds = []
  cmds.append( 'cp -rl {} {}'.format(os.path.join(sourcedir,'*'),targetdir) )
  cmds.append( 'rm -r {}'.format(sourcedir) )
  # run the commands
  for cmd in cmds:
    os.system(cmd)
