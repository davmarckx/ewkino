####################################################
# plot the result of electron reco reweighter test #
####################################################

import sys
import os
import ROOT
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
import multihistplotter as mhp

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  inputfiles = ['output_ttw_{}.root'.format(year) for year in years]
  tags = ({ 'unweightedpreselection': 'Unweighted pre-selection',
            'unweightedpostselection': 'Unweighted post-selection',
            'weightedpreselection': 'Weighted pre-selection',
            'weightedpostselection': 'Weighted post-selection' })

  # loop over input files
  for inputfile in inputfiles:
    print('Running on file {}...'.format(inputfile))
    # read all histograms
    histlist = ht.loadallhistograms(inputfile)
    print('Found {} histograms.'.format(len(histlist)))
    # collect the data
    hists = []
    uncertainties = []
    labels = []
    colors = []
    for tagkey,tagval in sorted(tags.items()):
      print('Finding histograms for tag {}'.format(tagkey))
      # find histograms for this tag
      veto = []
      if tagkey.startswith('weighted'): veto=['unweighted']
      thishistlist = ht.selecthistograms(histlist,mustcontainall=[tagkey],
                         maynotcontainone=veto)[1]
      print('Found {} histograms'.format(len(thishistlist)))
      # find nominal histogram
      nomhist = ht.selecthistograms(thishistlist,mustcontainall=['nominal'])[1]
      if len(nomhist)!=1:
        msg = 'ERROR: found {} nominal histograms'.format(len(nomhist))
        msg += ' while expecting 1 (for tag {})'.format(tagkey)
        raise Exception(msg)
      nomhist = nomhist[0]
      # find all systematics histograms and make envelope
      syshists = ht.selecthistograms(thishistlist,maynotcontainone=['nominal'])[1]
      if 'unweighted' in tagkey: unc = None
      else: 
        unc = ht.envelope( syshists, returntype='hist' )
      # add to the data structure
      labels.append('{}'.format(tagval))
      hists.append(nomhist)
      uncertainties.append(unc)
    # make a plot
    figname = inputfile.replace('.root','.png')
    mhp.plotmultihistograms(hists,
      uncertainties=uncertainties,
      figname=figname,
      xaxtitle='Number of electrons in event',
      yaxtitle='Number of events',
      labellist=labels,
      #colorlist=colors,
      extracmstext='#splitline{Preliminary}{Simulation}')
