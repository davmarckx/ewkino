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
  parser = argparse.ArgumentParser(description='Calculate impacts')
  parser.add_argument('--workspace', required=True, type=os.path.abspath)
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--expectsignal', default=False, action='store_true')
  parser.add_argument('--pois', default=['r'], nargs='+')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # split workspace in directory and file name
  datacarddir, workspace = os.path.split(args.workspace)
  card = workspace.replace('.root','.txt')

  # get the commands
  commands = cbt.get_impacts_commands( datacarddir, card, workspace=workspace,
               usedata=args.usedata, expectsignal=args.expectsignal, pois=args.pois )

  # run the commands
  if( args.runmode=='condor' ):
    ct.submitCommandsAsCondorJob( 'cjob_combine_impacts', commands,
                                  cmssw_version=CMSSW_VERSION )
  else:
    script_name = 'local_combine_impacts.sh'
    ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
    with open( script_name, 'a' ) as script:
      for c in commands: script.write(c+'\n')
    os.system('bash {}'.format(script_name))
