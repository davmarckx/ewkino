######################################################
# Simple folder merger for the output of eventbinner #
######################################################
# Simply calls same functionality in eventselection folder

import sys
import os

if __name__=='__main__':

  # settings
  inputdir = os.path.abspath(sys.argv[1])
  remove = "1"
  if len(sys.argv) == 3:
    remove = sys.argv[2]

  # call mergefolders in eventselection
  cmd = 'python ../eventselection/mergefolders.py {} {}'.format(inputdir, remove)
  os.system(cmd)
