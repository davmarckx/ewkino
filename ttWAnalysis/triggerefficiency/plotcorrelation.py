##########################################################
# plot the output of the trigger correlation measurement #
##########################################################

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

def trigger_to_label(trigger):
  if trigger=='singlelepton': return 'Single lepton triggers'
  if trigger=='dilepton': return 'Dilepton triggers'
  if trigger=='trilepton': return 'Trilepton triggers'
  if trigger=='anylepton': return 'All lepton triggers'
  raise Exception('ERROR: trigger {} not recognized.'.format(trigger))

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of trigger correlation measurement')
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

  # loop over all files
  for mergedfilepath in mergedfiles:
    directory = os.path.dirname(mergedfilepath)
    figdir = os.path.join(directory,'plots')
    if not os.path.exists(figdir): os.makedirs(figdir)

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
    # loop over triggers
    triggers = ['singlelepton','dilepton','trilepton','anylepton']
    for trigger in triggers:
      # extract the needed histograms
      datakey = 'Merged_Run'+year+'_'+trigger+'_correlation'
      datahist = objdict[datakey]
      # scale
      datahist.Scale(float(1)/datahist.Integral())
      # get values
      n_none = datahist.GetBinContent(1,1)
      n_both = datahist.GetBinContent(2,2)
      n_ref_only = datahist.GetBinContent(2,1)
      n_lep_only = datahist.GetBinContent(1,2)
      n_tot = datahist.Integral()
      n_ref = n_both + n_ref_only
      n_lep = n_both + n_lep_only
      correlation = float(n_ref*n_lep)/(n_both*n_tot) 
      # set plot properties
      xaxtitle = 'Passes reference triggers'
      yaxtitle = 'Passes lepton triggers'
      lumi = lumidict[year]
      lumistr = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'
      # make plot of simulated efficiency
      figname = os.path.join(figdir,'correlation_{}.png'.format(trigger))
      caxtitle = 'Counts (normalized to maximum)'
      extrainfos = []
      extrainfos.append('{} data'.format(year))
      extrainfos.append(trigger_to_label(trigger))
      extrainfos.append('')
      extrainfos.append('Lepton triggers: {0:.3f}'.format(n_lep))
      extrainfos.append('Reference triggers: {0:.3f}'.format(n_ref))
      extrainfos.append('Both triggers: {0:.3f}'.format(n_both))
      extrainfos.append('')
      extrainfos.append('Lepton / total: {0:.3f}'.format(n_lep/n_tot))
      extrainfos.append('Both / reference: {0:.3f}'.format(n_both/n_ref))
      extrainfos.append('')
      extrainfos.append('Correlation ratio: {0:.3f}'.format(correlation))
      plot2dhistogram(datahist, figname, outfmts=['.png'],
                    histtitle=None, logx=False, logy=False,
                    xtitle=xaxtitle, ytitle=yaxtitle, ztitle=caxtitle,
                    xtitleoffset=1.2, ytitleoffset=1.2, ztitleoffset=1.,
                    axtitlesize=None,
                    titlesize=None,
                    drawoptions='colztexte', cmin=None, cmax=None,
                    docmstext=True, cms_in_grid=True,
                    cmstext_size_factor=0.8, extracmstext='Preliminary', lumitext=lumistr,
                    topmargin=0.05, rightmargin=0.4,
                    extrainfos=extrainfos, infofont=None, infosize=None, 
                    infoleft=0.75, infotop=None )
