#########################################
# Looper for optimizeResponsMatrices.py #
#########################################

import sys
import os
import json
import argparse
sys.path.append('../../jobSubmission')
import condorTools as ct


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser('Optimize response matrix binning')
    parser.add_argument('--inputfile', required=True, type=os.path.abspath)
    parser.add_argument('--outputdir', required=True, type=os.path.abspath)
    parser.add_argument('--hists', required=True)
    parser.add_argument('--maxiter', default=10, type=int)
    parser.add_argument('--runmode', default='condor', choices=['condor','local'])
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
      print('  - {}: {}'.format(arg,getattr(args,arg)))

    # argument checks and parsing
    if not os.path.exists(args.inputfile):
      raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))
    if not os.path.exists(args.outputdir):
      os.makedirs(args.outputdir)
    if not os.path.exists(args.hists):
      raise Exception('ERROR: conf file {} does not exist.'.format(args.hists))

    # read configuration
    with open(args.hists, 'r') as f:
	hists = json.load(f)

    # make the commands
    cmds = []
    for hist,initbins in hists.items():
	outputfile = hist+'.json'
	outputfile = os.path.join(args.outputdir,outputfile)
	cmd = 'python optimizeResponseMatrices.py'
	cmd += ' --inputfile {}'.format(args.inputfile)
	cmd += ' --outputfile {}'.format(outputfile)
	cmd += ' --histname {}'.format(hist)
	cmd += ' --initbins \' {}\''.format(','.join([str(el) for el in initbins]))
	# (note: the extra space is intentional, to work correctly with negative numbers)
	cmd += ' --maxiter {}'.format(args.maxiter)
	cmds.append(cmd)

    # run the commands
    if args.runmode=='local':
	for cmd in cmds: 
	    print(cmd)
	    os.system(cmd)
    elif args.runmode=='condor':
	ct.submitCommandsAsCondorJob('cjob_optimizeResponseMatrices', cmds)
