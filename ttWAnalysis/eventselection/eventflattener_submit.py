####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

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

selection_types = []
selection_types.append('tight')
#selection_types.append('prompt')
#selection_types.append('fakerate')
#selection_types.append('chargeflips')
#selection_types.append('chargegood')
#selection_types.append('irreducible')

variations = []
variations.append('nominal')
variations.append('JECUp')
variations.append('JECDown')


dtypes = []
dtypes.append('sim')
#dtypes.append('data')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/fourtops_notused'
samplelistbase = 'samples_tttt_{}_{}.txt'

#bdtfile = None
bdtfile = '../bdtweights/XGBfinal_all_newbackgrd_30features_lepMVA_removed_withbettergridsearchshort.root'


nevents = -1

outputdir = 'flattened_robustnessv2'

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
    # make basic command
    cmd = 'python eventflattener.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --runmode condor'
    if nevents > 0: cmd += ' --nevents ' + str(int(nevents))
    if bdtfile is not None: cmd += ' --bdt ' + bdtfile
    # loop over selections, types and variations
    for region in regions:
      for selection_type in selection_types:
        for variation in variations:
          thiscmd = cmd
          # set correct output directory
          thisoutputdir = os.path.join(outputdir,
            '{}_{}'.format(year, dtype),
            '{}_{}_{}'.format(region, selection_type, variation))
          thiscmd += ' --event_selection ' + region
          thiscmd += ' --selection_type ' + selection_type
          thiscmd += ' --variation ' + variation
          thiscmd += ' --outputdir ' + thisoutputdir
          print('executing '+thiscmd)
          os.system(thiscmd)
