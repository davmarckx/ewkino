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
flavours = ['electron']
# (only 'electron supported for now')
runmode = 'local'
# (pick from 'condor' or 'local')
nentries = 100
# (number of entries to use per file)
samplelistdirectory = os.path.abspath('sampleListsUL')
samplelist = 'samples_chargeFlips_MC_{}.txt'
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/nivanden/skims_OSDL/{}'
# (directory where samples are located)

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
    # make the command and add it to the list
    command = '{} {} {} {} {} {}'.format(exe,
              flavour, year, thissamplelist, thissampledir, nentries)
    cmds.append(command)

# submit the commands as jobs
if( runmode=='local' ):
  for cmd in cmds: os.system(cmd)
elif( runmode=='condor' ):
  ct.submitCommandsAsCondorCluster('cjob_fillMCChargeFlipMeasurement', cmds,
		                   cmssw_version=CMSSW_VERSION)
