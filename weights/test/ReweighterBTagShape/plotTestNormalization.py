#########################################
# plot the result of normalization test #
#########################################

import sys
import os
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
import multihistplotter as mhp

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  inputfiles = ['output_ttw_{}_norm.root'.format(year) for year in years]
  tags = ({ 'unweightedpreselection': 'Unweighted pre-selection',
            'unweightedpostselection': 'Unweighted post-selection',
            'weightedprenorm': 'Weighted pre-norm',
            'weightedpostnorm': 'Weighted post-norm',
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
    for tagkey,tagval in tags.items():
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
      # get the total yield
      nomyield = nomhist.Integral()
      # find all systematics histograms and make envelope
      syshists = ht.selecthistograms(thishistlist,maynotcontainone=['nominal'])[1]
      if 'unweighted' in tagkey: unc = None
      else: 
        unc = ht.envelope( syshists, returntype='hist' )
        ht.printhistogram(unc)
      # add to the data structure
      labels.append(tagval+' (yield: {:.0f})'.format(nomyield))
      hists.append(nomhist)
      uncertainties.append(unc)
    # make a plot
    figname = inputfile.replace('.root','.png')
    mhp.plotmultihistograms(hists,
      uncertainties=uncertainties,
      figname=figname,
      xaxtitle='Number of jets',
      yaxtitle='Number of events',
      labellist=labels,
      #colorlist=,
      extracmstext='#splitline{Preliminary}{Simulation}')
