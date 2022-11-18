#################################################################################################
# A very simple submitter that runs eventbinner.py for a number of predefined regions and years #
#################################################################################################

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

dtypes = ['sim','data']

selection_types = []
selection_types.append('tight')
selection_types.append('prompt')
selection_types.append('fakerate')
selection_types.append('chargeflips')
selection_types.append('chargegood')
selection_types.append('irreducible')

variations = []
variations.append('nominal')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_{}.txt'

variables = '../variables/variables_main.json'

istest = False

for year in years:
  for dtype in dtypes:
    inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
    inputdiryear = year
    if dtype=='data':
      inputdir = inputdir.replace('_v4','_v5')
      if( year=='2016PreVFP' or year=='2016PostVFP' ):
        inputdiryear = '2016'
    inputdir = os.path.join(inputdir, inputdiryear)
    samplelist = os.path.join(samplelistdir,samplelistbase.format(year,dtype))
    outputdir = 'output_20221117'
    outputdir = os.path.join(outputdir, '{}_{}'.format(year,dtype))
    cmd = 'python eventbinner.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --outputdir ' + outputdir
    cmd += ' --event_selection'
    for r in regions: cmd += ' ' + r
    cmd += ' --selection_type'
    for s in selection_types: cmd += ' ' + s
    cmd += ' --variation'
    for v in variations: cmd += ' ' + v
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --variables ' + variables
    if istest:
      cmd += ' --runmode local'
      cmd += ' --nevents 1000'
    print('executing '+cmd)
    os.system(cmd)
