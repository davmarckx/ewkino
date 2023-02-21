###############################################
# Run makedatacard.py for predefined settings #
###############################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

# settings

topdir = '/user/llambrec/public/output_20230206_single'
  
years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
#years = ['run2']

regions = ({
    'signalregion_dilepton_inclusive': '_eventBDT',
    #'signalregion_trilepton': '_eventBDT',
    #'wzcontrolregion': '_yield',
    #'zzcontrolregion': '_yield',
    #'zgcontrolregion': '_yield',
    #'trileptoncontrolregion': '_nJets',
    #'fourleptoncontrolregion': '_nJets',
    #'npcontrolregion_dilepton_inclusive': '_eventBDT',
    #'cfcontrolregion': '_nJets'
})
  
inputfiletag = 'merged_npfromdata_cffromdata/merged.root'

outputdir = 'datacards_20230113_single'

runmode = 'local'

# make output directory
if not os.path.exists(outputdir):
  os.makedirs(outputdir)

# loop over years and regions
cmds = []
for year in years:
  for region,variable in regions.items():
    # find input file
    inputfile = os.path.join(topdir,year,region,inputfiletag)
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))
    # define output file
    outputfiletag = '{}_{}'.format(region,year)
    outputfiletag = os.path.join(outputdir, outputfiletag)
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
    cmds.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJob( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
