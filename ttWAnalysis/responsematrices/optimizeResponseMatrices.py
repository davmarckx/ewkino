#############################################
# Optimize the binning of a response matrix #
#############################################
# Prerequisite: run fillResponseMatrices.py with a fine binning (e.g. 1000 bins)

import sys
import os
import json
import argparse
import numpy as np
import ROOT
from array import array
sys.path.append('../../Tools/python')
import histtools as ht
import argparsetools as apt
sys.path.append('tools')
import responsematrixtools as rmt

def get_rebinned_hist(hist, bins):
    ### get a rebinned version of a response histogram
    # input arguments:
    # - hist: TH2D with fine binning
    # - bins: list with new bin edges
    newhist = ROOT.TH2D("","",
              len(bins)-1,array('d',bins),
              len(bins)-1,array('d',bins))
    for i in range(0,hist.GetNbinsX()+2):
        for j in range(0,hist.GetNbinsY()+2):
            xval = hist.GetXaxis().GetBinCenter(i)
            yval = hist.GetYaxis().GetBinCenter(j)
            content = hist.GetBinContent(i,j)
            newhist.Fill(xval, yval, content)
    #print('--- get_rebinned_hist ---')
    #print(bins)
    #ht.print2dhistogram(newhist)
    return newhist

def get_eval_params(hist):
    ### get some evaluation parameters
    # minimum stability
    stability = rmt.get_stability(hist, include_outerflow=False)
    minstability = 1
    for i in range(1,stability.GetNbinsX()+1):
        if stability.GetBinContent(i)<minstability:
            minstability = stability.GetBinContent(i)
    # minimum purity
    purity = rmt.get_purity(hist, include_outerflow=False)
    minpurity = 1
    for i in range(1,purity.GetNbinsX()+1):
        if purity.GetBinContent(i)<minpurity:
            minpurity = purity.GetBinContent(i)
    # uniformness of diagonal yields
    diag = []
    for i in range(1,hist.GetNbinsX()+1):
	diag.append(hist.GetBinContent(i,i))
    diagratio = min(diag)/max(diag)
    return (minstability, minpurity, diagratio)

def get_eval_metric(hist):
    ### get a single number to evaluate the current histogram
    params = get_eval_params(hist)
    return (params[0]+params[1]+params[2])/3.

def get_eval_all(hist):
    res = {}
    res['metric'] = get_eval_metric(hist)
    stability = rmt.get_stability(hist)
    res['stability'] = [stability.GetBinContent(i) for i in range(1,stability.GetNbinsX()+1)]
    purity = rmt.get_purity(hist)
    res['purity'] = [purity.GetBinContent(i) for i in range(1,purity.GetNbinsX()+1)]
    res['diagonal'] = [hist.GetBinContent(i,i) for i in range(1,hist.GetNbinsX()+1)]
    return res

def get_eval_str(hist):
    heval = get_eval_all(hist)
    res = 'Stability: '
    res += '{}\n'.format(heval['stability'])
    res += 'Purity: '
    res += '{}\n'.format(heval['purity'])
    res += 'Diagonal yields: '
    res += '{}'.format(heval['diagonal'])
    return res

def get_gradient(hist, bins, rate=0.05):
    ### get gradient
    # input arguments:
    # - hist: TH2D with fine binning
    # - bins: list with new bin edges

    # initialize gradient
    gradient = [0]*len(bins)
    # calculate eval metric
    temp = get_rebinned_hist(hist,bins)
    metric = get_eval_metric(temp)
    # loop over internal bins
    for i in range(1,len(bins)-1):
        # vary bin down
        bins_down = bins[:]
        diff_down = (bins[i]-bins[i-1])*rate
        bins_down[i] = bins[i]-diff_down
	temp = get_rebinned_hist(hist,bins_down)
        metric_down = get_eval_metric(temp)
        # vary bin up
        bins_up = bins[:]
        diff_up = (bins[i+1]-bins[i])*rate
        bins_up[i] = bins[i]+diff_up
	temp = get_rebinned_hist(hist,bins_up)
        metric_up = get_eval_metric(temp)
        # fill gradient
        if( metric > metric_down and metric > metric_up ): continue
        if( metric_up > metric and metric_up > metric_down): gradient[i] = diff_up
	if( metric_down > metric and metric_down > metric_up): gradient[i] = -diff_down
    return gradient


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser('Optimize response matrix binning')
    parser.add_argument('--inputfile', required=True, type=os.path.abspath)
    parser.add_argument('--histname', required=True)
    parser.add_argument('--outputfile', default=None, type=apt.path_or_none)
    parser.add_argument('--initbins', default=None)
    parser.add_argument('--maxiter', default=10, type=int)
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
      print('  - {}: {}'.format(arg,getattr(args,arg)))

    # argument checks and parsing
    if not os.path.exists(args.inputfile):
      raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))
  
    # get input histogram
    f = ROOT.TFile.Open(args.inputfile,'read')
    hist = f.Get(args.histname)
    hist.SetDirectory(0)
    f.Close()

    # set initial bins
    if args.initbins is not None:
        initbins = [float(el) for el in args.initbins.split(',')]
    else:
	nbins = hist.GetXaxis().GetNbins()
	xmin = hist.GetXaxis().GetBinLowEdge(1)
        xmax = hist.GetXaxis().GetBinLowEdge(nbins)+hist.GetXaxis().GetBinWidth(nbins)
        initbins = list(np.linspace(xmin, xmax, num=4))

    # initialize info logger
    info = {'histname': args.histname}

    # determine initial metrics
    ihist = get_rebinned_hist(hist,initbins)
    imetric = get_eval_metric(ihist)
    info['initial'] = get_eval_all(ihist)
    info['initialbins'] = initbins

    # start iteration
    bins = initbins[:]
    for i in range(args.maxiter):
	rate = max(0.1/(i+1), 0.01)
        gradient = get_gradient(hist, bins, rate=rate)
        absgradient = [abs(el) for el in gradient]
	for j in range(1,len(bins)-1): bins[j] += gradient[j]
	# check for stopping conditions
	if( max(absgradient)==0 ):
	    msg = 'INFO: gradient is zero, stopping iteration.'
	    print(msg)
	    break

    # determine final metrics
    fhist = get_rebinned_hist(hist,bins)
    fmetric = get_eval_metric(fhist)
    info['final'] = get_eval_all(fhist)
    info['finalbins'] = bins

    # print result
    print('Results of optimization loop:')
    print('=== Histogram: {}'.format(args.histname))
    print('--- Initial bins: {} -> metric {}'.format(initbins,imetric))
    print(get_eval_str(ihist))
    print('--- Final bins: {} -> metric {}'.format(bins,fmetric))
    print(get_eval_str(fhist))

    # write results to json file
    with open(args.outputfile,'w') as f:
	json.dump(info,f,indent=4)
