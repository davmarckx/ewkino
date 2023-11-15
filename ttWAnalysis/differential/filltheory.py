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


# list of systematics to include (hard-coded for now, maybe extend later)
systematics = ([
  # scale uncertainties
  "fScaleShape",
  "fScaleNorm",
  "fScaleTotal",
  "rScaleShape",
  "rScaleNorm",
  "rScaleTotal",
  "rfScalesShape",
  "rfScalesNorm",
  "rfScalesTotal",
  # scale envelopes
  "qcdScalesShapeVar",
  "qcdScalesTotalVar",
  "qcdScalesNorm",
  # pdf rms
  "pdfShapeVar",
  "pdfTotalVar",
  "pdfNorm",
  # parton shower uncertainties
  "isrShape",
  "isrNorm",
  "isrTotal",
  "fsrShape",
  "fsrNorm",
  "fsrTotal"
])
#systematics = (['eft'])


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Fill theoretical differential xsections')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
  parser.add_argument('--selection_type', default=['particlelevel'], choices=['particlelevel'], nargs='+')
  parser.add_argument('--nevents', default=0, type=int)
  parser.add_argument('--reweightVar', default=None)
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
  event_selections = ','.join(args.event_selection)
  selection_types = ','.join(args.selection_type)

  # check if executable is present
  exe = './filltheory'
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

  # parse systematics
  if len(systematics)==0: systematics = ['none']
  systematics = ','.join(systematics)

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    variablestxt, event_selections, selection_types, args.nevents,
                    systematics )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_filltheory', commands,
                                      cmssw_version=CMSSW_VERSION ) 
