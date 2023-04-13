###############################################
# Run makedatacard.py for predefined settings #
###############################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables

# settings

controlregions = ({
    'topdir': '../analysis/output_20230314_single',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdata_cffromdata/merged.root',
    'regions': {
        'trileptoncontrolregion': '_nJets',
        'fourleptoncontrolregion': '_nJets',
        'npcontrolregion_dilepton_inclusive': '_eventBDT',
        'cfcontrolregion': '_nJets'
    }
})

signalregion = ({
    'topdir': '../analysis/output_20230314_double',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdata_cffromdata/merged.root',
    'region': 'signalregion_dilepton_inclusive',
    'variables': '../variables/variables_particlelevel_double.json'
})
  
outputdir = 'datacards_20230316_double'

runmode = 'condor'

# make output directory
if not os.path.exists(outputdir):
  os.makedirs(outputdir)

# initialize empty list of commands to run
cmds = []

# read variables
variables = read_variables( signalregion['variables'] )
varnames = [str(var.name) for var in variables]
print('Will use the following variables:')
print(varnames)

# write signal region datacards
for year in signalregion['years']:
  region = signalregion['region']
  # find input file
  inputfile = os.path.join(signalregion['topdir'],year,region,signalregion['inputfiletag'])
  if not os.path.exists(inputfile):
    raise Exception('ERROR: file {} does not exist.'.format(inputfile))
  for varname in varnames:
    # define output file
    outputfiletag = '{}_{}_{}'.format(region,year,varname)
    outputfiletag = os.path.join(outputdir, outputfiletag)
    # make command
    cmd = 'python makedatacard.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --year {}'.format(year)
    cmd += ' --region {}'.format(region)
    cmd += ' --variable {}'.format(varname)
    cmd += ' --outputfile {}'.format(outputfiletag)
    cmd += ' --processes all'
    cmd += ' --signals TTW1,TTW2,TTW3,TTW4'
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
    cmd += ' --datatag Data'
    cmds.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJob( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
