###############################################################################
# Looper to do trigger efficiency measurement with conventional folder naming #
###############################################################################

import sys
import os

# settings for 1D measurements

topdir = 'output_temp_1d'

years = []
years.append('2016PreVFP')
years.append('2016PostVFP')
years.append('2017')
years.append('2018')

event_selections = []
event_selections.append('3tight_recoptcuts')
event_selections.append('3fo_tightveto_recoptcuts')
event_selections.append('2tightss_recoptcuts')
event_selections.append('2foss_tightveto_recoptcuts')

dtypes = []
dtypes.append('sim')
dtypes.append('data')

nevents = 0

runmode = 'condor'

for year in years:
  for event_selection in event_selections:
    for dtype in dtypes:
      inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4/{}'.format(year)
      if dtype=='data': inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw'
      samplelist = 'samplelists/samples_trigger_{}_{}.txt'.format(year,dtype)
      outputdir = os.path.join(topdir,year,event_selection,dtype)
      pt_threshold_id = 'notset'
      if '3' in event_selection: pt_threshold_id = 'ttwtrilep'
      if '2' in event_selection: pt_threshold_id = 'ttwdilep'
      if os.path.exists(outputdir): os.system('rm -r '+outputdir)
      command = 'python filltrigger.py'
      command += ' --dimension 1'
      command += ' --inputdir {}'.format(inputdir)
      command += ' --samplelist {}'.format(samplelist)
      command += ' --outputdir {}'.format(outputdir)
      command += ' --event_selection {}'.format(event_selection)
      command += ' --pt_threshold_id {}'.format(pt_threshold_id)
      command += ' --nevents {}'.format(nevents)
      command += ' --runmode {}'.format(runmode)
      os.system(command)

# settings for 2D measurements

topdir = 'output_temp_2d'

years = []
years.append('2016PreVFP')
years.append('2016PostVFP')
years.append('2017')
years.append('2018')

event_selections = []
event_selections.append('legacy_recoptcuts')

dtypes = []
dtypes.append('sim')
dtypes.append('data')

nevents = 0

runmode = 'condor'

for year in years:
  for event_selection in event_selections:
    for dtype in dtypes:
      inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4/{}'.format(year)
      if dtype=='data': inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw'
      samplelist = 'samplelists/samples_trigger_{}_{}.txt'.format(year,dtype)
      outputdir = os.path.join(topdir,year,event_selection,dtype)
      pt_threshold_id = 'ttwdilep'
      if os.path.exists(outputdir): os.system('rm -r '+outputdir)
      command = 'python filltrigger.py'
      command += ' --dimension 2'
      command += ' --inputdir {}'.format(inputdir)
      command += ' --samplelist {}'.format(samplelist)
      command += ' --outputdir {}'.format(outputdir)
      command += ' --event_selection {}'.format(event_selection)
      command += ' --pt_threshold_id {}'.format(pt_threshold_id)
      command += ' --nevents {}'.format(nevents)
      command += ' --runmode {}'.format(runmode)
      os.system(command)
