########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import json
import matplotlib.pyplot as plt
import argparse
import outputparsetools as opt
sys.path.append(os.path.abspath('../../Tools/python'))
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
            # normalization wrt first differential variable
            norm = info[varnames[0]][0]
            yaxtitle += ' (norm. wrt {})'.format(varnames[0])
        for varname in varnames:
            (r, down, up) = info[varname]
            info[varname] = (r/norm, down/norm, up/norm)
    # make the plot
    for i, varname in enumerate(varnames):
      (r, down, up) = info[varname]
      color = 'dodgerblue'
      if varname==inclusivekey: color = 'darkviolet'
      ax.plot([i, i], [r-down, r+up], color=color, linewidth=3)
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
    title = 'Inclusive signal strength with differential setup'
    ax.text(0.05, 1.03, title, fontsize=15, transform=ax.transAxes)
    return (fig, ax)


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True)
  parser.add_argument('--outputfile', required=True)
  parser.add_argument('--method', default='multidimfit', choices=['multidimfit'])
  parser.add_argument('--includeinclusive', default=False, action='store_true')
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
  if not args.outputfile.endswith('.png'):
    raise Exception('ERROR: output file must have .png extension')
  method = args.method
  if method == 'multidimfit': method = 'MultiDimFit'

  # initialize result dict
  info = {}

  # find and loop over variables
  variables = read_variables(args.variables)
  varnames = [var.name for var in variables]
  if args.includeinclusive: varnames = ['_eventBDTinclusive'] + varnames

  for varname in varnames:
    print('Running on variable {}'.format(varname))

    # find relevant workspace in the directory
    starttag = 'datacard_signalregion_dilepton_inclusive'
    if args.usecr: starttag = 'dc_combined'
    wspaces = ([ f for f in os.listdir(args.datacarddir)
                 if (f.startswith(starttag) 
                     and varname in f
                     and args.year in f 
                     and f.endswith('.root')) ])
    if len(wspaces)!=1:
      print('ERROR: wrong number of workspaces, skipping variable {}.'.format(varname))
      continue
    wspace = wspaces[0]
    card = wspace.replace('.root','.txt')

    # read results
    try:
        res = opt.read_signalstrength(
                     args.datacarddir, card,
                     statonly=False, mode='txt',
                     usedata=args.usedata)
    except:
        msg = 'WARNING: could not read result for card {}, setting to zero'.format(card)
        print(msg)
        res = (0, 0, 0)

    # add to dict
    info[varname.replace('_eventBDT','')] = res

  # make a plot
  fig, ax = makeplot(info, normalize=True)
  fig.savefig(args.outputfile)
