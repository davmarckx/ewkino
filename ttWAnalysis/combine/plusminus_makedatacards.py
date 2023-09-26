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

# settings

topdir = '../analysis/output_20230920_single'
  
#years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
years = ['run2']

regions = ({
    'signalregion_dilepton_plus': '_eventBDT',
    'signalregion_dilepton_minus': '_eventBDT',
    'trileptoncontrolregion': '_nJetsNLooseBJetsCat',
    'fourleptoncontrolregion': '_nJetsNZCat',
    'npcontrolregion_dilepton_inclusive': '_eventBDT',
    'cfjetscontrolregion': '_nJets'
})

inputfiletag = 'merged_npfromdatasplit_cffromdata/merged.root'

outputdir = 'datacards_20230922_plusminus'

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
    cmds.append(cmd)

# run commands
if runmode=='local':
  for cmd in cmds: os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorJob( 'cjob_makedatacard', cmds,
             cmssw_version=CMSSW_VERSION ) 
