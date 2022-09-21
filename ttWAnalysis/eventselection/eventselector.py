########################################################################
# python script to run the eventselector executable via job submission #
########################################################################

import sys
import os
import glob
import argparse
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append('../../Tools/python')
from samplelisttools import readsamplelist


# fixed settings
event_selections = (['wzcontrolregion','zzcontrolregion','zgcontrolregion',
                     'signalregion_trilepton',
                     'nonprompt_trilepton_noossf', 'nonprompt_trilepton_noz',
                     'nonprompt_trilepton', 'nonprompt_dilepton'])
selection_types = ['tight','prompt','fakerate','2tight']
variations = ['nominal','all','JECDown','JECUp','JERDown','JERUp','UnclDown','UnclUp']

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Event selection')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections)
  parser.add_argument('--selection_type', default='tight', choices=selection_types)
  parser.add_argument('--variation', default='nominal', choices=variations)
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
    print('WARNING: output directory already exists. Clean it? (y/n)')
    go=raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+args.outputdir)

  # check if executable is present
  exe = './eventselector'
  if not os.path.exists(exe):
    raise Exception('ERROR: {} executable was not found.'.format(exe))

  # create output directory
  os.makedirs(args.outputdir)

  # check samples
  samples = readsamplelist( args.samplelist, sampledir=args.inputdir )
  nsamples = samples.number()
  print('Found {} samples:'.format(nsamples))
  print(samples)

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    args.event_selection, args.selection_type, args.variation,
                    args.nevents )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_eventselector', commands,
                                      cmssw_version=CMSSW_VERSION )
