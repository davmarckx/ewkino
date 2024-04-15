####################################################################################################
# A very simple submitter that runs writeBTagNorms.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
for r in ['nplownjetscontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['cfcontrolregion']: regions.append(r)
for r in ['cfjetscontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

# settings for sample and file location
loc = {
  'bkg': {
    'samplelist': '../samplelists/backgrounds/samples_tttt_{}_sim.txt',
    'inputdir': '/pnfs/iihe/cms/store/user/nivanden/skims_v4/{}'
  },
  'sig': {
    'samplelist': '../samplelists/particlelevel/samplelist_{}_TTW_particlelevel.txt',
    'inputdir': '/pnfs/iihe/cms/store/user/llambrec/noskim_ttw_signal_nanoaodsettings'
  }
}

nevents = 1e6

runmode = 'condor'

outputdir = 'output_test'

for year in years:
  for sampletype, sampleloc in loc.items():
    # set correct input directory
    inputdir = sampleloc['inputdir'].format(year)
    # set correct sample list
    samplelist = sampleloc['samplelist'].format(year)
    # set correct output directory
    thisoutputdir = os.path.join(outputdir, '{}_{}'.format(year, sampletype))
    # make the command
    cmd = 'python writeBTagNorms.py'
    cmd += ' --inputdir ' + inputdir
    cmd += ' --samplelist ' + samplelist
    cmd += ' --outputdir ' + thisoutputdir
    cmd += ' --event_selection'
    for r in regions: cmd += ' ' + r
    cmd += ' --runmode ' + runmode
    if nevents > 0: cmd += ' --nevents ' + str(int(nevents))
    print('executing '+cmd)
    os.system(cmd)
