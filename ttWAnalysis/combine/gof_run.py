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
    parser.add_argument('--ntoyjobs', default=1, type=int)
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
        card = os.path.splitext(workspace)[0] + '.txt'
        (datacarddir, card) = os.path.split(card)

        # check if provided workspace is actually a workspace or rather a datacard
        if workspace.endswith('.root'): pass
        elif workspace.endswith('.txt'):
            ws = os.path.splitext(workspace)[0] + '.root'
            if os.path.exists(ws):
                msg = 'WARNING: provided workspace {} seems to be a datacard,'.format(workspace)
                msg += ' but the associated workspace {} exists,'.format(ws)
                msg += ' will run on that one.'
                print(msg)
                workspace = ws
            else:
                msg = 'WARNING: provided workspace {} seems to be a datacard,'.format(workspace)
                msg += ' and the associated workspace {} does not exist,'.format(ws)
                msg += ' will create the workspace and run on it.'
                print(msg)
                script_name = 'local_gof_run_temp.sh'
                ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
                with open( script_name, 'a' ) as script:
                    for c in cbt.get_workspace_commands( datacarddir, card ): script.write(c+'\n')
                os.system('bash {}'.format(script_name))
                os.system('rm {}'.format(script_name))

        # loop over number of jobs
        for randomseed in range(args.ntoyjobs+1):
            do_data = False
            do_toys = True
            # first job is for data
            if randomseed==0: 
                do_data = True
                do_toys = False
            commands = []
            for c in cbt.get_gof_commands( datacarddir, card, ntoys=args.ntoys,
                randomseed=randomseed, do_data=do_data, do_toys=do_toys ):
                commands.append(c)
            if( args.runmode=='condor' ):
                ct.submitCommandsAsCondorJob( 'cjob_gof_run', commands,
                  cmssw_version=CMSSW_VERSION )
            elif( args.runmode=='local'):
                script_name = 'local_gof_run.sh'
                ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
                with open( script_name, 'a' ) as script:
                    for c in commands: script.write(c+'\n')
                os.system('bash {}'.format(script_name))
