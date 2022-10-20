######################################################
# Submit the trigger efficiency measurement as a job #
######################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
from samplelisttools import readsamplelist
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Trigger efficiency measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--dimension', required=True, type=int, choices=[1,2])
  parser.add_argument('--event_selection', required=True )
  parser.add_argument('--pt_threshold_id', required=True )
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
  exe = './filltrigger1d'
  if args.dimension == 2:
    exe = './filltrigger2d'
  if not os.path.exists(exe):
    raise Exception('ERROR: {} executable was not found.'.format(exe))

  # make output directory
  os.makedirs(args.outputdir)

  # check samples
  samples = readsamplelist( args.samplelist, sampledir=args.inputdir )
  nsamples = samples.number()
  print('Found {} samples.'.format(nsamples))
  print('Full list of samples:')
  print(samples)

  # loop over input files and make commands
  inputfiles = sorted([s.path for s in samples.samples])
  commands = []
  for inputfile in inputfiles:
    output_file_name = inputfile.split('/')[-1].rstrip('.root')+'_trigger_histograms.root'
    output_file_path = os.path.join(args.outputdir,output_file_name)
    command = '{} {} {} {} {} {}'.format(exe, inputfile, output_file_path, 
              args.event_selection, args.pt_threshold_id, args.nevents)
    commands.append(command)

  # run jobs
  if args.runmode=='local':
    for cmd in commands: os.system(cmd)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_triggerefficiency', commands,
                                      cmssw_version=CMSSW_VERSION )
