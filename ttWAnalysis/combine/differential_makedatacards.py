###############################################
# Run makedatacard.py for predefined settings #
###############################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION="/user/dmarckx/CMSSW_10_2_16_UL3"
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables
from histtools import loadallhistnames
import listtools as lt
# settings

controlregions = ({
    'topdir': '../analysis/output_20240714_CR/',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged.root',
    'regions': {
        'trileptoncontrolregion': '_nJetsNBJetsCat',
        'fourleptoncontrolregion': '_nJetsNZCat',
        'npcontrolregion_dilepton_inclusive': '_eventBDT',
        #'npcontrolregion_dilepton_em': '_eventBDT',
        #'npcontrolregion_dilepton_me': '_eventBDT',
        #'npcontrolregion_dilepton_ee': '_eventBDT',
        'cfjetscontrolregion': '_nJets'
    }
})

signalregion = ({
    'topdir': '../analysis/output_20240714_SR',
    'years': ['run2'],
    'inputfiletag': 'merged_npfromdatasplit_cffromdata/merged.root',
    'region': 'signalregion_dilepton_inclusive',
    #'region': 'signalregion_trilepton',
    'variables': '../variables/variables_particlelevel_double.json'
})

#rateparams = None
rateparams = ['WZ', 'ZZ', 'TTZ']

outputdir = 'datacards_differential'

runmode = 'condor'

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
  if 'npcontrolregion' in region:
      inputfile = inputfile.replace('.root','_rebinned_lastbins.root')  
  if not os.path.exists(inputfile):
    raise Exception('ERROR: file {} does not exist.'.format(inputfile))
  for varname, secondaryvarname in zip(varnames, secondaryvarnames):
    # define output file
    othervarnames = [v.name for v in variables if v.name!=varname]
    outputfiletag = '{}_{}_{}'.format(region,year,varname)
    outputfiletag = os.path.join(outputdir, outputfiletag)
    # define signals
    signals = []
    for i in [1,2,3,4,5,6]: signals.append('TTW{}{}'.format(i,varname.strip('_')))
    signals = ','.join(signals)

    allprocesses = list(set( [x.split("_")[0] for x in loadallhistnames(inputfile)] ))
    ttwprocesses = lt.subselect_strings(allprocesses,mustcontainone=["TTW0","1"+varname.strip('_'),"2"+varname.strip('_'),"3"+varname.strip('_'),"4"+varname.strip('_'),"5"+varname.strip('_'),"6"+varname.strip('_'),"7"+varname.strip('_')],mustcontainall=[],maynotcontainone=[varname.strip('_')+"2"]+['_{}_'.format(el) for el in othervarnames])[1]
    bgrdprocesses = lt.subselect_strings(allprocesses,mustcontainone=[],mustcontainall=[],maynotcontainone=["TTW"])[1]
    print(varname)
    print(bgrdprocesses)
    selectedprocesses = ttwprocesses+bgrdprocesses
    # make command
    cmd = 'python makedatacard.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --year {}'.format(year)
    cmd += ' --region {}'.format(region)
    cmd += ' --variable {}'.format(varname)
    cmd += ' --outputfile {}'.format(outputfiletag)
    cmd += ' --processes {}'.format(",".join(selectedprocesses))
    cmd += ' --signals {}'.format(signals)
    cmd += ' --datatag Data'
    if rateparams is not None: cmd += ' --rateparams {}'.format(','.join(rateparams))
    cmds.append(cmd)

print("WRITE crS =====================================================================================================================================")

# write control region datacards
for year in controlregions['years']:
  for region,variable in controlregions['regions'].items():
   for varname, secondaryvarname in zip(varnames, secondaryvarnames):
    othervarnames = [v.name for v in variables if v.name!=varname]
    # find input file
    inputfile = os.path.join(controlregions['topdir'],year,region,controlregions['inputfiletag'])
    #if 'npcontrolregion' in region:
    #  inputfile = inputfile.replace('.root','_rebinned_lastbins.root')
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))

    allprocesses = list(set( [x.split("_")[0] for x in loadallhistnames(inputfile)] ))
    #print(allprocesses)
    ttwprocesses = lt.subselect_strings(allprocesses,mustcontainone=["TTW0","1"+secondaryvarname.strip('_'),"2"+secondaryvarname.strip('_'),"3"+secondaryvarname.strip('_'),"4"+secondaryvarname.strip('_'),"5"+secondaryvarname.strip('_'),"6"+secondaryvarname.strip('_'),"7"+secondaryvarname.strip('_')],mustcontainall=[],maynotcontainone=[varname.strip('_')+"2"]+['_{}_'.format(el) for el in othervarnames])[1] 
    bgrdprocesses = lt.subselect_strings(allprocesses,mustcontainone=[],mustcontainall=[],maynotcontainone=["TTW"])[1]
    selectedprocesses = ttwprocesses+bgrdprocesses

    signals = []
    for i in [1,2,3,4,5,6]: signals.append('TTW{}{}'.format(i,varname.strip('_')))
    signals = ','.join(signals)

    print(varname)
    print(bgrdprocesses)
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
    cmd += ' --processes {}'.format(",".join(selectedprocesses))
    cmd += ' --signals {}'.format(signals)
    cmd += ' --datatag Data'
    if rateparams is not None: cmd += ' --rateparams {}'.format(','.join(rateparams))
    cmds2.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
  for cmd in cmds2: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorCluster( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION )
  ct.submitCommandsAsCondorCluster( 'cjob_makedatacard', cmds2,
             cmssw_version=CMSSW_VERSION )
