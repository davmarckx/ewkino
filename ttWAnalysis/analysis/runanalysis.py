######################################################################
# python script to run the runanalysis executable via job submission #
######################################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_12_4_6' # newer version needed for BDT evaluation
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
from samplelisttools import readsamplelist
from variabletools import read_variables, write_variables_txt
sys.path.append(os.path.abspath('../eventselection'))
from eventselector import event_selections, selection_types, variations
from eventflattener import year_from_samplelist

# list of default systematics to include
default_systematics = ([
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
  "trigger",
  "prefire",
  # b-tagging
  "bTag_shape",
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
  # extra uncertainties for high jet multiplicities
  "njets",
  "nbjets",
  # shape uncertainties on fake rate
  "efakeratenorm",
  "efakeratept",
  "efakerateeta",
  "mfakeratenorm",
  "mfakeratept",
  "mfakerateeta"
])


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Run main analysis code')
  parser.add_argument('-i', '--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('-s', '--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--output_append', default=False, action='store_true')
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath)
  parser.add_argument('--event_selection', required=True,
    choices=event_selections + ['all'], nargs='+')
  parser.add_argument('--selection_type', default='tight',
    choices=selection_types + ['all'], nargs='+')
  parser.add_argument('--frdir', default=None, type=apt.path_or_none)
  parser.add_argument('--cfdir', default=None, type=apt.path_or_none)
  parser.add_argument('--bdt', default=None, type=apt.path_or_none)
  parser.add_argument('--bdtcut', default=None, type=float)
  parser.add_argument('-n', '--nevents', default=0, type=int)
  parser.add_argument('-f', '--forcenevents', default=False, action='store_true')
  parser.add_argument('--splitprocess', default=None)
  # process name to split at particle level (i.e. usually TTW in this analysis)
  # - if None, no process will be split at particle level.
  #   (this is the usual workflow for a regular inclusive measurement.)
  # - if it is specified and splitvarfile is None, the automatic setting will be used.
  #   (the specified process will be split according to the provided single or double variable.)
  # - if it is specified for and splitvarfile as well,
  #   the specified process will be split according to the variables in splitvarfile.
  parser.add_argument('--splitvarfile', default=None)
  parser.add_argument('--systematics', default=None, nargs='+')
  # use 'default' to include list of default systematics (defined above)
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
    if not args.output_append:
      print('WARNING: output directory {} already exists. Clean it? (y/n)'.format(args.outputdir))
      go=raw_input()
      if not go=='y': sys.exit()
      os.system('rm -r '+args.outputdir)
    else:
      print('WARNING: output directory {} already exists. Appending...'.format(args.outputdir))
  if not os.path.exists(args.variables):
    raise Exception('ERROR: variable file {} does not exist.'.format(args.variables))
  variables_ext = os.path.splitext(args.variables)[1]
  if not variables_ext=='.json':
    raise Exception('ERROR: variable file {} should be .json.'.format(args.variables))
  if 'all' in args.event_selection: args.event_selection = event_selections
  event_selections = ','.join(args.event_selection)
  if 'all' in args.selection_type: args.selection_type = selection_types
  selection_types = ','.join(args.selection_type)

  # check if executable is present
  exe = './runanalysis'
  if not os.path.exists(exe):
    raise Exception('ERROR: {} executable was not found.'.format(exe))

  # make output directory
  if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)

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
  if 'fakerate' in selection_types:
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
  if 'chargeflips' in selection_types:
    if args.cfdir is None:
      raise Exception('ERROR: charge flip dir must be specified for selection type chargeflip.')
    electroncfmap = os.path.join(args.cfdir,'chargeFlipMap_MC_electron_'+frmapyear+'.root')
    if not os.path.exists(electroncfmap):
      raise Exception('ERROR: fake rate map {} does not exist'.format(electroncfmap))

  # check bdt weight file
  bdt = 'none'
  bdtcut = -99
  if( args.bdt is not None ):
    if not os.path.exists(args.bdt):
      raise Exception('ERROR: BDT file {} does not exist'.format(args.bdt))
    bdt = args.bdt
  if( args.bdtcut is not None ):
    bdtcut = args.bdtcut
  if( args.bdt is None and args.bdtcut is not None ):
    msg = 'ERROR: you have specified a BDT cut value but no BDT weights file.'
    raise Exception(msg)

  # setup variable for splitting at particle level if requested
  splitvartxt = 'none'
  if( args.splitprocess is not None ):
    splitvartxt = 'auto'
    if( args.splitvarfile is not None ):
      splitvarlist = read_variables( args.splitvarfile )
      splitvartxt = args.splitvarfile.replace('.json','.txt')
      write_variables_txt( splitvarlist, splitvartxt )

  # parse systematics
  systematics = args.systematics
  if systematics is None: systematics = ['none']
  if 'default' in systematics: systemtics = default_systematics
  if len(systematics)==0: systematics = ['none']
  systematics = ','.join(systematics)

  # loop over input files and submit jobs
  commands = []
  for i in range(nsamples):
    # make the basic command
    command = exe + ' {} {} {} {} {} {} {} {} {} {} {} {} {} {}'.format(
                    args.inputdir, args.samplelist, i, args.outputdir,
                    variablestxt, event_selections, selection_types,
                    muonfrmap, electronfrmap, electroncfmap, 
                    args.nevents, args.forcenevents, bdt, bdtcut )
    command += ' {}'.format(splitvartxt)
    command += ' {}'.format(systematics)
    commands.append(command)

  # submit the jobs
  if args.runmode=='local':
    for command in commands: os.system(command)
  elif args.runmode=='condor':
    ct.submitCommandsAsCondorCluster( 'cjob_runanalysis', commands,
                                      cmssw_version=CMSSW_VERSION ) 
