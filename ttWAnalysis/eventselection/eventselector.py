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
event_selections = (['noselection',
                     'signalregion_dilepton_inclusive',
                     'signalregion_dilepton_oviedo',
		     'signalregion_dilepton_ee',
		     'signalregion_dilepton_em',
		     'signalregion_dilepton_me',
		     'signalregion_dilepton_mm',
                     'signalregion_dilepton_pee',
                     'signalregion_dilepton_pem',
                     'signalregion_dilepton_pme',
                     'signalregion_dilepton_pmm',
                     'signalregion_dilepton_nee',
                     'signalregion_dilepton_nem',
                     'signalregion_dilepton_nme',
                     'signalregion_dilepton_nmm',
		     'signalregion_dilepton_plus',
                     'signalregion_dilepton_minus',
		     'signalregion_trilepton',
		     'wzcontrolregion','zzcontrolregion','zgcontrolregion',
		     'trileptoncontrolregion','fourleptoncontrolregion',
                     'npcontrolregion_dilepton_inclusive',
                     'npcontrolregion_dilepton_oviedo',
		     'npcontrolregion_dilepton_ee',
		     'npcontrolregion_dilepton_em',
		     'npcontrolregion_dilepton_me',
		     'npcontrolregion_dilepton_mm',
		     'cfcontrolregion'])
selection_types = (['tight', 'prompt', 'chargegood', 'irreducible',
                    'fakerate', 'efakerate', 'mfakerate', 'chargeflips'])
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
