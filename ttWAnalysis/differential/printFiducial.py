#####################################################################
# python script to run the filltheory executable via job submission #
#####################################################################

import sys
import os
import glob
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_12_4_6' # newer version needed for BDT evaluation
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
from samplelisttools import readsamplelist
sys.path.append(os.path.abspath('../eventselection'))
from variabletools import read_variables, write_variables_txt
from eventselector import event_selections
from eventflattener import year_from_samplelist


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('print events that pass fiducial selection')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
  parser.add_argument('--selection_type', default=['particlelevel'], choices=['particlelevel'], nargs='+')
  parser.add_argument('--nevents', default=0, type=int)
  parser.add_argument('--runmode', default='condor', choices=['condor','local'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
 
  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))
  if not os.path.exists(args.samplelist):
    raise Exception('ERROR: sample list {} does not exist.'.format(args.samplelist))
  if os.path.exists(args.outputdir):
    print('WARNING: output directory {} already exists. Clean it? (y/n)'.format(args.outputdir))
    go=raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+args.outputdir)
  event_selections = ','.join(args.event_selection)
  selection_types = ','.join(args.selection_type)

  # check if executable is present
  exe = './printFiducial'
  if not os.path.exists(exe):
    raise Exception('ERROR: {} executable was not found.'.format(exe))

  # make output directory
  os.makedirs(args.outputdir)

  # check samples
  samples = readsamplelist( args.samplelist, sampledir=args.inputdir )
  nsamples = samples.number()
  print('Found {} samples.'.format(nsamples))
  #print('Full list of samples:')
  #print(samples)

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    event_selections, selection_types, args.nevents )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_printFiduciale', commands,
                                      cmssw_version=CMSSW_VERSION ) 
