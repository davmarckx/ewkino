#!/usr/bin/env python

###############################################
# a python submitter for fillMCClosureTest.cc #
###############################################

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
# (only 'electron' supported for now)
processes = ['DY','TT']
# (choose from 'DY' and 'TT')
runmode = 'condor'
# (pick from 'condor' or 'local')
nentries = -1
# (number of entries to use per file)
samplelistdirectory = os.path.abspath('../chargeFlips/sampleListsUL')
samplelist = 'samples_closureTest_chargeFlips_{}_{}.txt'
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_chargeflips/sim/{}'
# (directory where samples are located)
chargeflipmap = 'chargeFlipMaps_v20221109/'
chargeflipmap += 'chargeFlipMap_MC_electron_{}_process_DY_binning_Gianny.root'

# check if executable exists
exe = './fillMCClosureTest'
if not os.path.exists(exe):
  raise Exception('ERROR: executable {} does not seem to exist...'.format(exe))

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
  for flavour in flavours:
    for process in processes:
      # make path to sample list
      thissampledir = sampledirectory.format(year)
      thissamplelist = samplelist.format(process,year)
      thissamplelist = os.path.join(samplelistdirectory,
                       thissamplelist)
      thischargeflipmap = chargeflipmap.format(year)
      # check number of samples
      samples = readsamplelist( thissamplelist, sampledir=thissampledir )
      nsamples = samples.number()
      print('Found {} samples.'.format(nsamples))
      # make the command and add it to the list
      command = '{} {} {} {} {} {} {} {}'.format(exe,
                  process, flavour, year,
                  thissamplelist, thissampledir, thischargeflipmap,
		  nentries)
      cmds.append(command)

# submit the commands as jobs
if( runmode=='local' ):
  for cmd in cmds: os.system(cmd)
elif( runmode=='condor' ):
  ct.submitCommandsAsCondorCluster('cjob_fillMCClosureTest', cmds,
		                   cmssw_version=CMSSW_VERSION)
