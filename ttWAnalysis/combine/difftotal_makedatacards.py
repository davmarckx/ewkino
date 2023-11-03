###############################################
# Run makedatacard.py for predefined settings #
###############################################
# Internal consistency check: measure inclusive cross-section using differential setup.
# The specific method applied here is very simple (as a first check):
# - Use differential distributions, but a single signal strength for all TTW contributions.
# - So exactly like inclusive measurement, but with different distributions.

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables

# settings

controlregions = ({
    # note: apart from control regions, also includes inclusive signal regions
    'topdir': '../analysis/output_20231025_single',
    'years': ['2016PreVFP','2016PostVFP','2017','2018','run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged.root',
    'regions': {
        'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
        'fourleptoncontrolregion': '_nJetsNZCat',
        'npcontrolregion_dilepton_inclusive': '_eventBDT',
        'cfjetscontrolregion': '_nJets',
        'signalregion_dilepton_inclusive': '_eventBDT',
        'signalregion_trilepton': '_eventBDT'
    }
})

signalregions = ({
    'topdir': '../analysis/output_20231025_double',
    'years': ['2016PreVFP','2016PostVFP','2017','2018','run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged.root',
    'regions': {
        'signalregion_dilepton_inclusive',
        'signalregion_trilepton'
    },
    'variables': '../variables/variables_particlelevel_double.json'
})
  
outputdir = 'datacards_20231031_difftotal'

runmode = 'condor'

# make output directory
if not os.path.exists(outputdir):
  os.makedirs(outputdir)

# initialize empty list of commands to run
cmds = []

# read variables
variables = read_variables( signalregions['variables'] )
varnames = [str(var.name) for var in variables]
secondaryvarnames = [str(var.secondary.name) for var in variables]
print('Will use the following variables:')
print(varnames)

# write differential signal region datacards
for year in signalregions['years']:
  for region in signalregions['regions']:
    # find input file
    inputfile = os.path.join(signalregions['topdir'],
                  year, region, signalregions['inputfiletag'])
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))
    for varname, secondaryvarname in zip(varnames, secondaryvarnames):
      # define output file
      outputfiletag = '{}_{}_{}'.format(region,year,varname)
      outputfiletag = os.path.join(outputdir, outputfiletag)
      # define signals
      signals = []
      for i in [1,2,3,4,5,6]: signals.append('TTW{}{}'.format(i,secondaryvarname.strip('_')))
      signals = ','.join(signals)
      # make command
      cmd = 'python makedatacard.py'
      cmd += ' --inputfile {}'.format(inputfile)
      cmd += ' --year {}'.format(year)
      cmd += ' --region {}'.format(region)
      cmd += ' --variable {}'.format(varname)
      cmd += ' --outputfile {}'.format(outputfiletag)
      cmd += ' --processes all'
      cmd += ' --signals {}'.format(signals)
      cmd += ' --datatag Data'
      cmds.append(cmd)

# write control region datacards
for year in controlregions['years']:
  for region,variable in controlregions['regions'].items():
    # find input file
    inputfile = os.path.join(controlregions['topdir'],year,region,controlregions['inputfiletag'])
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))
    # define output file
    outputfiletag = '{}_{}'.format(region, year)
    # modify the datacard name for inclusive signal region
    # to mention 'inclusive' explicitly
    if 'signalregion' in region: outputfiletag += '__eventBDTinclusive'
    outputfiletag = os.path.join(outputdir, outputfiletag)
    # define signals
    signals = 'TTW'
    # make command
    cmd = 'python makedatacard.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --year {}'.format(year)
    cmd += ' --region {}'.format(region)
    cmd += ' --variable {}'.format(variable)
    cmd += ' --outputfile {}'.format(outputfiletag)
    cmd += ' --processes all'
    cmd += ' --signals {}'.format(signals)
    cmd += ' --datatag Data'
    cmds.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorCluster( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
