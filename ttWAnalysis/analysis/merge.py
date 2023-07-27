###########################################
# merger for the output of runanalysis.py #
###########################################

import sys
import os
import json
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
sys.path.append(os.path.abspath('../eventselection'))
from eventselector import event_selections, selection_types
from mergehists import mergehists


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Merge analysis histogram files')
  parser.add_argument('--directory', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--npmode', required=True, choices=['npfromsim','npfromdata','npfromdatasplit'])
  parser.add_argument('--cfmode', required=True, choices=['cffromsim','cffromdata'])
  parser.add_argument('--rename', default=None, type=apt.path_or_none)
  parser.add_argument('--renamemode', default='fast', choices=['custom','rootmv','fast'])
  parser.add_argument('--decorrelate', default=None, type=apt.path_or_none)
  parser.add_argument('--decorrelatemode', default='fast', choices=['fast'])
  parser.add_argument('--decorrelateyear', default=None)
  parser.add_argument('--decorrelateyears',
    default=['2016PreVFP','2016PostVFP','2017', '2018'], nargs='+')
  parser.add_argument('--selectmode', default='fast', choices=['custom','fast','noselect'])
  parser.add_argument('--doclip', action='store_true')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.directory))
  if( args.rename is not None and not os.path.exists(args.rename) ):
    raise Exception('ERROR: rename file {} does not exist.'.format(args.rename))

  # check if required directories exist
  dirstomerge = []
  if( args.npmode=='npfromsim' and args.cfmode=='cffromsim' ):
    dirstomerge.append(os.path.join(args.directory,'tight'))
  elif( args.npmode=='npfromdata' and args.cfmode=='cffromsim' ):
    dirstomerge.append(os.path.join(args.directory,'prompt'))
    dirstomerge.append(os.path.join(args.directory,'fakerate'))
  elif( args.npmode=='npfromsim' and args.cfmode=='cffromdata' ):
    dirstomerge.append(os.path.join(args.directory,'chargegood'))
    dirstomerge.append(os.path.join(args.directory,'chargeflips'))
  elif( args.npmode=='npfromdata' and args.cfmode=='cffromdata' ):
    dirstomerge.append(os.path.join(args.directory,'irreducible'))
    dirstomerge.append(os.path.join(args.directory,'fakerate'))
    dirstomerge.append(os.path.join(args.directory,'chargeflips'))
  elif( args.npmode=='npfromdatasplit' and args.cfmode=='cffromdata' ):
    dirstomerge.append(os.path.join(args.directory,'irreducible'))
    dirstomerge.append(os.path.join(args.directory,'efakerate'))
    dirstomerge.append(os.path.join(args.directory,'mfakerate'))
    dirstomerge.append(os.path.join(args.directory,'chargeflips'))
  else:
    msg = 'ERROR: this combination of npmode and cfmode was not yet implemented.'
    raise Exception(msg)
  # do the check
  for d in dirstomerge:
    if not os.path.exists(d):
      raise Exception('ERROR: required directory {} does not exist.'.format(d))

  # handle job submission if requested
  if args.runmode=='condor':
    cmd = 'python merge.py'
    cmd += ' --directory '+args.directory
    cmd += ' --outputfile '+args.outputfile
    cmd += ' --npmode '+args.npmode
    cmd += ' --cfmode '+args.cfmode
    if args.rename is not None: 
      cmd += ' --rename '+args.rename
      cmd += ' --renamemode '+args.renamemode
    if args.decorrelate is not None:
      cmd += ' --decorrelate '+args.decorrelate
      cmd += ' --decorrelatemode '+args.decorrelatemode
      cmd += ' --decorrelateyear '+args.decorrelateyear
      cmd += ' --decorrelateyears '
      for y in args.decorrelateyears: cmd += ' '+y
    cmd += ' --selectmode '+args.selectmode
    if args.doclip: cmd += ' --doclip'
    cmd += ' --runmode local'
    ct.submitCommandAsCondorJob( 'cjob_merge', cmd,
                                 cmssw_version=CMSSW_VERSION )
    sys.exit()

  # select files to merge
  selfiles = []
  for d in dirstomerge:
    for f in os.listdir(d):
      if f.endswith('.root'):
        selfiles.append(os.path.join(d,f))

  # call main function
  mergehists( selfiles, args )
