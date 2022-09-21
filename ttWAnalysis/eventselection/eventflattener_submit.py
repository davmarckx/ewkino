####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)
for r in ['nonprompt_dilepton']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

selection_types = []
selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')

variations = []
variations.append('nominal')

frdir = 'todo'

samplelistdir= 'todo'

for year in years:
  for region in regions:
    for stype in selection_types:
      for variation in variations:
        inputdir = 'todo'
        samplelist = os.path.join(samplelistdir,'samples_{}.txt'.format(year))
        outputdir = 'todo'
        outputdir = os.path.join(outputdir, year)
        cmd = 'python eventbinner.py'
        cmd += ' --inputdir ' + inputdir
        cmd += ' --samplelist ' + samplelist
        cmd += ' --outputdir ' + outputdir
        cmd += ' --event_selection ' + region
        cmd += ' --selection_type ' + stype
        cmd += ' --variation ' + variation
        cmd += ' --frdir ' + frdir
        print('executing '+cmd)
        os.system(cmd)
