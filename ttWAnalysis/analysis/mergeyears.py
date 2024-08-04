####################################################
# Simple year merger for the output of runanalysis #
####################################################
# Simply calls same functionality in eventselection folder

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION='~/CMSSW_10_6_28'

if __name__=='__main__':

  # settings
  inputdir = os.path.abspath(sys.argv[1])

  # call mergefolders in eventselection
  cmd = 'python ../eventselection/mergeyears.py'
  cmd += ' --directory {}'.format(inputdir)
  cmd += ' --filemode split'
  os.system(cmd)
  #ct.submitCommandAsCondorJob( 'cjob_mergeyears', cmd,
  #                             cmssw_version=CMSSW_VERSION )
