########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct

inputdir = sys.argv[1]
runmode = 'condor'

regions = []
for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)
#for r in ['nonprompt_dilepton']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromsim','npfromdata']

cmds = []
for year in years:
    for npmode in npmodes:
	subdir = os.path.join(year, 'merged_'+npmode)
	inputfile = os.path.join(inputdir, subdir, 'merged.root')
	if not os.path.exists(inputfile):
	    print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
	    continue
	for region in regions:
	    thisoutputdir = os.path.join(inputdir, subdir, 'plots', year+'_'+region+'_'+npmode)
	    doblind = False
	    if 'signalregion' in region: doblind = True
	    cmd = 'python makeplots.py '+inputfile+' '+year+' '+region+' '+thisoutputdir
	    cmd += ' '+str(doblind)
	    if runmode=='local':
		print('executing '+cmd)
		os.system(cmd)
	    elif runmode=='condor':
		print('submitting '+cmd)
		cmds.append(cmd)
	    else:
		raise Exception('ERROR: runmode "{}" not recognized')

if runmode=='condor':
    ct.submitCommandsAsCondorCluster('cjob_makeplots', cmds)
