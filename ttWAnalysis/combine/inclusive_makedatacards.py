###############################################
# Run makedatacard.py for predefined settings #
###############################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION="~/CMSSW_10_2_16_UL3"
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables
# settings

topdir = sys.argv[1]
outputdir = sys.argv[2]
  
years = ['2016PreVFP', '2016PostVFP', '2017', '2018','run2']
#years = ['run2']

regions = ({
  'peryear': {
    'signalregion_dilepton_inclusive': '_eventBDT',
    'signalregion_trilepton': '_eventBDT',#change back here
    'trileptoncontrolregion': '_nJetsNBJetsCat',#change back here
    'fourleptoncontrolregion': '_nJetsNZCat',#change back here
    'npcontrolregion_dilepton_inclusive': '_eventBDT',#change back here
    'cfjetscontrolregion': '_nJets'#change back here
  },
  'perchannel': {
    'signalregion_dilepton_ee': '_eventBDT',
    'signalregion_dilepton_em': '_eventBDT',
    'signalregion_dilepton_me': '_eventBDT',
    'signalregion_dilepton_mm': '_eventBDT',
    'signalregion_trilepton': '_eventBDT',
    'trileptoncontrolregion': '_nJetsNBJetsCat',
    'fourleptoncontrolregion': '_nJetsNZCat',
    #'npcontrolregion_dilepton_mm': '_eventBDT',
    #'npcontrolregion_dilepton_me': '_eventBDT',
    #'npcontrolregion_dilepton_em': '_eventBDT',
    #'npcontrolregion_dilepton_ee': '_eventBDT',
    'npcontrolregion_dilepton_inclusive': '_eventBDT',
    'cfjetscontrolregion': '_nJets' 
  },
  'persign': {
    'signalregion_dilepton_plus': '_eventBDT',
    'signalregion_dilepton_minus': '_eventBDT',
    'signalregion_trilepton': '_eventBDT',
    'trileptoncontrolregion': '_nJetsNBJetsCat',
    'fourleptoncontrolregion': '_nJetsNZCat',
    #'npcontrolregion_dilepton_mm': '_eventBDT',
    #'npcontrolregion_dilepton_me': '_eventBDT',
    #'npcontrolregion_dilepton_em': '_eventBDT',
    #'npcontrolregion_dilepton_ee': '_eventBDT',
    'npcontrolregion_dilepton_inclusive': '_eventBDT',
    'cfjetscontrolregion': '_nJets'
  }
})
  
inputfiletag = 'merged_npfromdatasplit_cffromdata/merged.root'

rateparams = None
rateparams = ['WZ', 'ZZ', 'TTZ']

runmode = 'condor'

# loop over years and regions
cmds = []
for year in years:
  for configtag, config in regions.items():
    if configtag == 'peryear' and year=='run2': continue
    if configtag != 'peryear' and year!='run2': continue
    thiscmds = []
    thisoutputdir = outputdir + '_{}'.format(configtag)
    region = 'signalregion_dilepton_inclusive'
    #variables = read_variables( '../variables/variables_inputfeatures.json' )#changed here
    #variables = [str(var.name) for var in variables] 
    #for variable in variables:
    for region,variable in config.items():#change back here
      # find input file
      inputfile = os.path.join(topdir,year,region,inputfiletag)
      if 'signalregion_tri' in region:
        inputfile = inputfile.replace('.root','_rebinned.root')
      if 'npcontrolregion' in region and not 'inclusive' in region:
        print("rebin np region")
        inputfile = inputfile.replace('.root','_rebinned_lastbins.root')
      if not os.path.exists(inputfile):
        raise Exception('ERROR: file {} does not exist.'.format(inputfile))
      # define output file
      outputfiletag = '{}_{}'.format(region,year)# region and variable replaced
      outputfiletag = os.path.join(thisoutputdir, outputfiletag)
      # make command
      cmd = 'python makedatacard.py'
      cmd += ' --inputfile {}'.format(inputfile)
      cmd += ' --year {}'.format(year)
      cmd += ' --region {}'.format(region)
      cmd += ' --variable {}'.format(variable)
      cmd += ' --outputfile {}'.format(outputfiletag)
      #cmd += ' --excludetags {}'.format("Nonprompt")
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
