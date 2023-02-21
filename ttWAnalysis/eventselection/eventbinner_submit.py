#################################################################################################
# A very simple submitter that runs eventbinner.py for a number of predefined regions and years #
#################################################################################################

import os
import sys

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']
#years = ['2016PreVFP']

dtypes = []
dtypes.append('sim')
dtypes.append('data')

selection_types = []
selection_types.append('tight')
#selection_types.append('prompt')
#selection_types.append('fakerate')
#selection_types.append('chargeflips')
#selection_types.append('chargegood')
#selection_types.append('irreducible')

variations = []
variations.append('nominal')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_{}.txt'

variables = '../variables/variables_main.json'

#bdtfile = None
<<<<<<< HEAD
bdtfile = '../bdtweights/XGBfinal_all.root'

nevents = 1e6
=======
#bdtfile = '../bdtweights/XGBfinal_all.root'
bdtfile = '../bdtweights/XGB_dummyanalysis_all_30features_lepMVA_removed.root'
#bdtfile = '../bdtweights/XGBfinal_all_newbackgrd_30features_lepMVA_removed_withbettergridsearchshort.root'
nevents = -1
>>>>>>> tmp

runmode = 'condor'

outputdir = 'output_test'

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
    thisoutputdir = os.path.join(outputdir, '{}_{}'.format(year, dtype))
    # make the command
    cmd = 'python eventbinner.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --outputdir ' + thisoutputdir
    cmd += ' --event_selection'
    for r in regions: cmd += ' ' + r
    cmd += ' --selection_type'
    for s in selection_types: cmd += ' ' + s
    cmd += ' --variation'
    for v in variations: cmd += ' ' + v
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --variables ' + variables
    cmd += ' --runmode ' + runmode
    if nevents > 0: cmd += ' --nevents ' + str(int(nevents))
    if bdtfile is not None: cmd += ' --bdt ' + bdtfile
    print('executing '+cmd)
    os.system(cmd)
