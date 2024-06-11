####################################################
# plot histograms obtained with fillResponseMatrix #
####################################################

import ROOT
import sys
import os
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
from variabletools import HistogramVariable, DoubleHistogramVariable, read_variables
sys.path.append(os.path.abspath('../../plotting/python'))
import hist2dplotter as h2dp
sys.path.append(os.path.abspath('tools'))
import responsematrixtools as rmt
import responsematrixplotter as rmp

#example: python3 plotResponseMatrices.py --inputfile output/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Autumn18.root --outputdir ~/public_html/2018 --variables ../variables/variables_responsematrices.json --writebincontentauto --docustom --response

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of detector to particle level comparison')
  parser.add_argument('-i', '--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('-o', '--outputdir', required=True)
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath)
  parser.add_argument('-e', '--event_selections', required=True, nargs='+')
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

  masses = ['low','high']

  # loop over variables
  for var in variables:
   for mass in masses: 
    # check variable type
    if isinstance(var, DoubleHistogramVariable):
      msg = 'ERROR: variable {} is of type DoubleHistogramVariable'.format(var.name)
      msg += ' wile a HistogramVariable was expected, skipping this variable.'
      print(msg)
      continue

    # initialize some settings
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
    print('Now running on variable {}...'.format(varname))

    # loop over event selections
    for event_selection in args.event_selections:
      selection_tag = event_selection + '_' + varname

      # select histograms
      thishists = ht.selecthistograms(histlist, mustcontainall=[selection_tag, mass])[1]
      # additional selections for overlapping histogram names
      thishists = ([hist for hist in thishists if
                    (hist.GetName().endswith(varname) or varname+'_' in hist.GetName())])
      if len(thishists)==0:
        print('ERROR: histogram list for tag {} is empty,'.format(selection_tag)
               +' skipping this variable.')
        continue
      if len(thishists)!=1:
        msg = 'ERROR: {} histograms found for tag {}'.format(len(thishists), selection_tag)
        msg += ' while 1 was expected; check file content.'
        raise Exception(msg)
      hist = thishists[0]

      # get normalized histograms, stability and purity
      colnormhist = rmt.normalize_columns(hist, include_outerflow=False)
      rownormhist = rmt.normalize_rows(hist, include_outerflow=False)
      colnormhist_outerflow = rmt.normalize_columns(hist, include_outerflow=True)

      stability = rmt.get_stability(hist, include_outerflow=False)
      purity = rmt.get_purity(hist, include_outerflow=False)
      efficiency = rmt.get_efficiency(hist)

      # set common plot properties
      if( axtitle is not None and unit is not None ): axtitle += ' ({})'.format(unit)
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
      lumi = None
      lumitext = ''
      extracmstext = 'Preliminary'
      extrainfos = []

      # generic approach using simple 2D histogram plotter
      if args.dogeneric:

        # make raw plot, against common approach but useful for statistics
        outfile = os.path.join(args.outputdir, varname+'_absolute'+mass)
        histtitle = 'Resolution matrix for '+axtitle[:1].lower()+axtitle[1:]
        h2dp.plot2dhistogram( hist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions,
    	    docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

        # make column-normalized plot
        outfile = os.path.join(args.outputdir, varname+'_colnorm'+mass)
        histtitle = 'Column normalized resolution matrix for '+axtitle[:1].lower()+axtitle[1:]
        h2dp.plot2dhistogram( colnormhist, outfile, outfmts=['.png'],
            histtitle=histtitle,
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events (column-normalized)',
            xtitleoffset=xtitleoffset, ytitleoffset=ytitleoffset, ztitleoffset=ztitleoffset,
            rightmargin=rightmargin,
            drawoptions=drawoptions, cmin=0., cmax=1.,
            docmstext=True, lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

        # make row-normalized plot
        outfile = os.path.join(args.outputdir, varname+'_rownorm'+mass)
        histtitle = 'Row normalized resolution matrix for '+axtitle[:1].lower()+axtitle[1:]
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
      
        # do response matrix
        outfile = os.path.join(args.outputdir, varname+'_response'+mass)
        # we also need to make the underflow bin part of the new histo but at the top.
        # ugly fix is to make a new one and fill by hand.
        # update: not needed since this information is already shown in efficiency,
        #         maybe to be re-discussed later.
        #colnormhist_outerflow = rmt.AddUnderflowBins(colnormhist_outerflow, binnings)
        rmp.plotresponsematrix( colnormhist_outerflow, efficiency, 
            stability, purity, outfile, outfmts=['.png'],
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events (normalized)',
            drawoptions=drawoptions,
            lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )

        # do resolution matrix
        outfile = os.path.join(args.outputdir, varname+'_resolution'+mass)
        rmp.plotresponsematrix( colnormhist, efficiency, 
            stability, purity, outfile, outfmts=['.png'],
            xtitle=xaxtitle, ytitle=yaxtitle, ztitle='Number of events (normalized)',
            drawoptions=drawoptions,
            lumitext=lumitext, extracmstext=extracmstext,
            extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
