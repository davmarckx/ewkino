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
from histtools import loadallhistnames
import listtools as lt

# settings
eft = sys.argv[1]

controlregions = ({
    'topdir': '../analysis/output_sbv3_split',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged'+eft+'.root',
    'regions': {
        'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
        'fourleptoncontrolregion': '_nJetsNZCat',
        'npcontrolregion_dilepton_inclusive': '_nMuons',
        'cfjetscontrolregion': '_nJetsNZCat'
    }
})

signalregion = ({
    'topdir': '../analysis/output_dbv3/',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged'+eft+'.root',
    'region': 'signalregion_dilepton_inclusive',
    'variables': '../variables/variables_particlelevel_double_BINSTUDY.json'
})

outputdir = 'datacards_EFTstudy_npsplitAndUnbiasedAndbgstat_'+eft

runmode = 'local'

# make output directory
if not os.path.exists(outputdir):
  os.makedirs(outputdir)

# initialize empty list of commands to run
cmds = []
cmds2 = []
# read variables
variables = read_variables( signalregion['variables'] )
varnames = [str(var.name) for var in variables]
secondaryvarnames = [str(var.secondary.name) for var in variables]
print('Will use the following variables:')
print(varnames)

# write signal region datacards
for year in signalregion['years']:
  region = signalregion['region']
  # find input file
  inputfile = os.path.join(signalregion['topdir'],year,region,signalregion['inputfiletag'])
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
splitCRs = ["npcontrolregion_dilepton_inclusive"]
for year in controlregions['years']:
  for region,variable in controlregions['regions'].items():
   for varname, secondaryvarname in zip(varnames, secondaryvarnames):
    # find input file
    print(secondaryvarname)
    inputfile = os.path.join(controlregions['topdir'],year,region,controlregions['inputfiletag'])
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))
    allprocesses = list(set( [x.split("_")[0] for x in loadallhistnames(inputfile)] ))
    print(allprocesses)
    ttwprocesses = lt.subselect_strings(allprocesses,mustcontainone=["TTW0",secondaryvarname.strip('_')],mustcontainall=[],maynotcontainone=[])[1] 
    bgrdprocesses = lt.subselect_strings(allprocesses,mustcontainone=[],mustcontainall=[],maynotcontainone=["TTW"])[1]
    selectedprocesses = ttwprocesses+bgrdprocesses
    signals = []
    for i in [1,2,3,4,5,6]: signals.append('TTW{}{}'.format(i,secondaryvarname.strip('_')))
    signals = ','.join(signals)
    # define output file
    outputfiletag = '{}_{}_{}'.format(region,year,varname)
    outputfiletag = os.path.join(outputdir, outputfiletag)
    # make command
    cmd = 'python makedatacard.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --year {}'.format(year)
    cmd += ' --region {}'.format(region)
    cmd += ' --variable {}'.format(variable)
    cmd += ' --outputfile {}'.format(outputfiletag)
    if region in splitCRs:
      print(",".join(selectedprocesses))
      cmd += ' --processes {}'.format(",".join(selectedprocesses))
      cmd += ' --signals {}'.format(signals)
    else:
      cmd += ' --processes all'
      cmd += ' --signals TTW'
    cmd += ' --datatag Data'
    cmds2.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: 
   print(cmd)
   os.system(cmd)
  for cmd in cmds2: 
   print(cmd)
   os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJob( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION )
  ct.submitCommandsAsCondorJob( 'cjob_CRmakedatacard', cmds2,
             cmssw_version=CMSSW_VERSION )
