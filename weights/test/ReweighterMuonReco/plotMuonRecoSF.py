########################################
# Plot the muon reco scale factor maps #
########################################
# note: for now, plot only the nominal map,
#       which has the combined syst + stat uncertainy
#       (added in quadrature) as bin errors.
#       maybe extend to plotting them separately later.

import sys
import os
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
from hist2dplotter import plot2dhistogram

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  sfdir = '../../weightFilesUL/muonRecoSF'
  sffilebase = 'muonRECO_SF_{}.root'

  for year in years:
    # find the file
    sffile = sffilebase.format(year)
    sfpath = os.path.join(sfdir,sffile)
    if not os.path.exists(sfpath):
      print('ERROR: scale factor file {}'.format(sfpath)
            +' does not exist, skipping.')
      continue
    # get the histogram
    sfhist = ht.loadhistograms(sfpath, mustcontainall=['nominal'])
    if len(sfhist)!=1:
      print('ERROR: found {} histograms'.format(len(sfhist))
            +' while 1 was expected, skipping.')
      continue
    sfhist = sfhist[0]
    # make a plot
    figname = sffile.replace('.root','.png')
    title = 'Muon Reco Scale Factors for {}'.format(year)
    plot2dhistogram( sfhist, figname,
                     histtitle=title,
                     xtitle='#eta', ytitle='p_{T}' )
