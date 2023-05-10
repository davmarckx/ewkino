#############################################
# run a goodness-of-fit test on a workspace #
#############################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_patch1'


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Run goodness-of-fit test')
    parser.add_argument('--workspaces', required=True, type=os.path.abspath, nargs='+')
    parser.add_argument('--ntoys', default=10, type=int)
    parser.add_argument('--runmode', default='local', choices=['local','condor'])
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
          print('  - {}: {}'.format(arg,getattr(args,arg)))

    # argument checking
    for workspace in args.workspaces:
        if not os.path.exists(workspace):
            raise Exception('ERROR: workspace {} does not exist.'.format(workspace))

    # loop over workspaces
    for workspace in args.workspaces:

        # get directory and datacard from provided workspace
        card = workspace.replace('.root','.txt')
        (datacarddir, card) = os.path.split(card)

        # execute goodness-of-fit commands
        commands = cbt.get_gof_commands( datacarddir, card, ntoys=args.ntoys )
        if( args.runmode=='condor' ):
            ct.submitCommandsAsCondorJob( 'cjob_gof_run', commands,
              cmssw_version=CMSSW_VERSION )
        elif( args.runmode=='local'):
            script_name = 'local_gof_run.sh'
            ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
            with open( script_name, 'a' ) as script:
                for c in commands: script.write(c+'\n')
            os.system('bash {}'.format(script_name))
