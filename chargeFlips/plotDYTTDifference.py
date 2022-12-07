########################################################################
# Plot the difference between charge flip maps obtained from DY and TT #
########################################################################

import sys
import os
import ROOT
import numpy as np
import matplotlib.pyplot as plt
sys.path.append('../Tools/python')
import histtools as ht

# command line and fixed arguments (update to argparse later)
inputdir = sys.argv[1]
flavour = 'electron'
years = ['2016PreVFP','2016PostVFP','2017','2018']
processes = ['DY','TT']
namepattern = 'chargeFlipMap_MC_{}_{}_process_{}_binning_TuThong.root'
figname = 'summary_dyvstt_binning_TuThong.png'
figname = os.path.join(inputdir,figname)

if __name__=='__main__':

  # read all required files
  hists = {}
  for process in processes:
    hists[process] = {}
    for year in years:
      fname = namepattern.format(flavour,year,process)
      fname = os.path.join(inputdir,fname)
      if not os.path.exists(fname):
        raise Exception('ERROR: expected file {} does not exist'.format(fname))
      histlist = ht.loadhistograms(fname,
                   mustcontainall=['chargeFlipRate_{}_{}'.format(flavour,year)])
      if len(histlist)!=1:
        raise Exception('ERROR in file {}:'.format(fname)
                +' found {} histograms while 1 was expected.'.format(len(histlist)))
      hist = histlist[0]
      hist = ht.histtoarray2d( hist, keepouterflow=False )
      hists[process][year] = hist

  # get dimension of a single histogram
  nxbins,nybins = hists[processes[0]][years[0]].shape

  # concatenate histograms over years
  for process in processes:
    concathist = tuple([hists[process][year] for year in years])
    concathist = np.concatenate(concathist)
    hists[process] = concathist

  # calculate relative difference
  firsthist = hists[processes[0]]
  secondhist = hists[processes[1]]
  minhist = np.minimum(firsthist,secondhist)
  diffhist = np.divide(firsthist-secondhist, minhist)

  # calculate max difference (absolute value)
  maxdiff = np.max(np.abs(diffhist))

  # initialize figure
  fig,ax = plt.subplots(figsize=(4.5,6))
  
  # make the plot
  im = ax.imshow(diffhist, cmap='bwr', vmin=-maxdiff, vmax=maxdiff)

  # add a colorbar
  cbar = fig.colorbar(im)
  cbar.set_label('Relative difference')

  # hide ticks
  ax.set_xticks([])
  ax.set_xticklabels([])
  ax.set_yticks([])
  ax.set_yticklabels([])

  # set axis titles
  ax.set_xlabel(r'$|\eta|$ bins')
  ax.set_ylabel(r'$p_{T}$ bins')

  # draw horizontal lines between years
  for i,year in enumerate(years):
    ax.axhline(i*nxbins-0.5, color='k', linestyle='dashed')
    ax.text(nybins-1.25, i*nxbins-0.25, year, 
      horizontalalignment='right', verticalalignment='top')

  # add title
  title = 'DY versus TT CF maps'
  ax.text(0., 1.05, title, transform=ax.transAxes)
  
  # set margins
  plt.subplots_adjust(left=-0.4)
   
  # save the figure
  fig.savefig(figname)
