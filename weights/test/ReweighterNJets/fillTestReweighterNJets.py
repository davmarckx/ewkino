###########################################
# a submitter for fillTestReweighterNJets #
###########################################

import sys
import os
sys.path.append('../../../jobSubmission')
import condorTools as ct

if __name__=='__main__':

    inputfile = os.path.abspath(sys.argv[1])
    outputfile = sys.argv[2]
    nevents = sys.argv[3]
    exe = './fillTestReweighterNJets'

    if not os.path.exists(inputfile):
	raise Exception('ERROR: input file does not exist.')
    if not os.path.exists(exe):
	raise Exception('ERROR: executable does not exist.')

    (inputdir,inputfile) = os.path.split(inputfile)
    command = '{} {} {} {} {}'.format(exe, inputdir, inputfile, outputfile, nevents)
    ct.submitCommandAsCondorJob( 'cjob_fillTestReweighterNJets', command )
