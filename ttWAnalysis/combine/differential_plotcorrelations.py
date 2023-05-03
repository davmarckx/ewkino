########################################################
# Gather and parse the results in a datacard directory #
########################################################
# Specifically: plot correlation and covariance matrices
# between the signal strengths in a differential measurement

import sys
import os
import json
import copy
import numpy as np
import matplotlib.pyplot as plt
import argparse
import outputparsetools as opt
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables

def make_cov_from_corr(corr, uncs):
  ### make a covariance matrix from a correlation matrix
  # input arguments:
  # - corr: correlation matrix in the form of a 2D dictionary
  # - uncs: absolute uncertainties in the form of a 1D dictionary
  cov = copy.deepcopy(corr)
  for key1 in uncs.keys():
    for key2 in uncs.keys():
      cov[key1][key2] = corr[key1][key2]*uncs[key1]*uncs[key2]
  return cov

def make_updown_cov_from_corr(corr, ss):
  ### make a covariance matrix from a correlation matrix
  # input arguments:
  # - corr: correlation matrix in the form of a 2D dictionary
  # - ss: signal strengths in the form of a 1D dictionary
  #   (each value in the dict is supposed to be (central,downerror,uperror))
  downuncs = {}
  for key in ss.keys(): downuncs[key] = ss[key][1]
  covdown = make_cov_from_corr(corr, downuncs)
  upuncs = {}
  for key in ss.keys(): upuncs[key] = ss[key][2]
  covup = make_cov_from_corr(corr, upuncs)
  return (covdown, covup)

def dict_to_array(d):
  ### convert a 2D dict to an array
  keys = sorted(d.keys())
  nkeys = len(keys)
  a = np.zeros((nkeys,nkeys))
  for i,key1 in enumerate(keys):
    for j,key2 in enumerate(keys):
      a[i,j] = d[key1][key2]
  return a


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--method', default='multidimfit', choices=['multidimfit'])
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--usecr', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.datacarddir))
  method = args.method
  if method == 'multidimfit': method = 'MultiDimFit'

  # create output directory
  if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)

  # loop over variables
  variables = read_variables(args.variables)
  varnames = [var.name for var in variables]
  for varname in varnames:

    # find relevant workspace in the directory
    starttag = 'datacard'
    if args.usecr: starttag = 'dc_combined'
    wspaces = ([ f for f in os.listdir(args.datacarddir)
                 if (f.startswith(starttag) and varname in f and f.endswith('.root')) ])
    if len(wspaces)!=1:
      print('ERROR: wrong number of workspaces, skipping variable {}.'.format(varname))
      continue
    wspace = wspaces[0]
    card = wspace.replace('.root','.txt')
    # first read results from txt file to get the pois
    dummy = opt.read_multidimfit(args.datacarddir, card,
                   statonly=False, mode='txt',
                   usedata=args.usedata, pois='auto')
    pois = sorted(dummy.keys())
    npois = len(pois)
    # read stat-only correlation
    statresults = opt.read_multidimfit(args.datacarddir, card,
                   statonly=True, correlations=True, mode='root',
                   usedata=args.usedata, pois=pois)
    statss = statresults[0]
    statcorr = statresults[1]
    # read total result
    totresults = opt.read_multidimfit(args.datacarddir, card,
                   statonly=False, correlations=True, mode='root',
                   usedata=args.usedata, pois=pois)
    totss = totresults[0]
    totcorr = totresults[1]
    # check for errors
    if( len(statss.keys())==0 or len(totss.keys())==0 ):
      msg = 'WARNING: some values appear to be missing for workspace {};'.format(wspace)
      msg += ' please check combine output files for unexpected format or failed fits.'
      print(msg)

    # make covariance matrices
    (statcovdown, statcovup) = make_updown_cov_from_corr(statcorr, statss)
    (totcovdown, totcovup) = make_updown_cov_from_corr(totcorr, totss)
    syscovdown = copy.deepcopy(totcovdown)
    syscovup = copy.deepcopy(totcovup)
    for key1 in pois:
      for key2 in pois:
        syscovdown[key1][key2] = totcovdown[key1][key2] - statcovdown[key1][key2]
        syscovup[key1][key2] = totcovup[key1][key2] - statcovup[key1][key2]

    # printouts to check the calculation
    doprint = False
    if doprint:
      print('--------------------')
      print('Correlation matrix of stat-only measurement:')
      print(dict_to_array(statcorr))
      print('Signal strengths and uncertainties for stat-only measurement:')
      print(statss)
      print('Resulting down-covariance matrix:')
      print(dict_to_array(statcovdown))
      print('Resulting up-covariance matrix:')
      print(dict_to_array(statcovup))
      print('--------------------')
      print('Correlation matrix of total measurement:')
      print(dict_to_array(totcorr))
      print('Signal strengths and uncertainties for total measurement:')
      print(totss)
      print('Resulting down-covariance matrix:')
      print(dict_to_array(totcovdown))
      print('Resulting up-covariance matrix:')
      print(dict_to_array(totcovup))

    # define a multiplication factor for covariance matrices
    # (for better readability)
    covmult = 100
    covmulttag = ''
    if covmult!=1: covmulttag = '(x{})'.format(covmult)

    # make plots
    fig,axs = plt.subplots(nrows=2, ncols=3, figsize=(18,12))
    axlist = [axs[0,0], axs[1,0], axs[0,1], axs[1,1], axs[0,2], axs[1,2]]
    gridlist = [dict_to_array(statcorr), dict_to_array(totcorr),
                covmult*dict_to_array(statcovup), covmult*dict_to_array(statcovdown),
                covmult*dict_to_array(syscovup), covmult*dict_to_array(syscovdown)]
    for ax, grid in zip(axlist, gridlist):
      # basic plot
      ax.imshow(grid, cmap='cool')
      # write values
      for (j,i),label in np.ndenumerate(grid):
        ax.text(i,j,'{:.2f}'.format(label), ha='center', va='center', fontsize=15)
      # axes
      ax.set_xticks(list(np.arange(npois)))
      ax.set_xticklabels(labels=pois, fontsize=15)
      ax.set_yticks(list(np.arange(npois)))
      ax.set_yticklabels(labels=pois, fontsize=15)
    # add labels
    axs[0,0].text(-0.5, -0.6, 'Correlation, statistical', fontsize=15)
    axs[1,0].text(-0.5, -0.6, 'Correlation, total', fontsize=15)
    axs[0,1].text(-0.5, -0.6, 'Covariance, statistical, up {}'.format(covmulttag), fontsize=15)
    axs[1,1].text(-0.5, -0.6, 'Covariance, statistical, down {}'.format(covmulttag), fontsize=15)
    axs[0,2].text(-0.5, -0.6, 'Covariance, systematic, up {}'.format(covmulttag), fontsize=15)
    axs[1,2].text(-0.5, -0.6, 'Covariance, systematic, down {}'.format(covmulttag), fontsize=15)

    outfilename = varname + '.png'
    outfilename = os.path.join(args.outputdir, outfilename)
    fig.savefig(outfilename)
    print('Figure {} has been created.'.format(outfilename))
    plt.close()
