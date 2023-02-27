#########################################################################
# python script to run the writeBTagNorms executable via job submission #
#########################################################################

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
from eventselector import event_selections
from eventflattener import year_from_samplelist


def get_variations(btagyear):
  if btagyear not in ['2016','2017','2018']:
    raise Exception('ERROR in get_variations: year {} invalid'.format(btagyear))
  variations = ([
	"hf","lf","hfstats1","hfstats2",
	"lfstats1","lfstats2","cferr1","cferr2",
	"jes",
	"jesAbsolute_"+btagyear, "jesAbsolute",
        "jesBBEC1_"+btagyear, "jesBBEC1",
	"jesEC2_"+btagyear, "jesEC2",
        "jesFlavorQCD",
	"jesHF_"+btagyear, "jesHF",
	"jesRelativeBal", "jesRelativeSample_"+btagyear
  ])
  return variations


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Event binning')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
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

  # check if executable is present
  exe = './writeBTagNorms'
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

  # parse variations
  btagyear = year_from_samplelist(args.samplelist)
  if( btagyear=='2016PreVFP' or btagyear=='2016PostVFP' ): btagyear = '2016'
  variations = ','.join(get_variations(btagyear))

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    event_selections, variations,
                    args.nevents )
    print(command)
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_writebtagnorms', commands,
                                      cmssw_version=CMSSW_VERSION)
