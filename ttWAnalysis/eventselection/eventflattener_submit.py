####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['signalregion_dilepton','signalregion_trilepton','signalregion_dimuon','signalregion_dielectron','signalregion_dilepton_em','signalregion_dilepton_me']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)
#for r in ['nonprompt_trilepton']: regions.append(r)
#for r in ['nonprompt_dilepton']: regions.append(r)
#for r in ['4lepton_controlregion', 'nonprompt_dilepton_invMET']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

selection_types = []
selection_types.append('tight')
#selection_types.append('prompt')
#selection_types.append('fakerate')

variations = []
variations.append('nominal')

dtypes = ['sim']

frdir = '../fakerates'

samplelistdir= '../samplelists/fourtops'
samplelistbase = 'samples_tttt_{}_{}.txt'


for year in years:
    for region in regions:
        for variation in variations:
            inputdir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4'
            inputdir = os.path.join(inputdir, year)

            samplelist = os.path.join(samplelistdir,samplelistbase.format(year,'sim'))
            outputdir = 'flattened_roots_all'
            outputdir = os.path.join(outputdir, '{}_{}'.format(year, region))
            cmd = 'python eventflattener.py'
            cmd += ' --inputdir ' + inputdir
            cmd += ' --samplelist ' + samplelist
            cmd += ' --outputdir ' + outputdir
            cmd += ' --event_selection ' + region
            #cmd += ' --selection_type '
            #for s in selection_types: cmd += ' ' + s
            cmd += ' --variation ' + variation
            cmd += ' --frdir ' + frdir
            cmd += ' --nevents -1'
            cmd += ' --runmode condor'
            print('executing '+cmd)
            os.system(cmd)
