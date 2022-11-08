#########################################################
# a python submitter for fillMCChargeFlipMeasurement.cc #
#########################################################

import sys
import os
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append(os.path.abspath('../Tools/python'))
from samplelisttools import readsamplelist

# set global properties
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (pick any combination from '2016PreVFP', '2016PostVFP', '2017' and '2018')
flavours = ['electron','muon']
# (pick from 'electron' and 'muon')
processes = ['DY', 'TT', 'all']
# (pick process tags present in the sample lists, use 'all' to use all samples)
binnings = ['Gianny','TuThong']
# (pick from 'Gianny' and 'TuThong')
runmode = 'condor'
# (pick from 'condor' or 'local')
nentries = 5e6
# (number of entries to use per file)
samplelistdirectory = os.path.abspath('../chargeFlips/sampleListsUL')
samplelist = 'samples_chargeFlips_MC_{}.txt'
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_chargeflips/sim/{}'
# (directory where samples are located)
outputdirectory = 'chargeFlipMaps_v20221108'

# check if executable exists
exe = './fillMCChargeFlipMeasurement'
if not os.path.exists(exe):
  raise Exception('ERROR: executable {} does not seem to exist...'.format(exe))

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
  for flavour in flavours:
    # make path to sample list
    thissampledir = sampledirectory.format(year)
    thissamplelist = samplelist.format(year)
    thissamplelist = os.path.join(samplelistdirectory,
                       thissamplelist)
    # check number of samples
    samples = readsamplelist( thissamplelist, sampledir=thissampledir )
    nsamples = samples.number()
    print('Found {} samples.'.format(nsamples))
    # loop over other settings
    for process in processes:
      for binning in binnings:   
        # make the command and add it to the list
        command = '{} {} {} {} {} {} {} {} {}'.format(exe,
                  flavour, year, process, binning,
                  thissamplelist, thissampledir, outputdirectory, nentries)
        cmds.append(command)

# submit the commands as jobs
if( runmode=='local' ):
  for cmd in cmds: os.system(cmd)
elif( runmode=='condor' ):
  ct.submitCommandsAsCondorCluster('cjob_fillMCChargeFlipMeasurement', cmds,
		                   cmssw_version=CMSSW_VERSION)
