#################################################################################################
# A very simple submitter that runs runanalysis.py for a number of predefined regions and years #
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

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

samplelistdir = '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_{}.txt'

variables = '../variables/variables_main.json'

outputdir = 'output_20221124'

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
    if runlocal: cms += ' --runmode local'
    if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
    # consider different submission strategies
    if( submit_event_selections_combined and submit_selection_types_combined ):
      # submit jobs combined in event selections and selection types
      thiscmd = cmd
      thiscmd += ' --event_selection'
      for region in regions: thiscmd += ' '+region
      thiscmd += ' --selection_type'
      for selection_type in selection_types: thiscmd += ' '+selection_type
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
        print('executing '+thiscmd)
        os.system(thiscmd)
    elif( not submit_event_selections_combined and not submit_selection_types_combined ):
      # submit jobs separately for event selections and selection types
      for region in regions:
        for selection_type in selection_types:
          thiscmd = cmd
          thiscmd += ' --event_selection ' + region
          thiscmd += ' --selection_type ' + selection_type
          print('executing '+thiscmd)
          os.system(thiscmd)
