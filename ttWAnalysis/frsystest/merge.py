######################################################
# Simple folder merger for the output of eventbinner #
######################################################
# Run after eventbinner and before mergehists,
# in order to merge the folders <year>_sim and <year>_data into one.
# The files in those folders and the histograms they contain are left untouched.

import sys
import os

def mergefolders( inputdir, years, remove ):
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

def mergehists( selfiles, outputfile ):

  # make output directory
  outputdirname, outputbasename = os.path.split(outputfile)
  if not os.path.exists(outputdirname):
    os.makedirs(outputdirname)

  # do hadd to merge histograms
  # with same process, variable, and systematic.
  print('Running hadd...')
  sys.stdout.flush()
  cmd = 'hadd -f {}'.format(outputfile)
  for f in selfiles: cmd += ' {}'.format(f)
  os.system(cmd)
  sys.stdout.flush()


if __name__=='__main__':

  # settings
  inputdir = sys.argv[1]
  years = ['2016PreVFP','2016PostVFP','2017','2018']

  # first merge folders
  mergefolders(inputdir, years, True)

  # loop over years, event selections and selection types
  for year in years:
    for eventselection in os.listdir(os.path.join(inputdir,year)):
        for selectiontype in os.listdir(os.path.join(inputdir,year,eventselection)):
            thisdir = os.path.join(inputdir,year,eventselection,selectiontype)
            # get files to merge
            selfiles = ([os.path.join(thisdir,f) for f in os.listdir(thisdir)
                         if f[-5:]=='.root'])
            # set output file
            outputfile = os.path.join(thisdir, 'merged.root')
            # merge the files
            mergehists( selfiles, outputfile )
