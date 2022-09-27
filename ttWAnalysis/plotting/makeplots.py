##############
# make plots #
##############

# input histograms are supposed to be contained in a single root file.
# the naming of the histograms should be <process name>_<region>_<variable name>_<systematic>.
# note that only "nominal" is supported as systematic for now.

import sys
import os
import argparse
sys.path.append('../../Tools/python')
import histtools as ht
from variabletools import read_variables
sys.path.append('../../plotting/python')
import histplotter as hp
import colors


def getmcsysthist(mchistlist, npunc=0.3):
  ### get a histogram with systematic uncertainties
  # note: dummy implementation with only nonprompt uncertainty!
  syshist = mchistlist[0].Clone()
  syshist.Reset()
  totmchist = mchistlist[0].Clone()
  totmchist.Reset()
  nphist = None
  for hist in mchistlist:
    totmchist.Add(hist)
    syshist.Add(hist)
    if hist.GetTitle()=="nonprompt": nphist = hist.Clone()
  if nphist is None: return None
  if nphist.GetBinContent(1)<1e-4: nphist.SetBinContent(1,0.)
  syshist.Add(nphist,npunc)
  syshist.Add(totmchist,-1)
  return syshist


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make plots')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True)
  parser.add_argument('--region', required=True)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--colormap', default='default')
  parser.add_argument('--extracmstext', default='Preliminary')
  parser.add_argument('--unblind', default=False)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # read all histograms
  histlist = ht.loadallhistograms(args.inputfile)
  histlist = ht.selecthistograms(histlist,mustcontainall=[args.region])[1]
  if len(histlist)==0:
    raise Exception('ERROR: histogram list is empty, cannot make plots.')

  # hard-coded fixes for overlapping region names...
  if( args.region=='nonprompt_trilepton' ):
    histlist = ht.selecthistograms(histlist,maynotcontainone=['noz','noossf'])[1]
  if len(histlist)==0:
    raise Exception('ERROR: histogram list is empty, cannot make plots.')

  # make output directory
  if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)

  # read variables
  variables = read_variables( args.variables )  

  # loop over variables
  for var in variables:
    varname = var.name
    axtitle = var.axtitle
    unit = var.unit
    
    # select histograms
    thishists = ht.selecthistograms(histlist, mustcontainall=[varname])[1]
    if len(histlist)==0:
      print('ERROR: histogram list for variable {} is empty,'.format(varname)
            +' skipping this variable.')
      continue
    
    # find data and sim histograms
    datahists = []
    simhists = []
    for hist in thishists:
      if( hist.GetName().startswith('data') or hist.GetName().startswith('Data') ): 
        datahists.append(hist)
      else: simhists.append(hist)
    if len(datahists)==0:
      print('WARNING: no data histogram found, plotting simulation only.')
      datahist = simhists[0].Clone()
      args.unblind = False
    elif len(datahists)!=1:
      raise Exception('ERROR: expecting one data histogram'
		      +' but found {}'.format(len(datahists)))
    else: datahist = datahists[0]

    # blind data histogram
    if not args.unblind:
      for i in range(0,datahist.GetNbinsX()+2):
        datahist.SetBinContent(i, 0)
        datahist.SetBinError(i, 0)

    # set plot properties
    xaxtitle = axtitle
    if( axtitle is not None and unit is not None ):
      xaxtitle += ' ({})'.format(unit)
    yaxtitle = 'Events'
    outfile = os.path.join(args.outputdir, varname)
    lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
		    '2016PreVFP':19520, '2016PostVFP':16810 }
    if not args.year in lumimap.keys():
      print('WARNING: year {} not recognized,'.format(args.year)
            +' will not write lumi header.')
    lumi = lumimap.get(args.year,None)
    colormap = colors.getcolormap(style=args.colormap)
    npunc = 0.3
    simsysthist = getmcsysthist(simhists, npunc=npunc)

    # make the plot
    hp.plotdatavsmc(outfile, datahist, simhists,
	    mcsysthist=simsysthist, 
	    xaxtitle=xaxtitle,
	    yaxtitle='Number of events',
	    colormap=colormap,
	    lumi=lumi, extracmstext=args.extracmstext )
