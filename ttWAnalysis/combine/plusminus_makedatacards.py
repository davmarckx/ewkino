###############################################
# Run makedatacard.py for predefined settings #
###############################################
# Special application of fitting ttW+ and ttW- simultaneously.
# The input histograms can be produced with runanalysis,
# (with a single ttW sample, not split in charge),
# as the split is done at this stage by renaming the ttW signal
# in the plus and minus signal region separately.
# Note: the ttW in the control regions is not split,
# (still to decide what to do with it exactly).

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION="/user/llambrec/CMSSW_10_2_X_combine/CMSSW_10_2_13/"

# settings

topdir = sys.argv[1]
outputdir = sys.argv[2]

  
years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
#years = ['run2']

regions = ({
    'signalregion_dilepton_plus': '_eventBDT',
    'signalregion_dilepton_minus': '_eventBDT',
    'trileptoncontrolregion': '_nJetsNBJetsCat',
    'fourleptoncontrolregion': '_nJetsNZCat',
    'npcontrolregion_dilepton_inclusive': '_eventBDT',
    #'npcontrolregion_dilepton_me': '_eventBDT',
    #'npcontrolregion_dilepton_em': '_eventBDT',
    #'npcontrolregion_dilepton_ee': '_eventBDT',
    'cfjetscontrolregion': '_nJets'
})

inputfiletag = 'merged_npfromdatasplit_cffromdata/merged.root'

#rateparams = None
rateparams = ['WZ', 'ZZ', 'TTZ']

runmode = 'condor'

# make output directory
if not os.path.exists(outputdir):
  os.makedirs(outputdir)

# loop over years and regions
cmds = []
for year in years:
  for region,variable in regions.items():
    # find input file
    inputfile = os.path.join(topdir,year,region,inputfiletag)
    if 'signalregion' in region:
        inputfile = inputfile.replace('.root','_rebinned.root')
    if 'npcontrolregion' in region:
        inputfile = inputfile.replace('.root','_rebinned_lastbins.root')
    if not os.path.exists(inputfile):
      raise Exception('ERROR: file {} does not exist.'.format(inputfile))
    # define output file
    outputfiletag = '{}_{}'.format(region,year)
    outputfiletag = os.path.join(outputdir, outputfiletag)
    # set signal name modifier
    signals = 'TTW'
    renamesignals = None
    if region=='signalregion_dilepton_plus': renamesignals = 'TTWplus'
    elif region=='signalregion_dilepton_minus': renamesignals = 'TTWminus'
    else: signals = None
    # make command
    cmd = 'python makedatacard.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --year {}'.format(year)
    cmd += ' --region {}'.format(region)
    cmd += ' --variable {}'.format(variable)
    cmd += ' --outputfile {}'.format(outputfiletag)
    cmd += ' --processes all'
    if signals is not None:
      cmd += ' --signals {}'.format(signals)
    if renamesignals is not None:
      cmd += ' --renamesignals {}'.format(renamesignals)
    cmd += ' --datatag Data'
    if rateparams is not None: cmd += ' --rateparams {}'.format(','.join(rateparams))
    cmds.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJob( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
