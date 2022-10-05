######################################################
# Simple folder merger for the output of eventbinner #
######################################################
# Run after eventbinner and before mergehists,
# in order to merge the folders <year>_sim and <year>_data into one.

import sys
import os

if __name__=='__main__':

  # settings
  inputdir = sys.argv[1]
  years = ['2016PreVFP','2016PostVFP','2017','2018']

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
    cmds.append( 'mv {} {}'.format(os.path.join(datadir,'*'),simdir) )
    cmds.append( 'rm -r {}'.format(datadir) )
    cmds.append( 'mv {} {}'.format(simdir,mergeddir) )
    # run the commands
    for cmd in cmds:
      os.system(cmd)
