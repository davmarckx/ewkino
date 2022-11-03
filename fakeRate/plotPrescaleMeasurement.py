####################################################
# a python looper for plotMCPrescaleMeasurement.cc #
####################################################

import os
import sys
import ROOT
sys.path.append('../plotting/python')
import histplotter as hp
sys.path.append('../Tools/python')
import histtools as ht

years = ['2016PreVFP','2016PostVFP','2017','2018']
use_mT = True
plotmode = 'new'
# (choose from 'old' (with older c++ plotting function) or 'new' (with newer python version))

# check if executable exists
if not os.path.exists('./plotPrescaleMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makePlotPrescaleMeasurement first.')
    sys.exit()

# hadd files if needed
for year in years:
    basename = 'prescaleMeasurement'
    if use_mT: basename += '_mT'
    else: basename += '_met'
    basename += '_histograms_'+year
    filename = basename + '.root'
    if os.path.exists(filename): continue
    subfolder = '.'
    cmd = 'hadd '+filename+' '
    cmd += os.path.join(subfolder,basename+'_sample*.root')
    print(cmd)
    os.system(cmd)

# loop over years
cwd = os.getcwd()
for year in years:
    
    ### step 1: fit the prescale (calling C++ function)
    # check if input file exists
    basename = 'prescaleMeasurement'
    if use_mT: basename += '_mT'
    else: basename += '_met'
    basename += '_histograms_'+year
    filename = basename + '.root'
    if not os.path.exists(filename):
        print('### ERROR ###: file '+filename+' not found, skipping it.')
        continue
    # fit the prescales and store the scaled histograms
    doplot = (plotmode=='old')
    dosave = True
    command = './plotPrescaleMeasurement {} {} {} {}'.format(str(use_mT), year, doplot, dosave)
    os.system(command)
    if plotmode!='new': continue

    ### step 2: plot the resulting histograms
    # check if the correct file was created
    filename = 'prescaleMeasurementPlots_{}.root'.format(year)
    if not os.path.exists(filename):
	print('### ERROR ###: file '+filename+' not found, skipping it.')
        continue
    # create the output directory
    outputdir = 'prescaleMeasurementPlots_{}'.format(year)
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)
    # read the histograms
    histlist = ht.loadallhistograms(filename)
    # get a list of triggers
    triggers = []
    for hist in histlist:
	trigger = 'HLT'+hist.GetName().split('HLT',1)[1]
	if trigger not in triggers: triggers.append(trigger)
    # loop over triggers
    for trigger in triggers:
	# find data histogram
	datahists = ht.selecthistograms(histlist, mustcontainall=[trigger,'data'])[1]
	if len(datahists)!=1:
	    raise Exception('ERROR: found {} data histograms'.format(len(datahists))
			    +' while expecting 1.')
	datahist = datahists[0]
	# find prompt histograms (split per process)
	prompthists = ht.selecthistograms(histlist, mustcontainall=[trigger,'_prompt'],
					    maynotcontainone=['nonprompt','unc','total','QCD'])[1]
        print('found {} prompt histograms'.format(len(prompthists)))
	for hist in prompthists:
	    print('  - {}'.format(hist.GetName()))
	# find nonprompt histograms
	nonprompthists = ht.selecthistograms(histlist, mustcontainall=[trigger,'_nonprompt','QCD'],
					    maynotcontainone=['unc','total'])[1]
        print('found {} nonprompt histograms'.format(len(nonprompthists)))
	for hist in nonprompthists:
            print('  - {}'.format(hist.GetName()))
	# merge lists of prompt and nonprompt histograms
	simhists = prompthists + nonprompthists
	# find systematic uncertainty histogram
	systunchists = ht.selecthistograms(histlist, mustcontainall=[trigger,'unc'])[1]
        if len(systunchists)>1:
            raise Exception('ERROR: found {} uncertainty histograms'.format(len(systunchists))
                            +' while expecting 1.')
        systunchist = systunchists[0]
	
	# plot settings
	xaxtitle = datahist.GetXaxis().GetTitle()
	yaxtitle = datahist.GetYaxis().GetTitle()
	lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
                    '2016PreVFP':19520, '2016PostVFP':16810 }
	lumi = lumimap[year]
	extracmstext = 'Preliminary'
	extrainfos = [trigger]
	colormap = {}
	colormap['WJets'] = ROOT.kAzure + 1
	colormap['TT'] = ROOT.kCyan + 1
	colormap['DY'] = ROOT.kBlue + 1
	colormap['VV'] = ROOT.kCyan - 7
	colormap['QCD'] = ROOT.kRed - 7
        colormap['Other nonprompt'] = ROOT.kRed + 1
	legendbox = [0.7,0.5,0.9,0.9]

	# make the plot
	hp.plotdatavsmc( os.path.join(outputdir, trigger), datahist, 
            simhists, mcsysthist=systunchist,
            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
            colormap=colormap, 
	    #labelmap=labelmap,
            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
            p1legendncols=1,p1legendbox=legendbox,
            extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=15 )
