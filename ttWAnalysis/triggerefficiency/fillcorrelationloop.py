################################################################################
# Looper to do trigger correlation measurement with conventional folder naming #
################################################################################

import sys
import os

topdir = 'output_temp_correlation'

years = []
years.append('2016PreVFP')
years.append('2016PostVFP')
years.append('2017')
years.append('2018')

dtypes = []
dtypes.append('data')

nevents = 0

runmode = 'condor'

for year in years:
  for dtype in dtypes:
      inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw'
      samplelist = 'samplelists/samples_trigger_{}_{}.txt'.format(year,dtype)
      outputdir = os.path.join(topdir,year, dtype)
      if os.path.exists(outputdir): os.system('rm -r '+outputdir)
      command = 'python fillcorrelation.py'
      command += ' --inputdir {}'.format(inputdir)
      command += ' --samplelist {}'.format(samplelist)
      command += ' --outputdir {}'.format(outputdir)
      command += ' --nevents {}'.format(nevents)
      command += ' --runmode {}'.format(runmode)
      os.system(command)
