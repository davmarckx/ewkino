########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import json
import copy
import argparse
import outputparsetools as opt
from differential_plotcorrelations import make_updown_cov_from_corr
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True)
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
  if not args.outputfile.endswith('.json'):
    raise Exception('ERROR: output file must have .json extension')
  method = args.method
  if method == 'multidimfit': method = 'MultiDimFit'

  # initialize result dict
  # structure of the info dict:
  # variable names:
  #   'pois':
  #     POI names:
  #       list of [central, statdown, statup, total down, total up]
  #   'syscovdown': dict of systematic down covariance matrix
  #   'syscovup': dict of systematic up covariance matrix
  #   'statcovdown': dict of statistical down covariance matrix
  #   'statcovup': dict of statistical up covariance matrix
  info = {}

  # find and loop over variables
  variables = read_variables(args.variables)
  varnames = [var.name for var in variables]
  for varname in varnames:
    print('Running on variable {}'.format(varname))

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
    print('  Found pois: {}'.format(pois))
    # read stat-only correlation
    try:
      statresults = opt.read_multidimfit(args.datacarddir, card,
                     statonly=True, correlations=True, mode='root',
                     usedata=args.usedata, pois=pois)
      statss = statresults[0]
      statcorr = statresults[1]
    except:
      msg = 'WARNING: could not read stat-only results for variable {},'.format(varname)
      msg += ' skipping this variable.'
      print(msg)
      continue
    
    # read total result
    try:
      totresults = opt.read_multidimfit(args.datacarddir, card,
                     statonly=False, correlations=True, mode='root',
                     usedata=args.usedata, pois=pois)
      totss = totresults[0]
      totcorr = totresults[1]
    except:
      msg = 'WARNING: could not read total fit results for variable {},'.format(varname)
      msg += ' skipping this variable.'
      print(msg)
      continue

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

    # format
    thisinfo = {}
    thisinfo['pois'] = {}
    for poi in pois:
      (r,down,up) = totss[poi]
      (_, statdown, statup) = statss[poi]
      thisinfo['pois'][poi] = [r,statdown,statup,down,up]
    thisinfo['statcorr'] = statcorr
    thisinfo['totcorr'] = totcorr
    thisinfo['syscovdown'] = syscovdown
    thisinfo['syscovup'] = syscovup
    thisinfo['statcovdown'] = statcovdown
    thisinfo['statcovup'] = statcovup
    keyname = varname.replace('eventBDT','') # find cleaner way later
    info[keyname] = thisinfo

  # write file
  with open(args.outputfile, 'w') as f:
    json.dump(info, f)
