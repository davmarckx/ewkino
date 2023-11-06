####################################################################################
# Simple submitter that runs merge.py for a number of predefined regions and years #
####################################################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

topdir = sys.argv[1]

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
#for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
#for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
#for r in ['nplownjetscontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)
#for r in ['cfjetscontrolregion']: regions.append(r)


years = ['auto']

npmodes = []
#npmodes.append( 'npfromsim' )
npmodes.append( 'npfromdata' )
npmodes.append( 'npfromdatasplit' )

cfmodes = []
#cfmodes.append( 'cffromsim' )
cfmodes.append( 'cffromdata' )

rename = '../eventselection/processes/rename_processes.json'
renamemode = 'fast'

decorrelate = 'correlations/correlations.json'
decorrelatemode = 'fast'

selectmode = 'noselect'

doclip = True

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
