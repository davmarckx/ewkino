############################################
# Plot the electron reco scale factor maps #
############################################

import sys
import os
sys.path.append('../../../Tools/python')
import histtools as ht
sys.path.append('../../../plotting/python')
from hist2dplotter import plot2dhistogram, geterrorhist, getzeroerrorhist

def process_histogram(hist, btype='c', hvar='sf'):
  ### process a scale factor histogram for plotting
  # input arguments:
  # - btype: either 'c' (to plot bin contents)
  #          or 'e' to plot (bin errors).
  #   in both cases, the values to plot will be put as bin contents
  #   and the bin errors will be set to zero.
  # - hvar: either 'sf', 'syst' or 'stat'.
  #   for 'syst' and 'stat', the bin contents are multiplied by 100
  #   (for better readability in plots)
  nxbins = hist.GetNbinsX()
  nybins = hist.GetNbinsY()
  if btype=='e':
    # replace bin contents by bin errors
    hist = geterrorhist( hist )
  # set all errors to zero
  hist = getzeroerrorhist( hist )
  if hvar in ['syst','stat']:
    # scale up bin contents
    for i in range(0,nxbins+2):
      for j in range(0,nybins+2):
        hist.SetBinContent(i,j,100*hist.GetBinContent(i,j))
  return hist

if __name__=='__main__':

  # settings
  years = ['2016PreVFP','2016PostVFP','2017','2018']
  flavours = ['electron','muon']
  wps = ['VLoose','Loose','Medium','Tight']
  sfdir = '../../weightFilesUL/leptonSF'
  sffilebase = 'leptonMVAUL_SF_{}s_{}_{}.root'
  # histogram names: specify the name and the type,
  # where 'c' means that the bin contents should be plotted,
  # and 'e' means that the bin errors should be plotted.
  histnames = ({
    'electron': ({
      'sf': ('EGamma_SF2D','c'),
      'syst': ('sys','c'),
      'stat': ('stat','c')
    }),
    'muon': ({
      'sf': ('NUM_LeptonMva{}_DEN_TrackerMuons_abseta_pt','c'),
      'syst': ('NUM_LeptonMva{}_DEN_TrackerMuons_abseta_pt_combined_syst','c'),
      'stat': ('NUM_LeptonMva{}_DEN_TrackerMuons_abseta_pt_stat','e')
    })
  })

  for year in years:
    for flavour in flavours:
      for wp in wps:
        # find the file
        sffile = sffilebase.format(flavour,wp,year)
        sfpath = os.path.join(sfdir,sffile)
        if not os.path.exists(sfpath):
          print('ERROR: scale factor file {}'.format(sfpath)
                +' does not exist, skipping.')
          continue
        # get all histogram
        hists = ht.loadallhistograms(sfpath)
        # loop over histograms
        for hvar in ['sf','syst','stat']:
	  hname = histnames[flavour][hvar]
	  hist = ht.findhistogram(hists, hname[0].format(wp))
          if hist is None:
            print('ERROR: histogram {} not found in file {}'.format(hname[0],sffile))
          hist = process_histogram(hist, btype=hname[1], hvar=hvar)
          # make a plot
          figname = sffile.replace('.root','_{}.png'.format(hvar))
          title = 'Lepton ID scale factors for {}/{}/{}'.format(year,flavour,wp)
          ztitle = 'Scale factor' 
          if hvar == 'syst': ztitle = 'Systematic uncertainty (x100)'
          elif hvar == 'stat':  ztitle = 'Statistical uncertainty (x100)'
          plot2dhistogram( hist, figname,
                           histtitle=title,
                           drawoptions='colztext',
                           rightmargin=0.25,
                           axtitlesize=25,
                           ztitle=ztitle,
                           ztitleoffset=1.3 )
