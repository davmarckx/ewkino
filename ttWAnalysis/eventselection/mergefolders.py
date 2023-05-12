######################################################
# Simple folder merger for the output of eventbinner #
######################################################
# Run after eventbinner and before mergehists,
# in order to merge the folders <year>_sim and <year>_data into one.
# The files in those folders and the histograms they contain are left untouched.

import sys
import os

def mergefolders( inputdir, years,remove ):
  # loop over years
  for year in years:
    simdir = os.path.join(inputdir,year+'_sim')
    datadir = os.path.join(inputdir,year+'_data')
    mergeddir = os.path.join(inputdir,year)
    # check input directories
    if not os.path.exists(simdir):
      print('ERROR: directory {} not found, skipping...'.format(simdir))
      continue
    if not os.path.exists(datadir):
      print('ERROR: directory {} not found, skipping...'.format(datadir))
      continue
    # make the commands
    cmds = []
    if remove:
      cmds.append( 'cp -rl {} {}'.format(os.path.join(datadir,'*'),simdir) )
      cmds.append( 'rm -r {}'.format(datadir) )
      cmds.append( 'mv {} {}'.format(simdir,mergeddir) )
      # run the commands
      for cmd in cmds:
        os.system(cmd)
    else:
      cmds.append( 'mkdir {}'.format(mergeddir) )
      cmds.append( 'cp -rl {} {}'.format(os.path.join(datadir,'*'),mergeddir) )
      cmds.append( 'cp -rl {} {}'.format(os.path.join(simdir,'*'),mergeddir) )
      # run the commands
      for cmd in cmds:
        os.system(cmd)

if __name__=='__main__':

  # settings
  inputdir = sys.argv[1]
  if len(sys.argv) == 2:
      remove = (sys.argv[2] == "1")
  else:
      remove = False
  years = ['2016PreVFP','2016PostVFP','2017','2018']

  # call main function
  mergefolders(inputdir, years, remove)
