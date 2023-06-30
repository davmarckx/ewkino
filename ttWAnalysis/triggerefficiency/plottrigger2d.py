#########################################################
# plot the output of the trigger efficiency measurement #
#########################################################

import os
import sys
import ROOT
import argparse
sys.path.append(os.path.abspath('../../constants'))
from luminosities import lumidict
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
sys.path.append(os.path.abspath('../../plotting/python'))
from hist2dplotter import plot2dhistogram

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of trigger efficiency measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))

  # first find files to run on
  mergedfilename = 'combined_trigger_histograms.root'
  mergedfiles = []
  for root,dirs,files in os.walk(args.inputdir):
    for f in files:
      if( f==mergedfilename ):
        mergedfiles.append(os.path.join(root,f))
  print('Found {} files.'.format(len(mergedfiles)))

  # loop over all files
  for mergedfilepath in mergedfiles:
    directory = os.path.dirname(mergedfilepath)
    figdir = os.path.join(directory,'plots')
    if not os.path.exists(figdir): os.makedirs(figdir)
    sfdir = os.path.join(directory,'scalefactors')
    if not os.path.exists(sfdir): os.makedirs(sfdir)

    # get year from path name
    year = 0
    if '2016PreVFP' in directory: year = '2016PreVFP'
    elif '2016PostVFP' in directory: year = '2016PostVFP'
    elif '2017' in directory: year = '2017'
    elif '2018' in directory: year = '2018'

    # make a list and a dict of all histograms in the file
    objlist = ht.loadallhistograms(mergedfilepath)
    objdict = {}
    for obj in objlist: objdict[obj.GetName()] = obj
    sfhists = []

    # loop over flavour combinations
    for flavour in ['mm', 'me', 'em', 'ee']:
      # extract the needed histograms
      simkey = 'mc_'+flavour
      datakey = 'Merged_Run'+year+'_'+flavour
      simtotcounts = objdict[simkey+'_tot']
      simtrigcounts = objdict[simkey+'_trig']
      datatotcounts = objdict[datakey+'_tot']
      datatrigcounts = objdict[datakey+'_trig']
      # make efficiency histograms
      simeff = simtrigcounts.Clone()
      simeff.Divide(simeff,simtotcounts,1,1,'B')
      dataeff = datatrigcounts.Clone()
      dataeff.Divide(dataeff,datatotcounts,1,1,'B')
      # make scale factor histogram
      sf = dataeff.Clone()
      sf.Divide(simeff)
      # prepare scale factor histogram for writing
      sf.SetName('scalefactors_{}_{}'.format(year,flavour))
      sf.SetTitle('Scale factors {} {}'.format(year,flavour))
      sfhists.append( sf )
      # set plot properties
      xaxtitle = 'Leading lepton p_{T} (GeV)'
      yaxtitle = 'Subleading lepton p_{T} (GeV)'
      lumi = lumidict[year]
      lumistr = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'
      # make plot of simulated efficiency
      figname = os.path.join(figdir,flavour+'_simeff.png')
      caxtitle = 'Trigger efficiency'
      extrainfos = []
      extrainfos.append('{} simulation'.format(year))
      extrainfos.append('{} channel'.format(flavour))
      plot2dhistogram(simeff, figname, outfmts=['.png'],
                    histtitle=None, logx=True, logy=True,
                    xtitle=xaxtitle, ytitle=yaxtitle, ztitle=caxtitle,
                    xtitleoffset=1.2, ytitleoffset=1.2, ztitleoffset=1.,
                    axtitlesize=None,
                    titlesize=None,
                    drawoptions='colztexte', cmin=None, cmax=None,
                    docmstext=True, cms_in_grid=True,
                    cmstext_size_factor=1., extracmstext='Preliminary', lumitext=lumistr,
                    topmargin=0.05, rightmargin=0.2,
                    extrainfos=extrainfos, infofont=None, infosize=None, infoleft=None, infotop=None )
      # make plot of data efficiency
      figname = os.path.join(figdir,flavour+'_dataeff.png')
      caxtitle = 'Trigger efficiency'
      extrainfos = []
      extrainfos.append('{} data'.format(year))
      extrainfos.append('{} channel'.format(flavour))
      plot2dhistogram(dataeff, figname, outfmts=['.png'],
                    histtitle=None, logx=True, logy=True,
                    xtitle=xaxtitle, ytitle=yaxtitle, ztitle=caxtitle,
                    xtitleoffset=1.2, ytitleoffset=1.2, ztitleoffset=1.,
                    axtitlesize=None,
                    titlesize=None,
                    drawoptions='colztexte', cmin=None, cmax=None,
                    docmstext=True, cms_in_grid=True,
                    cmstext_size_factor=1., extracmstext='Preliminary', lumitext=lumistr,
                    topmargin=0.05, rightmargin=0.2,
                    extrainfos=extrainfos, infofont=None, infosize=None, infoleft=None, infotop=None )
      # make plot of scale factors
      figname = os.path.join(figdir,flavour+'_sf.png')
      caxtitle = 'Data / Simulation'
      extrainfos = []
      extrainfos.append('{} scale factor'.format(year))
      extrainfos.append('{} channel'.format(flavour))
      plot2dhistogram(sf, figname, outfmts=['.png'],
                    histtitle=None, logx=True, logy=True,
                    xtitle=xaxtitle, ytitle=yaxtitle, ztitle=caxtitle,
                    xtitleoffset=1.2, ytitleoffset=1.2, ztitleoffset=1.,
                    axtitlesize=None,
                    titlesize=None,
                    drawoptions='colztexte', cmin=None, cmax=None,
                    docmstext=True, cms_in_grid=True,
                    cmstext_size_factor=1., extracmstext='Preliminary', lumitext=lumistr,
                    topmargin=0.05, rightmargin=0.2,
                    extrainfos=extrainfos, infofont=None, infosize=None, infoleft=None, infotop=None )

    # write scale factor histograms to file
    outfile = os.path.join(sfdir, 'scalefactors_{}.root'.format(year))
    f = ROOT.TFile.Open( outfile, 'recreate' )
    for hist in sfhists: hist.Write()
    f.Close()
