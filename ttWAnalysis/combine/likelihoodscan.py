##########################################
# make impact plot for a given workspace #
##########################################

import os
import sys
import json
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_patch1' # temporary
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt

if __name__=='__main__':
    
  # parse arguments
  parser = argparse.ArgumentParser(description='Make a likelihood scan')
  parser.add_argument('--workspace', required=True, type=os.path.abspath)
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--includedata', default=False, action='store_true')
  parser.add_argument('--statonly', default=False, action='store_true')
  parser.add_argument('--includestatonly', default=False, action='store_true')
  parser.add_argument('--pois', default=['r'], nargs='+')
  parser.add_argument('--scansingles', default=False, action='store_true')
  parser.add_argument('--npoints', default=100, type=int)
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # hard-coded arguments
  poiranges = {}
  for poi in args.pois:
    poiranges[poi] = (0.5, 2.)

  # split workspace in directory and file name
  datacarddir, workspace = os.path.split(args.workspace)
  card = workspace.replace('.root','.txt')

  # get the commands
  commands = []
  pois = args.pois
  if not args.scansingles: pois = [args.pois]
  for poiset in args.pois:
    if isinstance(poiset,str): poiset = [poiset]
    for c in cbt.get_likelihoodscan_commands( datacarddir, card, workspace=workspace,
                 usedata=args.usedata, dostatonly=args.statonly,
                 pois=poiset, poiranges=poiranges,
                 npoints=args.npoints ): commands.append(c)
    if( args.includedata and not args.usedata ):
      for c in cbt.get_likelihoodscan_commands( datacarddir, card, workspace=workspace,
                 usedata=True, dostatonly=args.statonly,
                 pois=poiset, poiranges=poiranges,
                 npoints=args.npoints ): commands.append(c)
    if( args.includestatonly and not args.statonly ):
      for c in cbt.get_likelihoodscan_commands( datacarddir, card, workspace=workspace,
                 usedata=args.usedata, dostatonly=True,
                 pois=poiset, poiranges=poiranges,
                 npoints=args.npoints ): commands.append(c)
    if( args.includedata and not args.usedata
        and args.includestatonly and not args.statonly ):
      for c in cbt.get_likelihoodscan_commands( datacarddir, card, workspace=workspace,
                 usedata=True, dostatonly=True,
                 pois=poiset, poiranges=poiranges,
                 npoints=args.npoints ): commands.append(c)

  # run the commands
  if( args.runmode=='condor' ):
    ct.submitCommandsAsCondorJob( 'cjob_combine_likelihoodscan', commands,
                                  cmssw_version=CMSSW_VERSION )
  else:
    script_name = 'local_combine_likelihoodscan.sh'
    ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
    with open( script_name, 'a' ) as script:
      for c in commands: script.write(c+'\n')
    os.system('bash {}'.format(script_name))
