#################################################################################################
# A very simple submitter that runs runanalysis.py for a number of predefined regions and years #
#################################################################################################

import os
import sys

regions = []
#for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion']: regions.append(r)
for r in ['zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)
#for r in ['nonprompt_trilepton']: regions.append(r)
#for r in ['nonprompt_dilepton']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

dtypes = ['sim','data']

selection_types = []
selection_types.append('tight')
#selection_types.append('prompt')
#selection_types.append('fakerate')

frdir = '../fakerates/fakeRateMaps_v20220912_tttt'

samplelistdir = '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_{}.txt'

variables = '../variables/variables_main.json'

outputdir = 'output_20221104'

nevents = 1e6
runlocal = False

submitcombined = True

# loop over years and data types
for year in years:
  for dtype in dtypes:
    # remove prompt selection type for data
    this_selection_types = selection_types[:]
    if( dtype=='data' and 'prompt' in this_selection_types ):
      this_selection_types.remove('prompt')
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
    # consider different submission strategies
    if submitcombined:
      # submit jobs combined in event selections and selection types
      thisoutputdir = os.path.join(outputdir, '{}_{}'.format(year,dtype))
      cmd = 'python runanalysis.py'
      cmd += ' --inputdir ' + inputdir
      cmd += ' --samplelist ' + samplelist
      cmd += ' --outputdir ' + thisoutputdir
      cmd += ' --event_selection'
      for region in regions: cmd += ' '+region
      cmd += ' --selection_type'
      for selection_type in this_selection_types: cmd += ' '+selection_type
      cmd += ' --frdir ' + frdir
      cmd += ' --variables ' + variables
      if runlocal: cms += ' --runmode local'
      if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
      print('executing '+cmd)
      os.system(cmd)
    else:
      # submit jobs separately for event selections and selection types
      for region in regions:
        for selection_type in this_selection_types:
          # set output directory
          thisoutputdir = os.path.join(outputdir, 
                          '{}_{}'.format(year,dtype),
                          region, selection_type)
          cmd = 'python runanalysis.py'
          cmd += ' --inputdir ' + inputdir
          cmd += ' --samplelist ' + samplelist
          cmd += ' --outputdir ' + thisoutputdir
          cmd += ' --event_selection ' + region
          cmd += ' --selection_type ' + selection_type
          cmd += ' --frdir ' + frdir
          cmd += ' --variables ' + variables
          if runlocal: cms += ' --runmode local'
          if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
          print('executing '+cmd)
          os.system(cmd)
