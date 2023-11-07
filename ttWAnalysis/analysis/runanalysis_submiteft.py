#########################################
# Submit runanalysis.py for EFT samples #
#########################################

import os
import sys


regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['signalregion_trilepton']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

dtypes = ['sim']

selection_types = []
selection_types.append('irreducible')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/eft'
samplelistbase = 'samplelist_{}_TTW_eftinfo.txt'

variables = '../variables/variables_particlelevel_double.json' # double variables

bdtfile = '../bdtweights/v20230601/XGBrobustnessv3_all.root'
bdtcut = None

splitprocess = None
splitvariables = None

outputdir = 'output_20231106_double_eft'

nevents = 1e6
runlocal = False

submit_selection_types_combined = True
submit_event_selections_combined = True

# loop over years and data types
for year in years:
  for dtype in dtypes:
    # set correct input directory
    inputdir = '/pnfs/iihe/cms/store/user/llambrec/heavyNeutrinoTTWEFT'
    inputdiryear = ''
    inputdir = os.path.join(inputdir, inputdiryear)
    # set correct sample list
    samplelist = os.path.join(samplelistdir,samplelistbase.format(year,dtype))
    # set correct output directory
    thisoutputdir = os.path.join(outputdir, '{}_{}'.format(year,dtype))
    # make the basic command
    cmd = 'python runanalysis.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --outputdir ' + thisoutputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --variables ' + variables
    if splitprocess is not None: cmd += ' --splitprocess {}'.format(splitprocess)
    if splitvariables is not None: cmd += ' --splitvarfile {}'.format(splitvariables)
    if runlocal: cmd += ' --runmode local'
    if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
    if bdtfile is not None: cmd += ' --bdt ' + bdtfile
    if bdtcut is not None: cmd += ' --bdtcut {}'.format(bdtcut)
    # consider different submission strategies
    if( submit_event_selections_combined and submit_selection_types_combined ):
      # submit jobs combined in event selections and selection types
      # update: split event selections into partitions
      part_size = 3
      regions_parts = [regions[i:i+part_size] for i in range(0, len(regions), part_size)]
      for regions_part in regions_parts:
        thiscmd = cmd
        thiscmd += ' --event_selection'
        for region in regions_part: thiscmd += ' '+region
        thiscmd += ' --selection_type'
        for selection_type in selection_types: thiscmd += ' '+selection_type
        thiscmd += ' --output_append'
        print('executing '+thiscmd)
        os.system(thiscmd)
