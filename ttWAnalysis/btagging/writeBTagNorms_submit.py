####################################################################################################
# A very simple submitter that runs writeBTagNorms.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['signalregion_trilepton']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
for r in ['cfcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

samplelistdir = '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_sim.txt'

nevents = 1e6

runmode = 'condor'

outputdir = 'output_20230215'

for year in years:
    # set correct input directory
    inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
    inputdir = os.path.join(inputdir, year)
    # set correct sample list
    samplelist = os.path.join(samplelistdir,samplelistbase.format(year))
    # set correct output directory
    thisoutputdir = os.path.join(outputdir, '{}'.format(year))
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
