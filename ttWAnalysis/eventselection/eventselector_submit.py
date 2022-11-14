####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['cfcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

selection_types = []
selection_types.append('tight')
selection_types.append('prompt')
selection_types.append('fakerate')

variations = []
variations.append('nominal')

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
        print('executing '+cmd)
        os.system(cmd)
