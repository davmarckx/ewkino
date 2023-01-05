######################################################################
# python script to run the eventbinner executable via job submission #
######################################################################

import sys
import os
import glob
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
from samplelisttools import readsamplelist
from variabletools import read_variables, write_variables_txt
from eventselector import event_selections, selection_types, variations
from eventflattener import year_from_samplelist


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Event flattening')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
  parser.add_argument('--selection_type', default=['tight'], choices=selection_types, nargs='+')
  parser.add_argument('--variation', default=['nominal'], choices=variations, nargs='+')
  parser.add_argument('--frdir', default=None, type=apt.path_or_none)
  parser.add_argument('--cfdir', default=None, type=apt.path_or_none)
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
  if not os.path.exists(args.variables):
    raise Exception('ERROR: variable file {} does not exist.'.format(args.variables))
  variables_ext = os.path.splitext(args.variables)[1]
  if not variables_ext=='.json':
    raise Exception('ERROR: variable file {} should be .json.'.format(args.variables))
  event_selections = '+'.join(args.event_selection)
  selection_types = '+'.join(args.selection_type)
  variations = '+'.join(args.variation)

  # check if executable is present
  exe = './eventbinner'
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

  # convert variables to txt for reading in c++
  varlist = read_variables( args.variables )
  variablestxt = args.variables.replace('.json','.txt')
  write_variables_txt( varlist, variablestxt )

  # set and check fake rate maps
  frmapyear = year_from_samplelist( args.samplelist )
  muonfrmap = None
  electronfrmap = None
  if 'fakerate' in args.selection_type:
    if args.frdir is None:
      raise Exception('ERROR: fake rate dir must be specified for selection type fakerate.')
    muonfrmap = os.path.join(args.frdir,'fakeRateMap_data_muon_'+frmapyear+'_mT.root')
    electronfrmap = os.path.join(args.frdir,'fakeRateMap_data_electron_'+frmapyear+'_mT.root')
    if not os.path.exists(muonfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(muonfrmap))
    if not os.path.exists(electronfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electronfrmap))
  
  # set and check charge flip maps
  cfmapyear = year_from_samplelist( args.samplelist )
  electroncfmap = None
  if 'chargeflips' in args.selection_type:
    if args.cfdir is None:
      raise Exception('ERROR: charge flip dir must be specified for selection type chargeflip.')
    electroncfmap = os.path.join(args.cfdir,'chargeFlipMap_MC_electron_'+cfmapyear+'.root')
    if not os.path.exists(electroncfmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electroncfmap))

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    variablestxt, event_selections, selection_types, variations,
                    muonfrmap, electronfrmap, electroncfmap, args.nevents )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_eventbinner', commands,
                                      cmssw_version='~/CMSSW_12_4_6')#instead of CMSSW_VERSION because TMVA::Experimental needs a new ROOT version
