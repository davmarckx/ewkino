#####################################################################################
# Gather results from differential measurement and sum to get inclusive measurement #
#####################################################################################

import sys
import os
import json
import argparse
import numpy as np
import matplotlib.pyplot as plt
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
from variabletools import read_variables


def makeplot( info, normalize=False ):
    fig, ax = plt.subplots(figsize=(12,6))
    # get the keys
    inclusivekey = 'inclusive'
    doinclusive = False
    varnames = info.keys()
    if inclusivekey in varnames:
        varnames.remove(inclusivekey)
        doinclusive = True
    varnames = sorted(varnames)
    if doinclusive: varnames = [inclusivekey] + varnames
    # initializations
    yaxtitle = 'Signal strength'
    # do normalization
    if normalize:
        if doinclusive:
            # normalization wrt inclusive
            norm = info[inclusivekey][0]
            yaxtitle += ' (norm. wrt {})'.format(inclusivekey)
        else:
            # normalization wrt average
            values = np.array([info[varname][0] for varname in varnames])
            errors = np.array([info[varname][0] for varname in varnames])
            norm = np.average(values, weights=1./errors)
            yaxtitle += ' (norm. wrt average)'
        for varname in varnames:
            (r, error) = info[varname]
            info[varname] = (r/norm, error/norm)
    # make the plot
    for i, varname in enumerate(varnames):
      (r, error) = info[varname]
      color = 'dodgerblue'
      if varname==inclusivekey: color = 'darkviolet'
      ax.plot([i, i], [r-error, r+error], color=color, linewidth=3)
      ax.scatter([i], [r], color=color, s=100)
      ax.text(i, -0.1, varname, ha='right', va='top', rotation=45, rotation_mode='anchor',
              fontsize=12)
    # plot aesthetics
    ax.grid()
    ax.xaxis.set_ticks([i for i in range(0,len(info))])
    ax.xaxis.set_ticklabels([])
    ylims = ax.get_ylim()
    ax.set_ylim(0., 2.)
    ax.set_ylabel(yaxtitle, fontsize=12)
    fig.subplots_adjust(bottom=0.35)
    ax.hlines([1.], -0.5, len(info)-0.5, colors='gray', linestyles='dashed')
    if doinclusive:
      ax.vlines([-0.5, 0.5], ax.get_ylim()[0], ax.get_ylim()[1], colors='gray', linestyle='dashed')
    title = 'Weighted inclusive signal strength from differential measurement'
    ax.text(0.05, 1.03, title, fontsize=15, transform=ax.transAxes)
    return (fig, ax)


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--diffjson', required=True, type=os.path.abspath)
  parser.add_argument('--prediction', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.diffjson):
    raise Exception('ERROR: input json file {} does not exist.'.format(args.diffjson))

  # load signal strength json
  with open(args.diffjson,'r') as f:
    signalstrengths = json.load(f)

  # extract variables
  varnames = signalstrengths.keys()

  # get all relevant histograms from theory input
  mustcontainall = ['nominal', 'TTW2018'] # process name hard-coded for now, maybe extend later
  mustcontainone = varnames
  # do loading and initial selection
  histnames = ht.loadhistnames(args.prediction, mustcontainall=mustcontainall,
                               mustcontainone=mustcontainone)
  for histname in histnames: print('  {}'.format(histname))
  # load the selected histograms
  histlist = ht.loadhistogramlist(args.prediction, histnames)

  # initialize result structure
  res = {}

  # loop over variables
  for varname in varnames:
    print('Running on variable {}'.format(varname))

    # select theory histogram
    hist = ht.selecthistograms(histlist, mustcontainall=['_{}_'.format(varname)])[1]
    if len(hist)==0:
      raise Exception('ERROR: could not find histogram for variable {}'.format(varname))
    if len(hist)>1:
      raise Exception('ERROR: found multiple histograms for variable {}'.format(varname))
    hist = hist[0]

    # divide by integral to get relative contribution from each bin
    hist.Scale(1./hist.Integral())
    weightfactors = []
    for i in range(1, hist.GetNbinsX()+1):
      weightfactors.append(hist.GetBinContent(i))

    # get signal strengths
    ss = signalstrengths[varname]['pois']
    pois = sorted(ss.keys())

    # printouts for testing
    print('Found weight factors: {}'.format(weightfactors))
    print('Found POIs: {}'.format(pois))

    # do some checks
    if( len(pois)!=len(weightfactors) ):
      raise Exception('ERROR: weight factors and pois do not agree.')
    if( abs(sum(weightfactors)-1)>1e-12 ):
      raise Exception('ERROR: sum of weight factors is not 1.')

    # make weighted sum of the signal strengths
    ss_sum = 0.
    for i, poi in enumerate(pois):
      ss_sum += weightfactors[i] * ss[poi][0]

    # make weighted quadratic sum of the errors
    # note: not really correct but good enough for first attempt
    # (errors are statistically dominated anyway)
    ss_error = 0.
    for i, poi in enumerate(pois):
      relative_error = (ss[poi][3]+ss[poi][4])/2. / ss[poi][0]
      ss_error += weightfactors[i]**2 * relative_error**2
    ss_error = ss_sum * ss_error**(0.5)

    # add results
    res[varname] = (ss_sum, ss_error)

  # make a plot
  fig, ax = makeplot(res, normalize=True)
  fig.savefig(args.outputfile)
