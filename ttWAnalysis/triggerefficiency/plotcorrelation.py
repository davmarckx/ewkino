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

def selection_to_summarylabel(selection):
  if selection=='dilepton': return 'Dilepton selection'
  if selection=='trilepton': return 'Trilepton selection'
  raise Exception('')

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
  parser.add_argument('--do_summary', action='store_true')
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
      n_none = datahist.GetBinContent(1,1)
      n_both = datahist.GetBinContent(2,2)
      n_ref_only = datahist.GetBinContent(2,1)
      n_lep_only = datahist.GetBinContent(1,2)
      n_tot = datahist.Integral()
      n_ref = n_both + n_ref_only
      n_lep = n_both + n_lep_only
      correlation = float(n_ref*n_lep)/(n_both*n_tot)
      summary_info[selection][year][trigger] = correlation
      nsummary += 1
      # set plot properties
      xaxtitle = 'Passes reference triggers'
      yaxtitle = 'Passes lepton triggers'
      lumi = lumidict[year]
      lumistr = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'
      # make plot of correlation
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

  if args.do_summary:
   
    # first approach (with ROOT plotting) 
    '''shist = ROOT.TH1D('shist', 'Correlation summary', nsummary, -0.5, nsummary-0.5)
    bincounter = 1
    for selection in sorted(summary_info.keys()):
      for year in ['2016PreVFP','2016PostVFP','2017','2018']:
        if not year in summary_info[selection].keys(): continue
        for trigger in summary_info[selection][year].keys():
          value = summary_info[selection][year][trigger]
          label = '{} / {} / {}'.format(selection, year, trigger)
          shist.SetBinContent(bincounter, value)
          shist.SetBinError(bincounter, 0)
	  shist.GetXaxis().SetBinLabel(bincounter, label)
          bincounter += 1
    shist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Correlation ratio'
    title = 'Correlation summary'
    drawoptions = ''
    outputfile = os.path.join(args.inputdir,'summary.png')
    plotsinglehistogram( shist, outputfile, title=title, 
	                 xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                         yaxmin=0.8, yaxmax=1.2,
                         label=None, color=None, drawoptions=drawoptions,
                         lumitext='', extralumitext='',
	                 bottommargin=0.65, topmargin=0.07)'''

    # second approach (with matplotlib)
    data = []
    labels = []
    selections = sorted(list(summary_info.keys()))
    nselections = len(selections)
    years = ([el for el in ['2016PreVFP','2016PostVFP','2017','2018'] 
              if el in summary_info[selections[0]].keys()])
    nyears = len(years)
    triggers = ([el for el in ['singlelepton','dilepton','trilepton','anylepton']
                 if el in summary_info[selections[0]][years[0]].keys()])
    ntriggers = len(triggers)
    for selection in selections:
      for year in years:
        for trigger in triggers:
          value = summary_info[selection][year][trigger]
          label = trigger_to_summarylabel(trigger)
          data.append(value)
          labels.append(label)
    fig,ax = plt.subplots()
    # make modified data and xax for ax.step to show last data point
    datamod = data[:]
    datamod.append(data[-1])
    datamod = np.array(datamod)
    xax = np.arange(0, len(data))
    xaxmod = np.arange(0, len(datamod))
    ax.step(xaxmod, datamod, where='post', color='royalblue', linewidth=2)
    ax.fill_between(xaxmod, datamod, y2=1., step='post', color='lightsteelblue')
    ax.grid(which='major')
    # set x and y ranges
    ax.set_xlim(0, len(data))
    ax.set_ylim(0.8, 1.2)
    # set y-axis title and ticks
    ax.set_ylabel('Correlation ratio', fontsize=15)
    ax.tick_params(axis='y', labelsize=12)
    # draw labels
    ax.set_xticks([x+0.5 for x in xax])
    ax.set_xticklabels(labels, rotation=60, ha='right')
    # draw horizontal lines between years
    stepsize = ntriggers
    for pos in np.arange(stepsize,len(data),step=stepsize):
      ax.axvline(pos, color='gray', linestyle='dashed')
    # draw horizontal lines between selections
    stepsize = ntriggers*nyears
    for pos in np.arange(stepsize,len(data),step=stepsize):
      ax.axvline(pos, color='k', linestyle='dashed', linewidth=2)
    # draw title
    ax.text(0., 1.05, 'Correlation summary', fontsize=15, transform=ax.transAxes)
    # set margins
    plt.subplots_adjust(bottom=0.3)
    # add year labels
    stepsize = ntriggers
    for pos in np.arange(0,len(data),step=stepsize):
      ax.text(pos+float(stepsize)/2, 1.17, years[pos/stepsize%nyears], 
              horizontalalignment='center', fontsize=5)
    # add selection labels
    stepsize = ntriggers*nyears
    for pos in np.arange(0,len(data),step=stepsize):
      text = selections[pos/stepsize%nselections]
      text = selection_to_summarylabel(text)
      ax.text(pos+float(stepsize)/2, 0.65, text, 
              horizontalalignment='center', fontsize=12)
    # save the figure
    outputfile = os.path.join(args.inputdir,'summary.png')
    fig.savefig(outputfile)
