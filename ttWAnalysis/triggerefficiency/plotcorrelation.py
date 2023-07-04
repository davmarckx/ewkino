##########################################################
# plot the output of the trigger correlation measurement #
##########################################################

import os
import sys
import matplotlib.pyplot as plt
import numpy as np
import ROOT
import argparse
sys.path.append(os.path.abspath('../../constants'))
from luminosities import lumidict
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
sys.path.append(os.path.abspath('../../plotting/python'))
from hist2dplotter import plot2dhistogram
from singlehistplotter import plotsinglehistogram

def trigger_to_label(trigger):
  if trigger=='singlelepton': return 'Single lepton triggers'
  if trigger=='dilepton': return 'Dilepton triggers'
  if trigger=='trilepton': return 'Trilepton triggers'
  if trigger=='anylepton': return 'All lepton triggers'
  raise Exception('ERROR: trigger {} not recognized.'.format(trigger))

def trigger_to_summarylabel(trigger):
  if trigger=='singlelepton': return '1L triggers'
  if trigger=='dilepton': return '2L triggers'
  if trigger=='trilepton': return '3L triggers'
  if trigger=='anylepton': return 'All triggers'
  raise Exception('ERROR: trigger {} not recognized.'.format(trigger))

def selection_to_label(selection):
  if selection=='dilepton': return 'Dilepton selection'
  if selection=='trilepton': return 'Trilepton selection'
  raise Exception('ERROR: selection {} not recognized.'.format(selection))

def selection_to_summarylabel(selection):
  if selection=='dilepton': return 'Dilepton selection'
  if selection=='trilepton': return 'Trilepton selection'
  raise Exception('ERROR: selection {} not recognized.'.format(selection))

def year_from_dirname(directory):
  if '2016PreVFP' in directory: return '2016PreVFP'
  elif '2016PostVFP' in directory: return '2016PostVFP'
  elif '2017' in directory: return '2017'
  elif '2018' in directory: return '2018'
  raise Exception('')

def selection_from_dirname(directory):
  if '3tight' in directory: return 'trilepton'
  elif '2tight' in directory: return 'dilepton'
  raise Exception('')

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of trigger correlation measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--do_individual', action='store_true')
  parser.add_argument('--do_summary', action='store_true')
  parser.add_argument('--do_errors', action='store_true')
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
  if len(mergedfiles)==0:
    print('No files found, exiting.')
    sys.exit()

  # loop over all files
  summary_info = {}
  nsummary = 0
  for mergedfilepath in mergedfiles:
    directory = os.path.dirname(mergedfilepath)
    figdir = os.path.join(directory,'plots')
    if not os.path.exists(figdir): os.makedirs(figdir)

    # get info from path name
    year = year_from_dirname(directory)
    selection = selection_from_dirname(directory)
    if selection not in summary_info.keys(): summary_info[selection] = {}
    summary_info[selection][year] = {}

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
      # naive approach (without error calculation)
      n_none = datahist.GetBinContent(1,1)
      n_both = datahist.GetBinContent(2,2)
      n_ref_only = datahist.GetBinContent(2,1)
      n_lep_only = datahist.GetBinContent(1,2)
      n_tot = datahist.Integral()
      n_ref = n_both + n_ref_only
      n_lep = n_both + n_lep_only
      correlation_naive = float(n_ref*n_lep)/(n_both*n_tot)
      cerror_naive = 0
      # alternative approach (with error calculation)
      # note: errors are calculated with simple gaussian propagation;
      # they will be overestimated because of ignoring correlations
      # between factors in the equation.
      n_none = ROOT.TH1D('n_none','n_none', 1, 0., 1.)
      n_none.SetBinContent(1, datahist.GetBinContent(1,1))
      n_none.SetBinError(1, datahist.GetBinError(1,1))
      n_both = ROOT.TH1D('n_both','n_both', 1, 0., 1.)
      n_both.SetBinContent(1, datahist.GetBinContent(2,2))
      n_both.SetBinError(1, datahist.GetBinError(2,2))
      n_ref_only = ROOT.TH1D('n_ref_only','n_ref_only', 1, 0., 1.)
      n_ref_only.SetBinContent(1, datahist.GetBinContent(2,1))
      n_ref_only.SetBinError(1, datahist.GetBinError(2,1))
      n_lep_only = ROOT.TH1D('n_lep_only','n_lep_only', 1, 0., 1.)
      n_lep_only.SetBinContent(1, datahist.GetBinContent(1,2))
      n_lep_only.SetBinError(1, datahist.GetBinError(1,2))
      n_tot = n_none.Clone()
      n_tot.Add(n_both)
      n_tot.Add(n_ref_only)
      n_tot.Add(n_lep_only)
      n_ref = n_ref_only.Clone()
      n_ref.Add(n_both)
      n_lep = n_lep_only.Clone()
      n_lep.Add(n_both)
      num = n_lep.Clone()
      num.Multiply(n_ref)
      denom = n_both.Clone()
      denom.Multiply(n_tot)
      ratio = num.Clone()
      ratio.Divide(denom)
      correlation = ratio.GetBinContent(1)
      cerror = ratio.GetBinError(1)
      # check if both methods agree on the nominal value
      if( abs(correlation_naive-correlation) > 1e-6 ):
        raise Exception('ERROR: alternative correlation calculations do not agree.')
      summary_info[selection][year][trigger] = (correlation,cerror)
      nsummary += 1
      # continue if individual plots are not needed
      if not args.do_individual: continue
      # set plot properties
      xaxtitle = 'Passes reference triggers'
      yaxtitle = 'Passes lepton triggers'
      lumi = lumidict[year]
      lumistr = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'
      caxtitle = 'Number of events (normalized)'
      datahist.GetXaxis().SetBinLabel(1, "No")
      datahist.GetXaxis().SetBinLabel(2, "Yes")
      datahist.GetXaxis().SetLabelSize(0.06)
      datahist.GetYaxis().SetBinLabel(1, "No")
      datahist.GetYaxis().SetBinLabel(2, "Yes")
      datahist.GetYaxis().SetLabelSize(0.06)
      extrainfos = []
      extrainfos.append('{} data'.format(year))
      extrainfos.append(trigger_to_label(trigger))
      extrainfos.append(selection_to_label(selection))
      extrainfos.append('')
      extrainfos.append('Lepton triggers: {0:.3f}'.format(n_lep.GetBinContent(1)))
      extrainfos.append('Reference triggers: {0:.3f}'.format(n_ref.GetBinContent(1)))
      extrainfos.append('Both triggers: {0:.3f}'.format(n_both.GetBinContent(1)))
      extrainfos.append('')
      extrainfos.append('Lepton / total: {0:.3f}'.format(n_lep.GetBinContent(1)/n_tot.GetBinContent(1)))
      extrainfos.append('Both / reference: {0:.3f}'.format(n_both.GetBinContent(1)/n_ref.GetBinContent(1)))
      extrainfos.append('')
      extrainfos.append('Correlation ratio: {0:.3f}'.format(correlation))
      if args.do_errors: extrainfos.append('Estimated error: {0:.3f}'.format(cerror))
      # make plot of correlation
      figname = os.path.join(figdir,'correlation_{}.png'.format(trigger))
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

  if args.do_summary:
  
    # make lists of data, errors and labels 
    data = []
    errors = []
    tlabels = []
    ylabels = []
    slabels = []
    selections = ['dilepton','trilepton']
    nselections = len(selections)
    years = ['2016PreVFP','2016PostVFP','2017','2018'] 
    nyears = len(years)
    triggers = ['singlelepton','dilepton','trilepton','anylepton']
    ntriggers = len(triggers)
    for selection in selections:
      for year in years:
        for trigger in triggers:
          # check if data entry exists
          try: (value,error) = summary_info[selection][year][trigger]
          except: continue
          # skip some combinations
          if(selection=='dilepton' and trigger=='trilepton'): continue
          # add data to lists
          tlabel = trigger_to_summarylabel(trigger)
          slabel = selection_to_summarylabel(selection)
          data.append(value)
          errors.append(error)
          tlabels.append(tlabel)
          ylabels.append(year)
          slabels.append(slabel)
    fig,ax = plt.subplots()
    # make modified data and xax for ax.step to show last data point
    datamod = data[:]
    datamod.append(data[-1])
    datamod = np.array(datamod)
    errorsmod = errors[:]
    errorsmod.append(errors[-1])
    errorsmod = np.array(errorsmod)
    xax = np.arange(0, len(data))
    xaxmod = np.arange(0, len(datamod))
    # make basic plot without error plotting
    if not args.do_errors:
      ax.step(xaxmod, datamod, where='post', color='royalblue', linewidth=2)
      ax.fill_between(xaxmod, datamod, y2=1.,
        step='post', color='lightsteelblue')
    # make basic plot with error plotting
    else:
      ax.axhline(y=1., color='red', linestyle='dashed')
      ax.step(xaxmod, datamod, where='post', color='royalblue', linewidth=2)
      ax.fill_between(xaxmod, datamod+errorsmod, y2=datamod-errorsmod, 
        step='post', color='lightsteelblue')
    ax.grid(which='major')
    # set x and y ranges
    ax.set_xlim(0, len(data))
    ax.set_ylim(0.8, 1.2)
    # set y-axis title and ticks
    ax.set_ylabel('Correlation ratio', fontsize=15)
    ax.tick_params(axis='y', labelsize=12)
    # draw x-axis labels (i.e. trigger labels)
    ax.set_xticks([x+0.5 for x in xax])
    ax.set_xticklabels(tlabels, rotation=60, ha='right')
    # draw horizontal lines between years and add year labels
    start = 0
    label = ylabels[0]
    for i,newlabel in enumerate(ylabels):
      if i==0: continue
      if newlabel!=label:
        ax.axvline(i, color='gray', linestyle='dashed')
        ax.text(start+float(i-start)/2, 1.17, label,
                horizontalalignment='center', fontsize=5)
        start = i
        label = newlabel
    ax.text(start+float(i+1-start)/2, 1.17, label,
            horizontalalignment='center', fontsize=5)
    # draw horizontal lines between selections and add selection labels
    start = 0
    label = slabels[0]
    for i,newlabel in enumerate(slabels):
      if i==0: continue
      if newlabel!=label:
        ax.axvline(i, color='k', linestyle='dashed', linewidth=2)
        ax.text(start+float(i-start)/2, 0.65, label,
                horizontalalignment='center', fontsize=12)
        start = i
        label = newlabel
    ax.text(start+float(i+1-start)/2, 0.65, label,
            horizontalalignment='center', fontsize=12)
    # draw title
    ax.text(0., 1.05, 'Correlation summary', fontsize=15, transform=ax.transAxes)
    # set margins
    plt.subplots_adjust(bottom=0.3)
    # save the figure
    outputfile = os.path.join(args.inputdir,'summary.png')
    fig.savefig(outputfile)
