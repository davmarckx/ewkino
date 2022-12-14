####################################################
# make plots of the histograms filled with cutflow #
####################################################

import sys
import os
import ROOT
import json
import argparse
sys.path.append('../../Tools/python')
import histtools as ht
sys.path.append('../../plotting/python')
import singlehistplotter as shp

### help functions for creating alternative views of the cutflow histogram
# note: the input histogram for each of these functions is the output of fillCutflow.cc.
#       (in each bin, absolute number of events that fail a given cut,
#	last bin: number of passing events)

def make_cumulative_absolute( hist ):
    ### output histograms: in each bin, total amount of remaining events after each cut

    # copy original histogram skeleton but with an extra first bin (before any cuts)
    # and one bin less (removing final bin)
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    chist = ROOT.TH1D( "", title, nbins, -0.5, nbins+0.5 )
    # set first bin
    nevents = hist.Integral()
    chist.SetBinContent(1, nevents)
    chist.SetBinError(1,0)
    chist.GetXaxis().SetBinLabel(1, "Before selections")
    # fill further bins
    for i in range(2,chist.GetNbinsX()+1):
	nevents = nevents - hist.GetBinContent(i-1)
	chist.SetBinContent(i, nevents)
	chist.SetBinError(i,0)
	chist.GetXaxis().SetBinLabel(i, 
	    hist.GetXaxis().GetBinLabel(i-1).replace('Fail', 'Pass'))
    return chist

def make_cumulative_relative( hist ):
    ### output histogram: same as make_cumulative_absolute_pass but with y-axis in per cent
    chist = make_cumulative_absolute( hist )
    scale = 1./hist.Integral()
    chist.Scale(scale)
    chist.SetMaximum(1.2)
    return chist

def make_relative_fail( hist ):
    ### output histogram: in each bin, relative fraction of events that fail given cut
    #			  (relative w.r.t. number of events that pass all previous cuts!)
    helphist = make_cumulative_absolute( hist )
    nevents = hist.Integral()
    # copy original histogram skeleton but with last bin removed
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    rhist = ROOT.TH1D( "", title, nbins-1, -0.5, nbins-0.5 )
    for i in range(1, rhist.GetNbinsX()+1):
	nfail = float(hist.GetBinContent(i))
	ntot = helphist.GetBinContent(i)
	rhist.SetBinContent(i, nfail/ntot)
	rhist.SetBinError(i,0)
	rhist.GetXaxis().SetBinLabel(i,
            hist.GetXaxis().GetBinLabel(i))
    rhist.SetMaximum(1.2)
    return rhist

def make_relative_pass( hist ):
    ### output histogram: same as make_relative_fail but with passing fraction instead of failing
    rhist = make_relative_fail( hist )
    for i in range(1, rhist.GetNbinsX()+1 ):
	rhist.SetBinContent(i, 1-rhist.GetBinContent(i))
	rhist.GetXaxis().SetBinLabel(i, 
	    rhist.GetXaxis().GetBinLabel(i).replace('Fail','Pass'))
    rhist.SetMaximum(1.2)
    return rhist
  

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot cutflow')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--extrainfos', default=None)
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
  extrainfos = None
  if args.extrainfos is not None:
    extrainfos = args.extrainfos.split(',')

  # get the histogram
  histlist = ht.loadallhistograms(args.inputfile)
  if len(histlist)!=1:
    raise Exception('ERROR: unexpected number of histograms found: {}'.format(len(histlist)))
  hist = histlist[0]

  # print the histogram to get numerical values
  #ht.printhistogram(hist)

  # modify the angle of the labels
  hist.GetXaxis().LabelsOption('v')

  # common settings for all plots
  xaxtitle = ''
  infoleft = 0.2
  infotop = 0.96
    
  # make raw plot
  yaxtitle = 'Number of events'
  drawoptions = 'hist e'
  outputfile = os.path.join(args.outputdir,'raw.png')
  shp.plotsinglehistogram(hist, outputfile, title=None, 
			    xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, 
                            drawoptions=drawoptions,
                            lumitext='', extralumitext='',
			    bottommargin=0.45,
                            writebincontent=False, bincontentfont=None,
                            bincontentsize=None, bincontentfmt=None,
                            extrainfos=extrainfos, infosize=None, 
                            infoleft=infoleft, infotop=infotop)

  # make cumulative absolute plot
  cumabshist = make_cumulative_absolute( hist )
  cumabshist.GetXaxis().LabelsOption('v')
  yaxtitle = 'Number of remaining events'
  drawoptions = 'hist e'
  outputfile = os.path.join(args.outputdir,'cumulative_absolute.png')
  shp.plotsinglehistogram(cumabshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, 
                            drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
                            writebincontent=False, bincontentfont=None,
                            bincontentsize=None, bincontentfmt=None,
                            extrainfos=extrainfos, infosize=None,
                            infoleft=infoleft, infotop=infotop)

  # make cumulative relative plot
  cumrelhist = make_cumulative_relative( hist )
  cumrelhist.GetXaxis().LabelsOption('v')
  yaxtitle = 'Fraction of remaining events'
  drawoptions = 'hist e'
  outputfile = os.path.join(args.outputdir,'cumulative_relative.png')
  shp.plotsinglehistogram(cumrelhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, 
                            drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
                            writebincontent=True, bincontentfont=None,
                            bincontentsize=None, bincontentfmt=None,
                            extrainfos=extrainfos, infosize=None,
                            infoleft=infoleft, infotop=infotop)

  # make relative fail plot
  relfailhist = make_relative_fail( hist )
  relfailhist.GetXaxis().LabelsOption('v')
  yaxtitle = 'Fraction of failing events'
  drawoptions = 'hist e'
  outputfile = os.path.join(args.outputdir,'relative_fail.png')
  shp.plotsinglehistogram(relfailhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, 
                            drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
                            writebincontent=True, bincontentfont=None,
                            bincontentsize=None, bincontentfmt=None,
                            extrainfos=extrainfos, infosize=None,
                            infoleft=infoleft, infotop=infotop)

  # make relative pass plot
  relpasshist = make_relative_pass( hist )
  relpasshist.GetXaxis().LabelsOption('v')
  yaxtitle = 'Fraction of passing events'
  drawoptions = 'hist e'
  outputfile = os.path.join(args.outputdir,'relative_pass.png')
  shp.plotsinglehistogram(relpasshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, 
                            drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
                            writebincontent=True, bincontentfont=None,
                            bincontentsize=None, bincontentfmt=None,
                            extrainfos=extrainfos, infosize=None,
                            infoleft=infoleft, infotop=infotop)

