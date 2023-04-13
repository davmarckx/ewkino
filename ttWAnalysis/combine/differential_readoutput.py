########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import json
import argparse
import outputparsetools as opt
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
  method = args.method
  if method == 'multidimfit': method = 'MultiDimFit'

  # initialize result dict
  info = {}

  # find and loop over variables
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
    # read stat-only result
    statdict = opt.read_multidimfit(args.datacarddir, card,
                   statonly=True, usedata=args.usedata, pois='auto')
    # read total result
    totdict = opt.read_multidimfit(args.datacarddir, card,
                   statonly=False, usedata=args.usedata, pois='auto')
    # check for errors
    if( len(statdict.keys())==0 or len(totdict.keys())==0 ):
      msg = 'WARNING: some values appear to be missing for workspace {};'.format(wspace)
      msg += ' please check combine output files for unexpected format or failed fits.'
      print(msg)
    # format
    thisinfo = []
    for key in sorted(totdict.keys()):
      (r,down,up) = totdict[key]
      (_, statdown, statup) = statdict[key]
      thisinfo.append( [r,statdown,statup,down,up] )
    keyname = varname.replace('eventBDT','') # find cleaner way later
    info[keyname] = thisinfo

  # write file
  with open(args.outputfile, 'w') as f:
    json.dump(info, f)
