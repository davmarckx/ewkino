################################################################################################
# a python script for submission of histfiller.py and histplotter.py to make all plots at once #
################################################################################################
import json
import os
import sys
import numpy as np
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

### Define regions to make plots for
regions = []
for r in ['signalregion']: regions.append(r)
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['signalsideband_noz','signalsideband_noossf']: regions.append(r)
years = ['2016']
ID = 'oldtzq' # does NOT set ID correctly, simply for folder management
usedata = True
donpfromsim = True
donpfromdata = True

### Global settings
outdir = 'histograms_0731_oldtzqid'
outdir = os.path.abspath(outdir)
variables = [
    {'name':'_abs_eta_recoil','bins':list(np.linspace(0,5,num=21)),
     'title':r'#||{#eta}_{recoil}','unit':''},
    {'name':'_Mjj_max','bins':list(np.linspace(0,1200,num=21)),
     'title':r'M_{jet+jet}^{max}','unit':'GeV'},
    {'name':'_lW_asymmetry','bins':list(np.linspace(-2.5,2.5,num=21)),
     'title':r'asymmetry (lepton from W)','unit':''},
    {'name':'_deepCSV_max','bins':list(np.linspace(0,1,num=21)),
     'title':r'highest deepCSV','unit':''},
    {'name':'_lT','bins':list(np.linspace(0,800,num=21)),
     'title':'L_{T}','unit':'GeV'},
    {'name':'_MT','bins':list(np.linspace(0,300,num=21)),
     'title':'M_{T}','unit':'GeV'},
    {'name':'_pTjj_max','bins':list(np.linspace(0,300,num=21)),
     'title':r'p_T^{max}(jet+jet)','unit':'GeV'},
    {'name':'_dRlb_min','bins':list(np.linspace(0,3.14,num=21)),
     'title':r'#Delta R(lep,bjet)_{min}','unit':''},
    {'name':'_dPhill_max','bins':list(np.linspace(0,3.14,num=21)),
     'title':r'#Delta #Phi (lep,lep)_{max}','unit':''},
    {'name':'_HT','bins':list(np.linspace(0,800,num=21)),
     'title':r'H_{T}','unit':'GeV'},
    {'name':'_nJets','bins':list(np.linspace(-0.5,9.5,num=11)),
     'title':r'number of jets','unit':''},
    {'name':'_dRlWrecoil','bins':list(np.linspace(0,10,num=21)),
     'title':r'#Delta R(lep_{W},jet_{recoil})','unit':''},
    {'name':'_dRlWbtagged','bins':list(np.linspace(0,7,num=21)),
     'title':r'#Delta R(lep_{W},jet_{b-tagged})','unit':''},
    {'name':'_M3l','bins':list(np.linspace(0,600,num=21)),
     'title':r'M_{3l}','unit':'GeV'},
    {'name':'_abs_eta_max','bins':list(np.linspace(0,5,num=21)),
     'title':r'#||{#eta}_{max}','unit':''},
    {'name':'_eventBDT','bins':list(np.linspace(-1,1,num=21)),
     'title':r'event BDT score','unit':''},
    {'name':'_nMuons','bins':list(np.linspace(-0.5,3.5,num=5)),
     'title':r'number of muons','unit':''},
    {'name':'_nElectrons','bins':list(np.linspace(-0.5,3.5,num=5)),
     'title':r'number of electrons','unit':''}
]
doextraselection = False

### Set output directory
# check if some of the output directories already exist
exists = False
for region in regions:
    for year in years:
	outpath = os.path.join(outdir,year,region)
	if os.path.exists(outpath):
	    exists = True
# ask for confirmation to overwrite
if exists:
    print('### WARNING ###: some of the output directories already exist!')
    print('                 Overwrite? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()
# overwrite
for region in regions:
    for year in years:
	outpath = os.path.join(outdir,year,region)
	if os.path.exists(outpath): os.system('rm -r '+outpath)
	os.makedirs(outpath)

### Run over input lists
currentdir = os.getcwd()
if ID=='tth': interpendix='tthid'
elif ID=='tzq': interpendix='tzqid'
elif ID=='oldtzq': interpendix='oldtzqid'
else:
    print('### ERROR ###: ID "'+ID+'"not recognized.')
    sys.exit()
npmethods = []
if donpfromsim: npmethods.append('npfromsim')
if donpfromdata: npmethods.append('npfromdata')
for region in regions:
    for year in years:
	for npmethod in npmethods:
	    outpath = os.path.join(outdir,year,region)
	    # set input path, see histfiller.py for example
	    mcrootdir = os.path.join('/user/llambrec/Files',interpendix,year+'MC',region)
	    mcsamplelist = '../samplelists'
	    mcsamplelist += '/samplelist_tzq_'+year+'_MC.txt'
	    datarootdir = os.path.join('/user/llambrec/Files',interpendix,year+'data',region)
	    datasamplelist = '../samplelists'
	    datasamplelist += '/samplelist_tzq_'+year+'_data.txt'
	    # special case for signal region
	    if region == 'signalregion':
		subtrees = ['treeCat1','treeCat2','treeCat3']
		subfolders = ['cat1','cat2','cat3']
	    else:
		subtrees = ['blackJackAndHookersTree']
		subfolders = ['']
	    # make plot commands and submit script
	    for tree,subfolder in zip(subtrees,subfolders):
		suboutpath = os.path.join(outpath,subfolder,npmethod)
		histfile = os.path.join(suboutpath,'histograms.root')
		normalization = 1
		lumi = 35900
		if year=='2017': lumi = 41500
		if year=='2018': lumi = 59700
		script_name = 'plotting.sh'
		command1 = 'python histfiller.py'
		command2 = 'python histplotter.py'
		with open(script_name,'w') as script:
		    initializeJobScript( script )
		    script.write('cd {}\n\n'.format(currentdir))
		    command1 += ' {} {}'.format(mcrootdir,mcsamplelist)
		    command1 += ' {} {}'.format(datarootdir,datasamplelist)
		    command1 += ' {} {}'.format(histfile,"'"+json.dumps(variables,separators=(',',':'))+"'")
		    command1 += ' {} {} {}'.format(tree,normalization,lumi)
		    command1 += ' {} {} {}\n\n'.format(doextraselection,usedata,npmethod=='npfromdata')
		    script.write(command1) 
		    command2 += ' {} {}\n'.format(histfile,"'"+json.dumps(variables,separators=(',',':'))+"'")
		    script.write(command2)
		# for testing: run sequentially on m-machine
		#os.system(command1)
		#sys.exit()
		#os.system(command2)
		submitQsubJob(script_name)
