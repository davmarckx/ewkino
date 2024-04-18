####################################################################################
# Simple submitter that runs merge.py for a number of predefined regions and years #
####################################################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

topdir = sys.argv[1]

regions = ['auto']

years = ['auto']

npmodes = []
#npmodes.append( 'npfromsim' )
#npmodes.append( 'npfromdata' )
npmodes.append( 'npfromdatasplit' )

cfmodes = []
#cfmodes.append( 'cffromsim' )
cfmodes.append( 'cffromdata' )

rename = 'processes/rename_processes.json'
renamemode = 'fast'

decorrelate = 'correlations/correlations.json'
decorrelatemode = 'fast'

selectmode = 'noselect'

doclip = False
# note: disabled clipping all histograms to minimum 0 as it is very slow;
#       instead one should make sure to do clipping downstream
#       e.g. when plotting or writing datacards.

runmode = 'condor'

cmds = []
if 'auto' in years: years = os.listdir(topdir)
for year in years:
  if 'auto' in regions: regions = os.listdir(os.path.join(topdir,year))
  for region in regions:
    for npmode in npmodes:
      for cfmode in cfmodes:
        inputdir = os.path.join(topdir, year, region)
        outputfile = os.path.join(topdir, year, region, 'merged_{}_{}'.format(npmode,cfmode), 'merged.root')
        cmd = 'python merge.py'
        cmd += ' --directory '+inputdir
        cmd += ' --outputfile '+outputfile
        cmd += ' --npmode '+npmode
        cmd += ' --cfmode '+cfmode
        if rename is not None: 
          cmd += ' --rename '+rename
          cmd += ' --renamemode '+renamemode
        if decorrelate is not None:
          cmd += ' --decorrelate '+decorrelate
          cmd += ' --decorrelatemode '+decorrelatemode
          cmd += ' --decorrelateyear '+year
        cmd += ' --selectmode '+selectmode
        if doclip: cmd += ' --doclip'
        cmd += ' --runmode local'
        cmds.append(cmd)

if runmode=='local':
  for cmd in cmds:
    print('executing '+cmd)
    os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorCluster( 'cjob_merge', cmds, cmssw_version=CMSSW_VERSION )
