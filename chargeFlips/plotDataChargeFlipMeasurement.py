###########################################
# plot the results of a data closure test #
###########################################

import os
import sys
import ROOT
import numpy as np
import matplotlib.pyplot as plt
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
import histplotter as hp
from constantfit import fitConstant

def plot_fitresult_summary( fitresults, title=None ):
    ### make summary plot of fit results for all variables
    # input argument:
    # - fitresults: a dict mapping names (e.g. variable name) 
    #   to dicts of fit results (with keys as in fitConstant)
    fig,ax = plt.subplots()
    # make x axis and data
    sortedkeys = sorted(fitresults.keys())
    xax = np.arange(len(sortedkeys))
    values = np.array([fitresults[k]['value'] for k in sortedkeys])
    uncs = np.array([fitresults[k]['uncertainty'] for k in sortedkeys])
    normchi2s = np.array([fitresults[k]['normalizedchi2'] for k in sortedkeys])
    successs = np.array([fitresults[k]['success'] for k in sortedkeys])
    # make the plot
    ax.errorbar(xax, values, yerr=uncs, fmt='o', color='royalblue',
                markersize=5, elinewidth=2)
    # add unit line
    ax.axhline(1., color='red', linestyle='dashed')
    ax.grid(which='major')
    # set x and y ranges
    ax.set_xlim(-0.5, xax[-1]+0.5)
    (ylow,yhigh) = ax.get_ylim()
    if yhigh<1.05: ax.set_ylim(ylow, 1.05)
    # set y-axis title and ticks
    ax.set_ylabel('Fit result', fontsize=15)
    ax.tick_params(axis='y', labelsize=12)
    # draw labels
    ax.set_xticks(xax)
    ax.set_xticklabels(sortedkeys, rotation=60, ha='right')
    # draw title
    if title is not None: ax.text(0., 1.05, title, fontsize=15, transform=ax.transAxes)
    # write chi2 values
    for x, normchi2 in enumerate(normchi2s):
      text = '$\chi^{2}_{norm} = $'+'{:.2e}'.format(normchi2)
      ax.text(x-0.4, 1.01, text, fontsize=7)
    # set margins
    plt.subplots_adjust(bottom=0.35)
    # return the figure
    return (fig,ax)

# global properties
systunc = 0.2
# (relative systematic uncertainty to add)

# read input files from the command line or else read all input files in the current directory
inputfiles = []
if len(sys.argv)>1:
  inputfiles = sys.argv[1:]
else: 
  inputfiles = ([f for f in os.listdir(os.getcwd()) 
         if ('closurePlots_data' in f and f[-5:]=='.root') ])
inputfiles = [os.path.abspath(f) for f in inputfiles]

# loop over input files
for f in inputfiles:
  print('now running on {}...'.format(f))
  fdir,fbase = os.path.split(f)

  # set output directory
  outdir = f.replace('.root','')
  if not os.path.exists(outdir): os.makedirs(outdir)

  # determine instance properties from filename
  # (the filename is assumed to be of the following form: 
  # closurePlots_MC_<process>_<year>.root)
  instancename = fbase.replace('closurePlots_data_','').replace('.root','')
  instanceparts = instancename.split('_')
  year = instanceparts[0]
  print('this file is found to have the following properties:')
  print('  - year: {}'.format(year))

  # determine luminosity value to display
  lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
             '2016PreVFP':19520, '2016PostVFP':16810 }
  lumi = lumimap[year]

  # load all histograms
  histlist = ht.loadallhistograms(f)

  # get a list of variables
  datahists = ht.selecthistograms(histlist, mustcontainall=['_Data'])[1]
  names = [h.GetName() for h in datahists]
  variables = []
  for name in names:
    var = name.split(instancename)[0].strip('_')
    if var not in variables: variables.append(var)
  print('found following variables: '+str(variables))

  # initialize fit results
  fitresults = {}

  # loop over variables
  for var in variables:
    print('running on variable {}'.format(var))

    # get the histograms
    datahists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'Data'])[1]
    prompthists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'PromptBkg'])[1]
    nonprompthists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'NonpromptBkg'])[1]
    cfhists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'ChargeFlips'])[1]
    promptcfhists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'PromptCF'])[1]
    nonpromptcfhists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'NonpromptCF'])[1]
    print('found {} histograms for data'.format(len(datahists)))
    print('found {} histograms for prompt background'.format(len(prompthists)))
    print('found {} histograms for nonprompt background'.format(len(nonprompthists)))
    print('found {} histograms for chargeflips'.format(len(cfhists)))
    print('found {} histograms for simulated chargeflips (prompt)'.format(len(promptcfhists)))
    print('found {} histograms for simulated chargeflips (nonprompt)'.format(len(nonpromptcfhists)))
    datahist = datahists[0].Clone()
    for h in datahists[1:]: datahist.Add(h)
    promptsum = datahist.Clone()
    promptsum.Reset()
    if len(prompthists)>0:
      promptsum = prompthists[0].Clone()
      for h in prompthists[1:]: promptsum.Add(h)
    nonpromptsum = datahist.Clone()
    nonpromptsum.Reset()
    if len(nonprompthists)>0:
      nonpromptsum = nonprompthists[0].Clone()
      for h in nonprompthists[1:]: nonpromptsum.Add(h)
    simsum = promptsum.Clone()
    simsum.Add(nonpromptsum)
    cfsum = cfhists[0].Clone()
    for h in cfhists[1:]: cfsum.Add(h)
    simcfhists = promptcfhists + nonpromptcfhists
    simcfsum = datahist.Clone()
    simcfsum.Reset()
    if len(simcfhists)>0:
      simcfsum = simcfhists[0].Clone()
      for h in simcfhists[1:]: simcfsum.Add(h)

    # do the fit
    temphist = datahist.Clone()
    temphist.Add( simsum, -1 )
    temphist.Divide( cfsum )
    fitresult = fitConstant(temphist)
    fitresults[var] = fitresult

    # add the systematic uncertainty
    cfsyst = cfsum.Clone()
    for i in range(0,cfsum.GetNbinsX()+2):
      cfsyst.SetBinContent(i,cfsum.GetBinContent(i)*systunc)

    # axis titles: get from histograms
    xaxtitle = datahist.GetXaxis().GetTitle()
    yaxtitle = datahist.GetYaxis().GetTitle()

    # set histogram titles
    promptsum.SetTitle('prompt')
    nonpromptsum.SetTitle('nonprompt')
    simsum.SetTitle('simsum')
    cfsum.SetTitle('chargeflips')

    # set other plot options
    mchists = [cfsum, simsum]
    colormap = ({'prompt': ROOT.kMagenta-7,
		 'nonprompt': ROOT.kMagenta+1,
                 'simsum': ROOT.kMagenta+1,
                 'chargeflips': ROOT.kAzure-4})
    labelmap = ({'prompt':'Prompt',
                 'nonprompt':'Nonprompt',
                 'simsum': 'Genuine SS',
		 'chargeflips':'Charge flips'})
    legendbox = [0.5,0.65,0.92,0.9]
    extracmstext = 'Preliminary'
    extrainfos = ['Year: {}'.format(year)]

    # make plot before scaling
    figname = os.path.join(outdir,var+'_'+instancename+'_beforescaling')
    extraextrainfos = ['Before scaling']
    hp.plotdatavsmc( figname, 
      datahist, mchists, 
      mcsysthist=cfsyst,
      colormap=colormap, labelmap=labelmap,
      xaxtitle=xaxtitle, yaxtitle=yaxtitle, lumi=lumi,
      p1legendncols=1, p1legendbox=legendbox,
      extracmstext=extracmstext,
      extrainfos=extrainfos+extraextrainfos)

    # make plot after scaling
    sf = fitresult['value']
    cfsum.Scale( sf )
    cfsyst.Scale( sf )
    figname = os.path.join(outdir,var+'_'+instancename+'_afterscaling')
    extraextrainfos = (['After scaling','(with factor {:.2f})'.format(sf)])
    hp.plotdatavsmc( figname, 
      datahist, mchists, 
      mcsysthist=cfsyst,
      colormap=colormap, labelmap=labelmap,
      xaxtitle=xaxtitle, yaxtitle=yaxtitle, lumi=lumi,
      p1legendncols=1, p1legendbox=legendbox,
      extracmstext=extracmstext,
      extrainfos=extrainfos+extraextrainfos)

  # make a summary figure of fit results
  title = 'Charge flip fit summary for {}'.format(year)
  fig,ax = plot_fitresult_summary( fitresults, title=title )
  figname = os.path.join(outdir, 'fitresult_summary.png')
  fig.savefig(figname)
