######################################################################
# python script to run the runanalysis executable via job submission #
######################################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
from samplelisttools import readsamplelist
from variabletools import read_variables, write_variables_txt
sys.path.append(os.path.abspath('../eventselection'))
from eventselector import event_selections, selection_types, variations
from eventflattener import year_from_samplelist

# list of systematics to include (hard-coded for now, maybe extend later)
systematics = ([
  # JEC and related
  "JEC", 
  "JER",
  "Uncl",
  #"JECAll" # not in current samples
  "JECGrouped",
  # via standard reweighting
  "muonReco",
  "electronReco",
  "muonIDSyst",
  "muonIDStat",
  "electronIDSyst",
  "electronIDStat",
  "pileup",
  "prefire",
  # b-tagging
  #"bTag_shape" # not yet implemented
  # scale uncertainties
  "fScale",
  "fScaleNorm",
  "rScale",
  "rScaleNorm",
  "rfScales",
  "rfScalesNorm",
  # envelopes and related
  "pdfShapeVar",
  "pdfNorm",
  "qcdScalesShapeVar",
  "qcdScalesNorm",
  # parton shower uncertainties
  "isrShape",
  "isrNorm",
  "fsrShape",
  "fsrNorm",
])


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Run main analysis code')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
  parser.add_argument('--selection_type', default='tight', choices=selection_types, nargs='+')
  parser.add_argument('--frdir', default=None, type=apt.path_or_none)
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
  event_selections = ','.join(args.event_selection)
  selection_types = ','.join(args.selection_type)

  # check if executable is present
  exe = './runanalysis'
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

  # convert variables to txt for reading in c++
  varlist = read_variables( args.variables )
  variablestxt = args.variables.replace('.json','.txt')
  write_variables_txt( varlist, variablestxt )

  # set and check fake rate maps
  frmapyear = year_from_samplelist( args.samplelist )
  muonfrmap = None
  electronfrmap = None
  if args.selection_type=='fakerate':
    if args.frdir is None:
      raise Exception('ERROR: fake rate dir must be specified for selection type fakerate.')
    muonfrmap = os.path.join(args.frdir,'fakeRateMap_data_muon_'+frmapyear+'_mT.root')
    electronfrmap = os.path.join(args.frdir,'fakeRateMap_data_electron_'+frmapyear+'_mT.root')
    if not os.path.exists(muonfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(muonfrmap))
    if not os.path.exists(electronfrmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electronfrmap))

  # parse systematics
  systematics = ','.join(systematics)

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the command
    command = exe + ' {} {} {} {} {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    variablestxt, event_selections, selection_types,
                    muonfrmap, electronfrmap, args.nevents, systematics )
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_runanalysis', commands,
                                      cmssw_version=CMSSW_VERSION ) 
