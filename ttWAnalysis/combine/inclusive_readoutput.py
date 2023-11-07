########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import fnmatch
import argparse
import outputparsetools as opt

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True)
  parser.add_argument('--method', default='multidimfit',
    choices=['multidimfit','fitdiagnostics'])
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--useelementary', default=False, action='store_true')
  parser.add_argument('--usecombined', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  data = "exp"
  if args.usedata:
    data = "obs"
  # argument checking
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.datacarddir))
  method = args.method
  if method == 'multidimfit': method = 'MultiDimFit'
  if method == 'fitdiagnostics': method = 'FitDiagnostics'

  # find all workspaces in the directory
  wspaces = []
  matchtags = []
  if args.useelementary: matchtags.append('datacard_*.root')
  if args.usecombined: matchtags.append('dc_combined_*.root')
  for f in os.listdir(args.datacarddir):
    match = False
    for matchtag in matchtags:
      if fnmatch.fnmatch(f,matchtag): match = True
    if not match: continue
    if 'likelihoodscan' in f: continue
    wspaces.append(f)
  if len(wspaces)==0:
    raise Exception('ERROR: no combine workspaces found, terminating.')

  # sort the filenames
  wspaces.sort()

  # fill values
  info = {}
  chnames = []
  for wspace in wspaces:
    # format the datacard name
    card = wspace.replace('.root','.txt')
    # format the channel name
    chname = wspace.replace('.root','')
    chname = chname.replace('datacard_','')
    chname = chname.replace('dc_combined_','')
    info[chname] = {}
    chnames.append(chname)
    # read stat-only result
    try:
      (r,down,up) = opt.read_signalstrength(args.datacarddir, card,
                     statonly=True, usedata=args.usedata, method=method)
    except:
      print('WARNING: could not read {}'.format(os.path.join(args.datacarddir, card)))
      up = 0
      down = 0
    info[chname]['uperror_stat'] = up
    info[chname]['downerror_stat'] = down
    # read total result
    try:
      (r,down,up) = opt.read_signalstrength(args.datacarddir, card,
                     statonly=False, usedata=args.usedata, method=method)
    except:
      print('WARNING: could not read {}'.format(os.path.join(args.datacarddir, card)))
      r = 1
      up = 0
      down = 0
    info[chname]['uperror_tot'] = up
    info[chname]['downerror_tot'] = down
    info[chname]['r'] = r

  # make file content
  lines = []
  for chname in chnames:
    # retrieve values
    r = info[chname]['r']
    uperror_stat = info[chname]['uperror_stat']
    downerror_stat = info[chname]['downerror_stat']
    uperror_tot = info[chname]['uperror_tot']
    downerror_tot = info[chname]['downerror_tot']
    # make a label
    label = chname
    label = label.replace('signalregion', 'SR')
    label = label.replace('controlregion', 'CR')
    label = label.replace('dilepton', '2L')
    label = label.replace('trilepton', '3L')
    line = '{} {}'.format(chname,label)
    line += ' {}'.format(r)
    line += ' -{} +{}'.format(downerror_stat, uperror_stat)
    line += ' -{} +{}'.format(downerror_tot, uperror_tot)
    lines.append(line)

  # write file
  with open(args.outputfile, 'w') as f:
    for line in lines:
      f.write(line+'\n')
