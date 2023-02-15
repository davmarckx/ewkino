####################################################
# plot histograms obtained with fillResponseMatrix #
####################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
from variabletools import read_variables
sys.path.append(os.path.abspath('../../plotting/python'))
import hist2dplotter as h2dp
sys.path.append(os.path.abspath('tools'))
import responsematrixtools as rmt
import responsematrixplotter as rmp

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of detector to particle level comparison')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--include_outerflow', default=False, action='store_true')
  parser.add_argument('--writebincontent', default=False, action='store_true')
  parser.add_argument('--writebincontentauto', default=False, action='store_true')
  parser.add_argument('--dogeneric', default=False, action='store_true')
  parser.add_argument('--docustom', default=False, action='store_true')
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
    if len(thishists)!=1:
      msg = 'ERROR: {} histograms found for variable {}'.format(len(thishists),varname)
      msg += ' while 1 was expected; check file content.'
      raise Exception(msg)
      # this error will be triggered in case fillResponseMatrices is run 
      # with multiple event selections;
      # assume for now that that will not happen;
      # extend histogram selection later if needed.
    hist = thishists[0]

    # get normalized histograms, stability and purity
    colnormhist = rmt.normalize_columns(hist, include_outerflow=args.include_outerflow)
    rownormhist = rmt.normalize_rows(hist, include_outerflow=args.include_outerflow)
    stability = rmt.get_stability(hist, include_outerflow=args.include_outerflow)
    purity = rmt.get_purity(hist, include_outerflow=args.include_outerflow)
 
    # set common plot properties
    if( axtitle is not None and unit is not None ):
      axtitle += ' ({})'.format(unit)
    xaxtitle = 'Particle-level'
    yaxtitle = 'Detectorl-level'
    xtitleoffset = 1.2
    ytitleoffset = 1.2
    ztitleoffset = 1.2
    rightmargin = 0.2
    drawoptions = 'colz'
    if args.writebincontent: drawoptions += 'texte'
    if args.writebincontentauto:
      if hist.GetNbinsX()<10: drawoptions += 'texte'
    histtitle = 'Response matrix for '+axtitle[:1].lower()+axtitle[1:]
    lumi = None
    lumitext = ''
    extracmstext = 'Preliminary'
    extrainfos = []

    # generic approach using simple 2D histogram plotter
    if args.dogeneric:

      # make raw plot
      outfile = os.path.join(args.outputdir, varname+'_absolute')
      h2dp.plot2dhistogram( hist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions,
    	    docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

      # make column-normalized plot
      outfile = os.path.join(args.outputdir, varname+'_colnorm')
      h2dp.plot2dhistogram( colnormhist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events (column-normalized)',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions, cmin=0., cmax=1.,
            docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

      # make row-normalized plot
      outfile = os.path.join(args.outputdir, varname+'_rownorm')
      h2dp.plot2dhistogram( rownormhist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events (row-normalized)',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions, cmin=0., cmax=1.,
            docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

    # custom approach using dedicated plotting function
    if args.docustom:
      outfile = os.path.join(args.outputdir, varname+'_custom')
      xaxtitle = 'Particle level '+axtitle[:1].lower()+axtitle[1:]
      yaxtitle = 'Detector level '+axtitle[:1].lower()+axtitle[1:]
      rmp.plotresponsematrix( hist, stability, purity, outfile, outfmts=['.png'],
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            drawoptions=drawoptions,
            lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
