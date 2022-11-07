###########################################
# merger for the output of runanalysis.py #
###########################################

import sys
import os
import json
import ROOT
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
import histtools as ht
from samplelisttools import readsamplelist
sys.path.append(os.path.abspath('../eventselection'))
from eventselector import event_selections, selection_types, variations
from eventflattener import year_from_samplelist
from mergehists import rename_processes_in_file


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Merge analysis histogram files')
  parser.add_argument('--directory', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True, choices=['2016PreVFP','2016PostVFP','2017','2018'])
  parser.add_argument('--region', required=True, choices=event_selections)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--npmode', required=True, choices=['npfromsim','npfromdata'])
  parser.add_argument('--rename', default=None, type=apt.path_or_none)
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.directory))
  if( args.rename is not None and not os.path.exists(args.rename) ):
    raise Exception('ERROR: rename file {} does not exist.'.format(args.rename))

  # check if all required input directories exist
  required_dirs = []
  if args.npmode=='npfromsim':
    required_dirs.append(os.path.join(args.directory,'{}_sim'.format(args.year),args.region,'tight'))
    #required_dirs.append(os.path.join(args.directory,'{}_data'.format(args.year),args.region,'tight'))
  else:
    required_dirs.append(os.path.join(args.directory,'{}_sim'.format(args.year),args.region,'prompt'))
    required_dirs.append(os.path.join(args.directory,'{}_sim'.format(args.year),args.region,'fakerate'))
    #required_dirs.append(os.path.join(args.directory,'{}_data'.format(args.year),args.region,'tight'))
    #required_dirs.append(os.path.join(args.directory,'{}_data'.format(args.year),args.region,'fakerate'))
  for d in required_dirs:
    if not os.path.exists(d):
      raise Exception('ERROR: required directory {} does not exist.'.format(d))

  # handle job submission if requested
  if args.runmode=='condor':
    cmd = 'python merge.py'
    cmd += ' --directory '+args.directory
    cmd += ' --year '+args.year
    cmd += ' --region '+args.region
    cmd += ' --outputfile '+args.outputfile
    cmd += ' --npmode '+args.npmode
    if args.rename is not None: cmd += ' --rename '+args.rename
    cmd += ' --runmode local'
    ct.submitCommandAsCondorJob( 'cjob_merge', cmd,
                                 cmssw_version=CMSSW_VERSION )
    sys.exit()

  # select files to merge
  selfiles = []
  for d in required_dirs:
    for f in os.listdir(d):
      if f.endswith('.root'):
        selfiles.append(os.path.join(d,f))

  # printouts
  print('Will merge the following files:')
  for f in selfiles: print('  - {}'.format(f))
  print('into {}'.format(args.outputfile))
  sys.stdout.flush()

  # make output directory
  outputdirname, outputbasename = os.path.split(args.outputfile)
  if not os.path.exists(outputdirname):
    os.makedirs(outputdirname)

  # make a temporary directory to store intermediate results
  tempdir = os.path.join(args.directory, 'temp_{}_{}_{}'.format(args.year,args.region,args.npmode))
  print('Making temporary directory {}'.format(tempdir))
  if not os.path.exists(tempdir):
    os.makedirs(tempdir)

  # copy all files to temporary directory
  tempfiles = []
  for i,f in enumerate(selfiles):
    filename = os.path.split(f)[1]
    tempfile = os.path.join(tempdir,'temp{}.root'.format(i))
    os.system('cp {} {}'.format(f, tempfile))
    tempfiles.append(tempfile)

  # rename processes if requested
  if args.rename is not None:
    print('Renaming processes in all selected files...')
    with open(args.rename,'r') as f:
      renamedict = json.load(f)
    for i,f in enumerate(tempfiles):
      print('  - file {}/{}'.format(i+1,len(tempfiles)))
      sys.stdout.flush()
      rename_processes_in_file(renamedict,f, mode='rootmv')

  # do hadd to merge histograms
  # with same process, variable, and systematic.
  print('Running hadd...')
  cmd = 'hadd -f {}'.format(args.outputfile)
  for f in tempfiles: cmd += ' {}'.format(f)
  os.system(cmd)

  # delete temporary files
  os.system('rm -r {}'.format(tempdir))

  # clip all resulting histograms to minimum zero
  print('Clipping histograms to minimum zero...')
  ht.clipallhistograms(args.outputfile)
