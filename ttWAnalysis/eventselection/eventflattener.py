#########################################################################
# python script to run the eventflattener executable via job submission #
#########################################################################

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
from eventselector import event_selections, selection_types, variations


def year_from_samplelist( slname ):
    ### small utility function to extract the year from a sample list name.
    # only needed to set the correct fake rate map.
    if( '2016PreVFP' in slname ): return '2016PreVFP'
    elif( '2016PostVFP' in slname ): return '2016PostVFP'
    elif( '2017' in slname ): return '2017'
    elif( '2018' in slname ): return '2018'
    else:
        raise Exception('ERROR: could not retrieve year'
                        +' from sample list name "{}"'.format(slname))


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Event flattening')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections)
  parser.add_argument('--selection_type', default='tight', choices=selection_types)
  parser.add_argument('--variation', default='nominal', choices=variations)
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
    print('WARNING: output directory already exists. Clean it? (y/n)')
    go=raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+args.outputdir)

  # check if executable is present
  exe = './eventflattener'
  if not os.path.exists(exe):
    raise Exception('ERROR: {} executable was not found.'.format(exe))

  # make output directory
  os.makedirs(args.outputdir)

  # check samples
  samples = readsamplelist( args.samplelist, sampledir=args.inputdir )
  nsamples = samples.number()
  print('Found {} samples:'.format(nsamples))
  print(samples)

  # set and check fake rate maps
  frmapyear = year_from_samplelist( args.samplelist )
  muonfrmap = os.path.join(args.frdir,'fakeRateMap_data_muon_'+frmapyear+'_mT.root')
  electronfrmap = os.path.join(args.frdir,'fakeRateMap_data_electron_'+frmapyear+'_mT.root')
  if args.selection_type=='fakerate':
    if not os.path.exists(muonfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(muonfrmap))
    if not os.path.exists(electronfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electronfrmap))

  # set and check charge flip maps
  cfmapyear = year_from_samplelist( args.samplelist )
  electroncfmap = None
  if args.selection_type=='chargeflips':
    if args.cfdir is None:
      raise Exception('ERROR: charge flip dir must be specified for selection type chargeflips.')
    electroncfmap = os.path.join(args.cfdir,'chargeFlipMap_MC_electron_'+frmapyear+'.root')
    if not os.path.exists(electronfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electronfrmap)) 

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {} {} {} {} {}'.format(
		    args.inputdir, args.samplelist, i, args.outputdir,
                    args.event_selection, args.selection_type, args.variation,
		    muonfrmap, electronfrmap, electroncfmap, args.nevents )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_eventflattener', commands,
                                      cmssw_version=CMSSW_VERSION )
