##########################################################
# merge the output of the trigger efficiency measurement #
##########################################################
# note: proper merging depends on file naming and structuring conventions.

import os
import sys
import ROOT
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of trigger efficiency measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--data_only', action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))

  # first find directories to run on
  directories = []
  required_contents = ['sim','data']
  if args.data_only: required_contents = ['data']
  for root,dirs,files in os.walk(args.inputdir):
    contents = os.listdir(os.path.join(root))
    consider = True
    for c in required_contents:
      if( c not in contents ): consider = False
    if consider: directories.append(root)
  print('Will run on following directories:')
  print(directories)

  # loop over all directories
  for directory in directories:
    # define the merged file containing all histograms to plot
    mergedfilename = 'combined_trigger_histograms.root'
    mergedfilepath = os.path.join(directory,mergedfilename)

    # do merging
    if os.path.exists(mergedfilepath): os.system('rm '+mergedfilepath)
    datadir = os.path.join(directory,'data')
    simdir = os.path.join(directory,'sim')
    datafiles = [os.path.join(datadir,f) for f in os.listdir(datadir) if f[-5:]=='.root']
    simfiles = []
    if not args.data_only: 
      simfiles = [os.path.join(simdir,f) for f in os.listdir(simdir) if f[-5:]=='.root']
    command = 'hadd '+mergedfilepath
    for f in datafiles+simfiles:
      command += ' '+os.path.join(f)
    os.system(command)
