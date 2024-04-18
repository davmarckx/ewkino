######################################################
# Simple folder merger for the output of runanalysis #
######################################################
# Simply calls same functionality in eventselection folder.
# Run after analysis and before merging,
# in order to merge the folders <year>_sim and <year>_data into one.
# The files in those folders and the histograms they contain are left untouched.

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
