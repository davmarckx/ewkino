####################################################
# Simple year merger for the output of runanalysis #
####################################################
# Simply calls same functionality in eventselection folder

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION


if __name__=='__main__':

  # settings
  inputdir = os.path.abspath(sys.argv[1])
  
  efts = ['']#["EFTsm","","EFTcQq81","EFTcQq83","EFTcQei","EFTcQl3i","EFTcQlMi","EFTcQq11","EFTcQq13","EFTcbW","EFTcpQ3","EFTcpQM","EFTcpt","EFTcptb","EFTctG","EFTctW","EFTctZ","EFTctei","EFTctlSi","EFTctlTi","EFTctli","EFTctp","EFTctq1","EFTctq8"]

  # call mergefolders in eventselection
  for eft in efts:
    cmd = 'python ../eventselection/mergeyears.py'
    cmd += ' --directory {}'.format(inputdir)
    cmd += ' --filemode split'
    if eft != '':
      cmd += ' --eft {}'.format(eft)
    #os.system(cmd)
    ct.submitCommandAsCondorJob( 'cjob_mergeyears', cmd,
                               cmssw_version=CMSSW_VERSION )
