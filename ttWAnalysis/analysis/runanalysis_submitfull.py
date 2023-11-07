##################################
# Submit full analysis in one go #
##################################

import os
import sys

# submission settings
submit_event_selections_combined = True
submit_selection_types_combined = True

# loop over variable types
mtypes = ['single', 'double']
for mtype in mtypes:

  # define regions
  regions = []
  if mtype=='single':
    for r in ['signalregion_dilepton_inclusive']: regions.append(r)
    #for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
    for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
    for r in ['signalregion_trilepton']: regions.append(r)
    #for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
    for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
    for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
    #for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
    for r in ['nplownjetscontrolregion_dilepton_inclusive']: regions.append(r)
    for r in ['cfcontrolregion']: regions.append(r)
    for r in ['cfjetscontrolregion']: regions.append(r)
  elif mtype=='double':
    for r in ['signalregion_dilepton_inclusive']: regions.append(r)
    for r in ['signalregion_trilepton']: regions.append(r)
  else:
    raise Exception('ERROR: mtype {} not recognized'.format(mtype))

  # define years
  years = ['2016PreVFP','2016PostVFP','2017','2018']

  # define selection types
  selection_types = []
  selection_types.append('fakerate')
  selection_types.append('efakerate')
  selection_types.append('mfakerate')
  selection_types.append('chargeflips')
  selection_types.append('irreducible')

  # define fake rate and charge flip map location
  frdir = '../fakerates/fakeRateMaps_v20220912_tttt'
  cfdir = '../chargefliprates/chargeFlipMaps_v20221109'

  # loop over data types
  dtypes = ['sig', 'bkg', 'data']
  for dtype in dtypes:

    # define sample lists
    samplelistdir = None
    samplelistbase = None
    if dtype=='sig':
      samplelistdir = '../samplelists/particlelevel'
      samplelistbase = 'samplelist_{}_TTW_particlelevel.txt'
    elif dtype=='bkg':
      samplelistdir = '../samplelists/backgrounds'
      samplelistbase = 'samples_tttt_{}_sim.txt'
    elif dtype=='data':
      samplelistdir = '../samplelists/backgrounds'
      samplelistbase = 'samples_tttt_{}_data.txt'

    # define variables
    variables = None
    if mtype=='single':
      #variables = '../variables/variables_main.json'
      variables = '../variables/variables_main_reduced.json'
    elif mtype=='double':
      #variables = '../variables/variables_particlelevel_double.json'
      variables = '../variables/variables_particlelevel_double_bintest.json'

    # define BDT file
    bdtfile = '../bdtweights/v20230601/XGBrobustnessv3_all.root'
    bdtcut = None

    # define splitting at particle level
    splitprocess = None # do not split any process at particle level
    if( dtype=='sig' and mtype=='double' ):
      splitprocess = 'TTW' # split TTW process at particle level

    # define output directory
    datetag = 'test'
    outputdir = 'output_{}_{}_{}'.format(datetag, mtype, dtype)

    # define number of events
    nevents = 1e6
    runlocal = False

    # loop over years
    for year in years:
      
      # set correct input directory
      inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
      inputdiryear = year
      if dtype=='sig':
        inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_signal'
        inputdiryear = ''
      if dtype=='data':
        inputdir = inputdir.replace('_v4','_v5')
        if( year=='2016PreVFP' or year=='2016PostVFP' ): inputdiryear = '2016'
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
      #if splitvariables is not None: cmd += ' --splitvarfile {}'.format(splitvariables)
      if runlocal: cmd += ' --runmode local'
      if nevents!=0: cmd += ' --nevents {}'.format(int(nevents))
      if bdtfile is not None: cmd += ' --bdt ' + bdtfile
      if bdtcut is not None: cmd += ' --bdtcut {}'.format(bdtcut)
      cmd += ' --systematics default'
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
