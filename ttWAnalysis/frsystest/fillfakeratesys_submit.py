import os
import sys

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))

years = ['2016PreVFP','2016PostVFP','2017','2018']

dtypes = ['sim','data']

selection_types = []
selection_types.append('fakerate')
selection_types.append('efakerate')
selection_types.append('mfakerate')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = 'samplelists/'
samplelistbase = 'samplelist_test_{}_{}.txt'

variables = '../variables/variables_main_reduced.json'

outputdir = 'output_20231018'

nevents = -1

runlocal = False

# loop over years and data types
for year in years:
  for dtype in dtypes:
    # set correct input directory
    inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
    inputdiryear = year
    if dtype=='data':
      inputdir = inputdir.replace('_v4','_v5')
      if( year=='2016PreVFP' or year=='2016PostVFP' ):
        inputdiryear = '2016'
    inputdir = os.path.join(inputdir, inputdiryear)
    # set correct sample list
    samplelist = os.path.join(samplelistdir,samplelistbase.format(year,dtype))
    # set correct output directory
    thisoutputdir = os.path.join(outputdir, '{}_{}'.format(year,dtype))
    # make the basic command
    cmd = 'python fillfakeratesys.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --outputdir ' + thisoutputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --variables ' + variables
    if runlocal: cmd += ' --runmode local'
    if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
    # submit jobs combined in event selections and selection types
    cmd += ' --event_selection'
    for region in regions: cmd += ' '+region
    cmd += ' --selection_type'
    for selection_type in selection_types: cmd += ' '+selection_type
    print('executing '+cmd)
    os.system(cmd)
