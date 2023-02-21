####################################################
# plot histograms obtained with fillResponseMatrix #
####################################################

import ROOT
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

#example: python3 plotResponseMatrices.py --inputfile output/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Autumn18.root --outputdir ~/public_html/2018 --variables ../variables/variables_responsematrices.json --writebincontentauto --docustom --response

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of detector to particle level comparison')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  parser.add_argument('--include_outerflow', default=False, action='store_true') # in true definitions outerflow should be False
  parser.add_argument('--writebincontent', default=False, action='store_true')
  parser.add_argument('--writebincontentauto', default=False, action='store_true')
  parser.add_argument('--dogeneric', default=False, action='store_true')
  parser.add_argument('--docustom', default=False, action='store_true')
  parser.add_argument('--response', default=False, action='store_true')
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
    binnings = var.bins
    if binnings == None:
        binnings = []

        binnr = var.nbins
        xmin = var.xlow
        xmax = var.xhigh
        avrg = (xmax-xmin)/binnr
        for i in range(binnr+1):
            binnings.append(xmin + avrg*i)
    
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
    colnormhist_withunderflow = rmt.normalize_columns(hist, include_outerflow=True)

    stability = rmt.get_stability(hist, include_outerflow=False)
    purity = rmt.get_purity(hist, include_outerflow=False)
    efficiency = rmt.get_efficiency(hist)

 
    # set common plot properties
    if( axtitle is not None and unit is not None ):
      axtitle += ' ({})'.format(unit)
    xaxtitle = 'Particle-level'
    yaxtitle = 'Detector-level'
    xtitleoffset = 1.2
    ytitleoffset = 1.2
    ztitleoffset = 1.2
    rightmargin = 0.2
    drawoptions = 'colz'
    if args.writebincontent: drawoptions += 'texte'
    if args.writebincontentauto:
      if hist.GetNbinsX()<10: drawoptions += 'texte'
    if args.response:
        histtitle = 'Response matrix for '+axtitle[:1].lower()+axtitle[1:]
    else:
        histtitle = 'Resolution matrix for '+axtitle[:1].lower()+axtitle[1:]
    lumi = None
    lumitext = ''
    extracmstext = 'Preliminary'
    extrainfos = []

    # generic approach using simple 2D histogram plotter
    if args.dogeneric:

      # make raw plot, against common approach but useful for statistics
      outfile = os.path.join(args.outputdir, varname+'_absolute')
      h2dp.plot2dhistogram( hist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions,
    	    docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

      # make column-normalized plot, against common approach
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
      xaxtitle = 'Particle level '+axtitle[:1].lower()+axtitle[1:]
      yaxtitle = 'Detector level '+axtitle[:1].lower()+axtitle[1:]

      if args.response:
          outfile = os.path.join(args.outputdir, 'responsetest' + varname + '_custom')
          # we also need to make the underflow bin part of the new histo but at the top. ugly fix is to make a new one and fill by hand.
          hist = rmt.AddUnderflowBins(colnormhist_withunderflow, binnings)

          rmp.plotresponsematrix(hist, efficiency, stability, purity, outfile, outfmts=['.png'],
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            drawoptions=drawoptions,
            lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

      else:
          outfile = os.path.join(args.outputdir, 'resolutiontest' + varname+'_custom')
          rmp.plotresponsematrix( colnormhist, efficiency, stability, purity, outfile, outfmts=['.png'],
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            drawoptions=drawoptions,
            lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
