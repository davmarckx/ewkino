########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION

inputdir = sys.argv[1]
runmode = 'local'

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['cfcontrolregion']: regions.append(r)

years = []
years = ['2016PreVFP','2016PostVFP','2017','2018']
#years.append('run2')

npmodes = ['npfromdata']
cfmodes = ['cffromdata']

dolog = True

variables = '../variables/variables_main.json'

colormap = 'ttw'

filemode = 'combined'#'split'

cmds = []
for year in years:
  for npmode in npmodes:
    for cfmode in cfmodes:
      for region in regions:
        regiondir = ''
        if filemode=='split': regiondir = region
        subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
        inputfile = os.path.join(inputdir, subdir, 'merged.root')
        if not os.path.exists(inputfile):
          print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
          continue
        thisoutputdir = '{}_{}_{}_{}'.format(year,region,npmode,cfmode)
        thisoutputdir = os.path.join("~/public_html/newBDT", subdir, 'plots', thisoutputdir)
        unblind = True
        #if 'signalregion' in region: unblind = False
        cmd = 'python makeplots.py'
        cmd += ' --inputfile '+inputfile
        cmd += ' --year '+year
        cmd += ' --region '+region
        cmd += ' --variables '+variables
        cmd += ' --outputdir '+thisoutputdir
        if unblind: cmd += ' --unblind'
        if dolog: cmd += ' --dolog'
        cmd += ' --colormap '+colormap
        if runmode=='local':
          print('executing '+cmd)
          os.system(cmd)
        elif runmode=='condor':
          print('submitting '+cmd)
          cmds.append(cmd)
        else: raise Exception('ERROR: runmode "{}" not recognized'.format(runmode))

if runmode=='condor':
  ct.submitCommandsAsCondorCluster('cjob_makeplots', cmds,
                                    cmssw_version=CMSSW_VERSION)
