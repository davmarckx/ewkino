################################################
# plot histograms obtained with fillComparison #
################################################

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
  parser = argparse.ArgumentParser('Plot results of detector to particle level comparison')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
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
    if len(thishists)!=2:
      msg = 'ERROR: {} histograms found for variable {}'.format(len(thishists),varname)
      msg += ' while 2 were expected; check file content.'
      raise Exception(msg)
      # this error will be triggered in case fillComparison is run 
      # with multiple event selections and/or selection types;
      # assume for now that that will not happen;
      # extend histogram selection later if needed.
    if 'particlelevel' in thishists[0].GetName():
      plhist = thishists[0]
      dlhist = thishists[1]
    else:
      plhist = thishists[1]
      dlhist = thishists[0]
    
    # printouts for testing
    print('found following histograms:')
    print('detector level: {}'.format(dlhist.GetName()))
    print('particle level: {}'.format(plhist.GetName()))

    # set plot properties
    xaxtitle = axtitle
    if( axtitle is not None and unit is not None ):
      xaxtitle += ' ({})'.format(unit)
    yaxtitle = 'Events'
    outfile = os.path.join(args.outputdir, varname)
    lumi = None
    extrainfos = []
    xlabels = None
    #labelsize = None
    #if( var.iscategorical and var.xlabels is not None ):
    #    xlabels = var.xlabels
    #    labelsize = 15

    # make plot
    mhp.plotmultihistograms( [dlhist,plhist],
            figname=outfile, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            normalize=False, normalizefirst=False,
            dolegend=True, labellist=['Detector level','Particle level'],
            colorlist=None,
            logy=False,
            drawoptions='',
            lumitext='', extracmstext='Preliminary',
            doratio=False, ratiorange=None, ylims=None, yminzero=False,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
