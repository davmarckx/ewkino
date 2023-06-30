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

event_selections = []
event_selections.append('3tight_recoptcuts')
event_selections.append('2tightss_recoptcuts')

dtypes = []
dtypes.append('data')

nevents = 0

runmode = 'condor'

for year in years:
  for event_selection in event_selections:
    for dtype in dtypes:
      inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_data'
      samplelist = 'samplelists/samples_trigger_{}_{}.txt'.format(year,dtype)
      outputdir = os.path.join(topdir,year,event_selection,dtype)
      pt_threshold_id = 'notset'
      if '3' in event_selection: pt_threshold_id = 'ttwtrilep'
      if '2' in event_selection: pt_threshold_id = 'ttwdilep'
      if os.path.exists(outputdir): os.system('rm -r '+outputdir)
      command = 'python fillcorrelation.py'
      command += ' --inputdir {}'.format(inputdir)
      command += ' --samplelist {}'.format(samplelist)
      command += ' --outputdir {}'.format(outputdir)
      command += ' --event_selection {}'.format(event_selection)
      command += ' --pt_threshold_id {}'.format(pt_threshold_id)
      command += ' --nevents {}'.format(nevents)
      command += ' --runmode {}'.format(runmode)
      print('Now running:')
      print(command)
      os.system(command)
