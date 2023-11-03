#################################################################################################
# A very simple submitter that runs runanalysis.py for a number of predefined regions and years #
#################################################################################################
# - update: can also be used for the runanalysis2 executable!
#   runanalysis2 is used for the signal region for differential measurements,
#   where all samples are put into 2D histograms
#   and additionally the signal is split into particle level bins.

import os
import sys

exe = 'runanalysis2'

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
#for r in ['nplownjetscontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)
#for r in ['cfjetscontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']
#years = ['2018']

dtypes = ['sim','data']
#dtypes = ['sim']
#dtypes = ['data']

selection_types = []
#selection_types.append('tight')
#selection_types.append('prompt')
#selection_types.append('fakerate')
selection_types.append('efakerate')
selection_types.append('mfakerate')
#selection_types.append('chargeflips')
#selection_types.append('chargegood')
#selection_types.append('irreducible')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/backgrounds' # background sample lists
samplelistbase = 'samples_tttt_{}_{}.txt' # background sample lists
#samplelistdir = 'samplelists' # sample lists for testing
#samplelistbase = 'samplelist_test_{}_WZ.txt' # sample lists for testing
#samplelistdir = '../samplelists/particlelevel' # sample lists for TTW signal samples
#samplelistbase = 'samplelist_{}_TTW_particlelevel.txt' # sample lists for TTW signal samples

#variables = '../variables/variables_main.json' # single variables
variables = '../variables/variables_main_reduced.json' # single variables (slightly reduced set)
#variables = '../variables/variables_eventbdt.json' # bdt variable
#variables = '../variables/variables_crfit.json' # reduced set of variables for CRs in fit
#variables = '../variables/variables_particlelevel_double.json' # double variables

#bdtfile = None
bdtfile = '../bdtweights/v20230601/XGBrobustnessv3_all.root'
#bdtfile = '../../ML/models/XGBrobustnessv3_all.root'
bdtcut = None

splitprocess = None # do not split any process at particle level
#splitprocess = 'TTW' # split TTW process at particle level
splitvariables = None
#splitvariables = '../variables/variables_particlelevel_single.json'

outputdir = 'output_20231019_test'

nevents = 1e6
runlocal = False

submit_selection_types_combined = True
submit_event_selections_combined = True

# loop over years and data types
for year in years:
  for dtype in dtypes:
    # set correct input directory
    inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
    inputdiryear = year
    #inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_signal'
    #inputdiryear = ''
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
    cmd = 'python runanalysis.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --outputdir ' + thisoutputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --frdir ' + frdir
    cmd += ' --cfdir ' + cfdir
    cmd += ' --variables ' + variables
    cmd += ' --exe ' + exe
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
    elif( not submit_event_selections_combined and submit_selection_types_combined ):
      # submit jobs separately for event selections
      # but combined in selection types
      for region in regions:
        thiscmd = cmd
        thiscmd += ' --event_selection ' + region
        thiscmd += ' --selection_type'
        for selection_type in selection_types: thiscmd += ' '+selection_type
        thiscmd += ' --output_append'
        print('executing '+thiscmd)
        os.system(thiscmd)
    elif( submit_event_selections_combined and not submit_selection_types_combined ):
      # submit jobs combined in event selections
      # but separately in selection types
      for selection_type in selection_types:
        thiscmd = cmd
        thiscmd += ' --event_selection'
        for region in regions: thiscmd += ' '+region
        thiscmd += ' --selection_type ' + selection_type
        thiscmd += ' --output_append'
        print('executing '+thiscmd)
        os.system(thiscmd)
    elif( not submit_event_selections_combined and not submit_selection_types_combined ):
      # submit jobs separately for event selections and selection types
      for region in regions:
        for selection_type in selection_types:
          thiscmd = cmd
          thiscmd += ' --event_selection ' + region
          thiscmd += ' --selection_type ' + selection_type
          thiscmd += ' --output_append'
          print('executing '+thiscmd)
          os.system(thiscmd)
