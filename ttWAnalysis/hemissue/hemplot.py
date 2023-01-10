#########################################
# plot histograms obtained with hemfill #
#########################################

import sys
import os
import argparse
sys.path.append('../../Tools/python')
import histtools as ht
from variabletools import read_variables
sys.path.append('../../plotting/python')
import multihistplotter as mhp

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot HEM issue histograms')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))
  if not os.path.exists(args.outputdir):
    os.makedirs(args.outputdir)
  if not os.path.exists(args.variables):
    raise Exception('ERROR: variable file {} does not exist.'.format(args.variables))
  
  # parse tags
  extratags = []
  if args.tags is not None: extratags = args.tags.split(',')
  extratags = [t.replace('_',' ') for t in extratags]

  # read variables
  variables = read_variables( args.variables )

  # read all histograms
  histlist = ht.loadallhistograms(args.inputfile)

  # loop over variables
  for var in variables:
    varname = var.name
    axtitle = var.axtitle
    unit = var.unit
    print('now running on variable {}...'.format(varname))

    # select histograms
    thishists = ht.selecthistograms(histlist, mustcontainall=[varname])[1]
    # additional selections for overlapping histogram names
    thishists = ([hist for hist in thishists if
                  (hist.GetName().endswith(varname) or varname+'_' in hist.GetName())])
    if len(thishists)==0:
      print('ERROR: histogram list for variable {} is empty,'.format(varname)
            +' skipping this variable.')
      continue
    if len(thishists)!=3:
      msg = 'ERROR: {} histograms found for variable {}'.format(len(thishists),varname)
      msg += ' while 3 were expected; check file content.'
      raise Exception(msg)
    nominalhist = ht.selecthistograms(thishists, mustcontainall=['nominal'])[1][0]
    uphist = ht.selecthistograms(thishists, mustcontainall=['HEM1516Up'])[1][0]
    downhist = ht.selecthistograms(thishists, mustcontainall=['HEM1516Down'])[1][0]
 
    # printouts for testing
    print('found following histograms:')
    print('nominal: {}'.format(nominalhist.GetName()))
    print('up: {}'.format(uphist.GetName()))
    print('down: {}'.format(downhist.GetName()))

    # set plot properties
    xaxtitle = axtitle
    if( axtitle is not None and unit is not None ):
      xaxtitle += ' ({})'.format(unit)
    yaxtitle = 'Events'
    outfile = os.path.join(args.outputdir, varname)
    lumi = None
    extrainfos = []
    for tag in extratags: extrainfos.append(tag)
    xlabels = None
    #labelsize = None
    #if( var.iscategorical and var.xlabels is not None ):
    #    xlabels = var.xlabels
    #    labelsize = 15

    # make plot
    plothistlist = [nominalhist, uphist]
    labellist = ['Nominal', 'HEM15/16 variation']
    mhp.plotmultihistograms( plothistlist,
            figname=outfile, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            normalize=False, normalizefirst=False,
            dolegend=True, labellist=labellist,
            colorlist=None,
            logy=False,
            drawoptions='hist e',
            lumitext='', extracmstext='Preliminary',
            doratio=True, ratiorange=(0.901,1.099), ylims=None, yminzero=False,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
