#############################################
# plot the result of pileup reweighter test #
#############################################

import sys
import os
import ROOT
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
import multihistplotter as mhp

if __name__=='__main__':

  years = ['2016PreVFP']
  inputfiles = ['output_ttw_{}.root'.format(year) for year in years]
  tags = ({ 'electronRecoWeight': 'Electron RECO',
            'electronRecoPtLWeight': 'Electron RECO (high pT)',
	    'electronRecoPtSWeight': 'Electron RECO (low pT)',
	    'electronIDWeight': 'Electron ID',
            'muonRecoWeight': 'Muon RECO',
	    'muonIDWeight': 'Muon ID',
	    'pileupWeight': 'Pileup',
	    'prefireWeight': 'Prefire' })
  colorlist = ([ROOT.kAzure-4, # nominal
                ROOT.kViolet, ROOT.kMagenta-9, # first variation
                ROOT.kRed+2, ROOT.kRed-7]) # second variation

  # loop over input files
  for inputfile in inputfiles:
    print('Running on file {}...'.format(inputfile))
    # read all histograms
    histlist = ht.loadallhistograms(inputfile)
    print('Found {} histograms.'.format(len(histlist)))

    # part 1: make plot of total weight
    print('Making plot of total weight...')
    thishistlist = ht.selecthistograms(histlist,mustcontainall=['totWeight'])[1]
    print('Found {} histograms for this plot.'.format(len(thishistlist)))
    # get the nominal histogram
    nomtag = 'totWeightNom'
    nomhist = ht.findhistogram(thishistlist, nomtag)
    if nomhist is None:
      msg = 'ERROR: nominal histogram with name {} not found.'.format(nomtag)
      raise Exception(msg)
    # get all other histograms
    syshists = ht.selecthistograms(thishistlist, maynotcontainone=[nomtag])[1]
    # define colors
    colorlist = [ROOT.kAzure+6]*len(syshists)+[ROOT.kAzure-1]
    # make a plot
    figname = inputfile.replace('.root','.png')
    title = 'Total weight with variations'
    mhp.plotmultihistograms(syshists+[nomhist],
        figname=figname,
        xaxtitle='Relative event weight',
        yaxtitle='Number of events',
        colorlist=colorlist,
        dolegend=False,
        title=title,
        drawoptions='hist',
        extracmstext='#splitline{Preliminary}{Simulation}')

    # part 2: make plots of separate weights
    # define colors
    colorlist = ([ROOT.kAzure-4, # nominal
                ROOT.kViolet, ROOT.kMagenta-9, # first variation
                ROOT.kRed+2, ROOT.kRed-7]) # second variation
    # collect the data
    for tagkey,tagval in sorted(tags.items()):
      print('Finding histograms for tag {}'.format(tagkey))
      # find histograms for this tag
      thishistlist = ht.selecthistograms(histlist,mustcontainall=[tagkey])[1]
      print('Found {} histograms'.format(len(thishistlist)))
      # find nominal histogram
      nomhist = ht.selecthistograms(thishistlist,mustcontainall=['Nom'])[1]
      if len(nomhist)!=1:
        msg = 'ERROR: found {} nominal histograms'.format(len(nomhist))
        msg += ' while expecting 1 (for tag {})'.format(tagkey)
        raise Exception(msg)
      nomhist = nomhist[0]
      # find all systematics histograms
      syshists = ht.selecthistograms(thishistlist,maynotcontainone=['Nom'])[1]
      # add to the data structure
      hists = [nomhist]+syshists
      # make the labels
      labels = [hist.GetTitle()+" (mean: {:.3f})".format(hist.GetMean()) for hist in hists]
      # make a plot
      figname = inputfile.replace('.root','_{}.png'.format(tagkey))
      title = tagval
      mhp.plotmultihistograms(hists,
        figname=figname,
        xaxtitle='Relative event weight',
        yaxtitle='Number of events',
        labellist=labels,
        colorlist=colorlist[:len(hists)],
        title=title,
        drawoptions='hist',
        extracmstext='#splitline{Preliminary}{Simulation}')
