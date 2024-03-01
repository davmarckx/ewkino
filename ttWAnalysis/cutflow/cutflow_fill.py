###############
# Run cutFlow #
###############

import sys
import os
import argparse
sys.path.append('../../jobSubmission')
import condorTools as ct


def make_cutflow_command( args ):
    ### make a command line call to ./cutflow
    # input arguments:
    # - args: object of type OptionCollection with correct attributes

    cmd = './cutflow'
    cmd += ' {}'.format(args.inputfile)
    cmd += ' {}'.format(args.outputfile)
    cmd += ' {}'.format(args.eventselection)
    cmd += ' {}'.format(args.selectiontype)
    cmd += ' {}'.format(args.variation)
    cmd += ' {}'.format(args.nevents)
    cmd += ' {}'.format(args.maxcutflowvalue)
    cmd += ' {}'.format(args.do_particle_level)
    return cmd


if __name__=='__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfile', type=os.path.abspath, required=True)
    parser.add_argument('-o', '--outputfile', type=os.path.abspath, required=True)
    parser.add_argument('-e', '--eventselection', default='signalregion')
    parser.add_argument('-t', '--selectiontype', default='tight')
    parser.add_argument('-v', '--variation', default='nominal')
    parser.add_argument('-n', '--nevents', type=int, default=-1)
    parser.add_argument('-m', '--maxcutflowvalue', type=int, default=10)
    parser.add_argument('--do_particle_level', default=False, action='store_true')
    parser.add_argument('--runmode', default='local', choices=['local', 'condor'])
    args = parser.parse_args()
    
    # parse input arguments
    if os.path.splitext(args.outputfile)[1]!='.root':
	print('WARNING: adding .root to output file name {}'.format(args.outputfile))
	args.outputfile = os.path.splitext(args.outputfile)[0]+'.root'
    outputdir = os.path.dirname(args.outputfile)
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    # make the commands
    cmds = []
    cmds.append( make_cutflow_command(args) )

    # submit the job
    if args.runmode=='local':
	for cmd in cmds: 
	    print('Now running {}...'.format(cmd))
	    os.system(cmd)
    else: ct.submitCommandsAsCondorJob( 'cjob_cutflow.sh', cmds, cmssw_version='CMSSW_12_4_6')
