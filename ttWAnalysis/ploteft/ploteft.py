#######################################
# Make plots of EFT variations #
#######################################
# This script is supposed to be used on an output file of runanalysis.py,
# i.e. a root file containing histograms with the following naming convention:
# <process tag>_<selection region>_<variable name>_<EFT variation>

import sys
import os
import json
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
from variabletools import read_variables
sys.path.append(os.path.abspath('../plotting'))
from infodicts import get_region_dict
from colors import getcolormap
sys.path.append(os.path.abspath('../plotsystematics'))
from systplotter import plotsystematics


if __name__=="__main__":
    
  # parse arguments
  parser = argparse.ArgumentParser('Plot systematics')
  parser.add_argument('-i', '--inputfile', required=True, type=os.path.abspath,
                      help='Input file to start from, supposed to be an output file'
                          +' from runsystematics.cc or equivalent')
  parser.add_argument('-y', '--year', required=True)
  parser.add_argument('-r', '--region', required=True)
  parser.add_argument('-p', '--process', required=True)
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('-o', '--outputdir', required=True, 
                      help='Directory where to store the output.')
  parser.add_argument('--datatag', default='data',
                      help='Process name of data histograms in input file.')
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # parse input file
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: requested to run on '+args.inputfile
                    +' but it does not seem to exist...')

  # parse the variables
  varlist = read_variables(args.variables)
  variablenames = [v.name for v in varlist]

  # parse tags
  extratags = []
  if args.tags is not None: extratags = args.tags.split(',')
  extratags = [t.replace('_',' ') for t in extratags]

  # make the output directory
  if not os.path.exists(args.outputdir):
    os.makedirs(args.outputdir)

  # get all relevant histograms
  print('Loading histogram names from input file...')
  mustcontainall = []
  mustcontainall.append(args.process)
  if len(variablenames)==1: mustcontainall.append(variablenames[0])
  mustcontainall.append('EFT')
  # do loading and initial selection
  histnames = ht.loadhistnames(args.inputfile, mustcontainall=mustcontainall)
  print('Initial selection:')
  print(' - mustcontainall: {}'.format(mustcontainall))
  print('Resulting number of histograms: {}'.format(len(histnames)))
  # select variables
  histnames = lt.subselect_strings(histnames, mustcontainone=variablenames)[1]
  print('Further selection (variables):')
  print('Resulting number of histograms: {}'.format(len(histnames)))

  # do printouts (ony for testing)
  #for histname in histnames: print('  {}'.format(histname))

  # loop over variables
  for var in varlist:

    # get name and title
    variablename = var.name
    xaxtitle = var.axtitle
    print('Now running on variable {}...'.format(variablename))

    # extra histogram selection for overlapping variable names
    othervarnames = [v.name for v in varlist if v.name!=variablename]
    thishistnames = lt.subselect_strings(histnames,
                      mustcontainall=[variablename],
                      maynotcontainone=['_{}_'.format(el) for el in othervarnames])[1]

    # get the nominal histogram name
    splittag = args.region+'_'+variablename
    nominalhistname = '_'.join([args.process, splittag, 'EFTsm'])
    if not nominalhistname in thishistnames:
        msg = 'ERROR: nominal histogram {} not found.'.format(nominalhistname)
        raise Exception(msg)
 
    syshistnames = []
    for thishistname in thishistnames:
      if 'EFTsm' in thishistname: continue
      syshistnames.append(thishistname)

    # load histograms
    nominalhist = ht.loadhistogramlist(args.inputfile, [nominalhistname])[0]
    syshistlist = ht.loadhistogramlist(args.inputfile, syshistnames)

    # printouts for testing
    print(nominalhist)
    for hist in syshistlist: print(hist)

    # format the labels
    # (remove year and process tags for more readable legends)
    for hist in syshistlist:
      label = str(hist.GetName()).split(splittag)[-1].replace('_EFT','')
      hist.SetTitle(label)

    # make extra infos to display on plot
    extrainfos = []
    # processes
    pinfostr = 'Process: {}'.format(args.process)
    # year
    yeartag = args.year.replace('run2', 'Run 2')
    extrainfos.append(yeartag)
    # region
    regiontag = get_region_dict().get(args.region, args.region)
    extrainfos.append(regiontag)
    # others
    for tag in extratags: extrainfos.append(tag)

    # choose color map
    colormap = getcolormap('eft')

    # set plot properties
    figname = args.inputfile.split('/')[-1].replace('.root','')+'_var_'+variablename 
    figname = os.path.join(args.outputdir,figname)
    yaxtitle = 'Events'
    relyaxtitle = 'Normalized'
    # make absolute plot
    plotsystematics(nominalhist, syshistlist, figname+'_abs', 
                    colormap=colormap,
                    yaxtitle=yaxtitle, xaxtitle=xaxtitle,
                    style='absolute', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
    # make normalized plot
    plotsystematics(nominalhist, syshistlist, figname+'_nrm',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='normalized', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
    # make relative plot
    plotsystematics(nominalhist, syshistlist, figname+'_rel',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='relative', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
