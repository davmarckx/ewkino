############################################
# Plot the electron reco scale factor maps #
############################################

import sys
import os
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
from hist2dplotter import plot2dhistogram

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  sfdir = '../../weightFilesUL/electronRecoSF'
  sffilebase = 'electronRECO_SF_{}_{}.root'
  pts = ['ptAbove20','ptBelow20']

  for year in years:
    for pt in pts:
      # find the file
      sffile = sffilebase.format(year,pt)
      sfpath = os.path.join(sfdir,sffile)
      if not os.path.exists(sfpath):
        print('ERROR: scale factor file {}'.format(sfpath)
              +' does not exist, skipping.')
        continue
      # get the histogram
      sfhist = ht.loadhistograms(sfpath, mustcontainall=['EGamma_SF2D'])
      if len(sfhist)!=1:
        print('ERROR: found {} histograms'.format(len(sfhist))
              +' while 1 was expected, skipping.')
        continue
      sfhist = sfhist[0]
      # make a plot
      figname = sffile.replace('.root','.png')
      title = 'Electron Reco Scale Factor for {}, {}'.format(year,pt)
      plot2dhistogram( sfhist, figname,
                       histtitle=title )
