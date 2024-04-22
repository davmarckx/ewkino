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
CMSSW_VERSION = '~/CMSSW_10_2_13_combine/CMSSW_10_2_13'
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt

if __name__=='__main__':
    
  # parse arguments
  parser = argparse.ArgumentParser(description='Calculate impacts')
  parser.add_argument('-w', '--workspace', required=True, type=os.path.abspath)
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--expectsignal', default=False, action='store_true')
  parser.add_argument('--pois', default=['auto'], nargs='+')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # if workspace is a directory, find workspaces in it
  if not args.workspace.endswith('.root'):
    print('Looking for workspaces in directory {}'.format(args.workspace))
    workspaces = [f for f in os.listdir(args.workspace)
                  if( (f.startswith('datacard_') or f.startswith('dc_combined_')) and f.endswith('.root') )]
    print('Will submit impact plot generation for the following workspaces ({}):'.format(len(workspaces)))
    for w in workspaces: print('  - {}'.format(w))
    print('Continue? (y/n)')
    go = raw_input()
    if go!='y': sys.exit()

    for w in workspaces:
      cmd = 'python impacts.py'
      cmd += ' --workspace {}'.format(os.path.join(args.workspace,w))
      if args.usedata: cmd += ' --usedata'
      if args.expectsignal: cmd += ' --expectsignal'
      cmd += ' --pois {}'.format(' '.join(args.pois))
      cmd += ' --runmode {}'.format(args.runmode)
      os.system(cmd)
    sys.exit()

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
