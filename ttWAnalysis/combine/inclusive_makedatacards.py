###############################################
# Run makedatacard.py for predefined settings #
###############################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

# settings

topdir = '../analysis/output_20231025_single'
 
years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
#years = ['run2']
#years = ['2018']

regions = ({
  'peryear': {
    'signalregion_dilepton_inclusive': '_eventBDT',
    'signalregion_trilepton': '_eventBDT',
    'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
    'fourleptoncontrolregion': '_nJetsNZCat',
    'npcontrolregion_dilepton_inclusive': '_eventBDT',
    'cfjetscontrolregion': '_nJets'
  },
  #'perchannel': {
  #  'signalregion_dilepton_ee': '_eventBDT',
  #  'signalregion_dilepton_em': '_eventBDT',
  #  'signalregion_dilepton_me': '_eventBDT',
  #  'signalregion_dilepton_mm': '_eventBDT',
  #  'signalregion_trilepton': '_eventBDT',
  #  'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
  #  'fourleptoncontrolregion': '_nJetsNZCat',
  #  'npcontrolregion_dilepton_inclusive': '_eventBDT',
  #  'cfjetscontrolregion': '_nJets' 
  #},
  #'persign': {
  #  'signalregion_dilepton_plus': '_eventBDT',
  #  'signalregion_dilepton_minus': '_eventBDT',
  #  'signalregion_trilepton': '_eventBDT',
  #  'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
  #  'fourleptoncontrolregion': '_nJetsNZCat',
  #  'npcontrolregion_dilepton_inclusive': '_eventBDT',
  #  'cfjetscontrolregion': '_nJets'
  #}
})
  
inputfiletag = 'merged_npfromdatasplit_cffromdata/merged.root'

#rateparams = None
rateparams = ['WZ', 'ZZ', 'TTZ']

outputdir = 'datacards_inclusive_test_newfiles_newuncs'

runmode = 'condor'

# loop over years and regions
cmds = []
for year in years:
  for configtag, config in regions.items():
    thiscmds = []
    thisoutputdir = outputdir + '_{}'.format(configtag)
    for region,variable in config.items():
      # find input file
      inputfile = os.path.join(topdir,year,region,inputfiletag)
      if not os.path.exists(inputfile):
        raise Exception('ERROR: file {} does not exist.'.format(inputfile))
      # define output file
      outputfiletag = '{}_{}'.format(region,year)
      outputfiletag = os.path.join(thisoutputdir, outputfiletag)
      # make command
      cmd = 'python makedatacard.py'
      cmd += ' --inputfile {}'.format(inputfile)
      cmd += ' --year {}'.format(year)
      cmd += ' --region {}'.format(region)
      cmd += ' --variable {}'.format(variable)
      cmd += ' --outputfile {}'.format(outputfiletag)
      cmd += ' --processes all'
      cmd += ' --signals TTW'
      cmd += ' --datatag Data'
      if rateparams is not None: cmd += ' --rateparams {}'.format(','.join(rateparams))
      thiscmds.append(cmd)
    cmds.append(thiscmds)

# run commands
if runmode=='local':
  for cmdset in cmds:
    for cmd in cmdset: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJobs( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
