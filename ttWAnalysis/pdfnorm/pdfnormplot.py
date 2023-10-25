#############################
# Plot output of pdfnorm.py #
#############################

import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import argparsetools as apt
import histtools as ht
from samplelisttools import readsamplelist


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Run main analysis code')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--samplelist', required=True, type=os.path.abspath)
  parser.add_argument('--extrainfos', default=None, nargs='+')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))
  if not os.path.exists(args.samplelist):
    raise Exception('ERROR: sample list {} does not exist.'.format(args.samplelist))

  # read samples
  samples = readsamplelist( args.samplelist, sampledir=args.inputdir )

  # initialize data structure
  hnames = ['pdf', 'qcdScales']
  data = {}
  for hname in hnames:
    data[hname] = {}

  # loop over samples and histograms
  for sample in samples.samples:
    histlist = ht.loadallhistograms(sample.path)
    for hname in hnames:
      hist = ht.findhistogram(histlist, hname)
      if hist is None: raise Exception('ERROR')
      # read histogram bin contents
      hdata = []
      for i in range(1, hist.GetNbinsX()+1):
        hdata.append(hist.GetBinContent(i))
      sampletag = sample.name.split('_TuneC')[0]
      data[hname][sampletag] = np.array(hdata)

  # format extra infos
  extrainfos = []
  if args.extrainfos is not None:
    for info in args.extrainfos:
      extrainfos.append(info.replace('_',' '))

  # loop over histogram types
  for hname in hnames:
    samples = sorted(data[hname].keys())
    nsamples = len(samples)
    # make a plot
    figheight = 6 + max(nsamples-10, 0)*0.1
    figwidth = 6 + max(nsamples-10, 0)*0.6
    fig, ax = plt.subplots(figsize=(figwidth,figheight))
    for i, sample in enumerate(samples):
      values = data[hname][sample]
      ax.scatter([i]*len(values), values, color='dodgerblue', alpha=0.8,
                 marker='_', s=300)
      ax.text(i, -0.1, sample, ha='right', va='top', rotation=45, rotation_mode='anchor',
              fontsize=12)
    # plot aesthetics
    ax.grid()
    ax.xaxis.set_ticks([i for i in range(0,nsamples)])
    ax.xaxis.set_ticklabels([])
    ylims = ax.get_ylim()
    ax.set_ylim(0., ylims[1])
    fig.subplots_adjust(bottom=0.3)
    ax.hlines([1.], -0.5, nsamples-0.5, colors='gray', linestyles='dashed', alpha=0.5)
    title = ''
    if hname=='pdf': title = 'PDF normalizations'
    if hname=='qcdScales': title = 'QCD scale normalizations'
    ax.text(0.05, 0.95, title, va='top', fontsize=15, transform=ax.transAxes)
    for i, info in enumerate(extrainfos):
      ax.text(0.05, 0.95-(i+1)*0.05, info, va='top', fontsize=15, transform=ax.transAxes)
    # save the plot
    figname = os.path.join(args.inputdir, 'summary_{}.png'.format(hname))
    fig.savefig(figname)
