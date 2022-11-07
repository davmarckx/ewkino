########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION

inputdir = sys.argv[1]
runmode = 'condor'

regions = []
#for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)
#for r in ['nonprompt_trilepton']: regions.append(r)
#for r in ['nonprompt_dilepton']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromsim']

variables = '../variables/variables_main.json'

datatag = 'Data'

colormap = 'tttt'

cmds = []
for year in years:
  for npmode in npmodes:
    subdir = os.path.join(year, 'merged_'+npmode)
    inputfile = os.path.join(inputdir, subdir, 'merged.root')
    if not os.path.exists(inputfile):
      print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
      continue
    for region in regions:
      thisoutputdir = os.path.join(inputdir, subdir, 'plots', year+'_'+region+'_'+npmode)
      unblind = True
      if 'signalregion' in region: unblind = False
      cmd = 'python prefitplots.py'
      cmd += ' --inputfile '+inputfile
      cmd += ' --year '+year
      cmd += ' --region '+region
      cmd += ' --processes all'
      cmd += ' --variables '+variables
      cmd += ' --outputdir '+thisoutputdir
      cmd += ' --datatag '+datatag
      cmd += ' --colormap '+colormap
      if unblind: cmd += ' --unblind'
      if runmode=='local':
        print('executing '+cmd)
        os.system(cmd)
      elif runmode=='condor':
        print('submitting '+cmd)
        cmds.append(cmd)
      else: raise Exception('ERROR: runmode "{}" not recognized'.format(runmode))

if runmode=='condor':
  ct.submitCommandsAsCondorCluster('cjob_prefitplots', cmds,
                                    cmssw_version=CMSSW_VERSION)
