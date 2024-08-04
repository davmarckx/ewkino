################################################################
# Plot the results of a simultaneous ttW+ and ttW- measurement #
################################################################

import sys
import os
import argparse
import numpy as np
import ROOT
from array import array
sys.path.append('../../plotting/python')
import plottools as pt
import pandas as pd

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Plot combine output')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True)
  parser.add_argument('--xaxbins', default=None)
  parser.add_argument('--yaxbins', default=None)
  parser.add_argument('--addinclusiveresult',
                    action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # read input file
  f = ROOT.TFile.Open(args.inputfile,'read')
  limit = f.Get('limit')
  # 'limit' is a TTree containing (among others) the following branches:
  # - r_TTWplus (or whatever POI name was chosen)
  # - r_TTWminus (or whatever POI name was chosen)
  # - deltaNLL (the negative log likelihood difference w.r.t. best fit)
  # each entry in the tree therefore contains a unique combination 
  # of r_TTWplus, r_TTWminus, and the corresponing deltaNLL value.

  # define axis variables and titles
  poix = 'r_TTWplus'
  xaxtitle = '#mu(ttW+)'
  poiy = 'r_TTWminus'
  yaxtitle = '#mu(ttW-)'
  zaxtitle = '-2 #Delta log(L)'
  
  # extract values as np arrays
  npoints = limit.GetEntries()
  rx = np.zeros(npoints)
  ry = np.zeros(npoints)
  dnll = np.zeros(npoints)
  for i in range(npoints):
    limit.GetEntry(i)
    rx[i] = getattr(limit, 'r_TTWplus')
    ry[i] = getattr(limit, 'r_TTWminus')
    dnll[i] = getattr(limit, 'deltaNLL')

  # set ranges and bins
  if args.xaxbins is None:
    xaxbins = (np.amin(rx), np.amax(rx), len(rx))
  else:
    xaxbins = args.xaxbins.split(',')
    xaxbins = (float(xaxbins[0]),float(xaxbins[1]),int(xaxbins[2]))
  if args.yaxbins is None:
    yaxbins = (np.amin(ry), np.amax(ry), len(ry))
  else:
    yaxbins = args.yaxbins.split(',')
    yaxbins = (float(yaxbins[0]),float(yaxbins[1]),int(yaxbins[2]))

  # make a 2D histogram out of the tree points
  hist = ROOT.TH2D("hist", "hist", xaxbins[2], xaxbins[0], xaxbins[1],
           yaxbins[2], yaxbins[0], yaxbins[1])
  for i in range(npoints):
    binnb = hist.FindBin(rx[i], ry[i])
    value = 2*dnll[i]
    if value > 10: value = 10
    hist.SetBinContent(binnb, value)

  # make contours
  # note: to draw several contours in differents styles,
  # the easiest approach seems to be to fit one contour each
  # to several copies of the histogram
  cont1hist = hist.Clone()
  cont1 = array('d')
  cont1.append(2.30)
  cont1hist.SetContour(1,cont1)
  cont1hist.SetLineWidth(2)
  cont1hist.SetLineColor(ROOT.kRed+2)
  cont4hist = hist.Clone()
  cont4 = array('d')
  cont4.append(5.99)
  cont4hist.SetContour(1,cont4)
  cont4hist.SetLineWidth(2)
  cont4hist.SetLineColor(ROOT.kGreen+3)

  if args.addinclusiveresult:
    df = pd.read_csv('inclusiveResult/inclusive_SCAN.csv')
    print(df.keys())
    npoints_incl = len(df)
    
    # init all the arrays
    rx_incl = df["ttW+"] / 497.5
    ry_incl = df["ttW-"] / 247.9
    dnll_incl = df["Delta"]

    hist_incl = ROOT.TH2D("hist_incl", "hist_incl", len(rx_incl.unique())/5, rx_incl.min(), rx_incl.max(),
                                                    len(ry_incl.unique())/5, ry_incl.min(), ry_incl.max())

    rx_incl = list(rx_incl)
    ry_incl = list(ry_incl)
    dnll_incl = list(dnll_incl)
    for i in range(npoints_incl-1):
      binnb = hist_incl.FindBin(rx_incl[i], ry_incl[i])
      value = dnll_incl[i]
      if value > 10: value = 10
      hist_incl.SetBinContent(binnb, value)

    cont1hist_incl = hist_incl.Clone()
    cont1 = array('d')
    cont1.append(2.30)
    cont1hist_incl.SetContour(1,cont1)
    cont1hist_incl.SetLineWidth(2)
    cont1hist_incl.SetLineColor(ROOT.kRed-7)
    cont1hist_incl.SetLineStyle(9)

    cont4hist_incl = hist_incl.Clone()
    cont4 = array('d')
    cont4.append(5.99)
    cont4hist_incl.SetContour(1,cont4)
    cont4hist_incl.SetLineWidth(2)
    cont4hist_incl.SetLineColor(ROOT.kGreen-9)
    cont4hist_incl.SetLineStyle(9)
  ### make a plot ###
  pt.setTDRstyle()
  ROOT.gROOT.SetBatch(ROOT.kTRUE)
    
  # set global properties
  cheight = 500 # height of canvas
  cwidth = 700 # width of canvas
  titlefont = 4
  titlesize = 22
  axtitlefont = 4
  axtitlesize = 22
  infofont = 4
  infosize = 15
  # title offset
  xtitleoffset = 1.5
  ytitleoffset = 1.5
  ztitleoffset = 1
  # margins
  topmargin = 0.07
  bottommargin = 0.15
  leftmargin = 0.15
  rightmargin = 0.2
  xmin = hist.GetXaxis().GetXmin()
  xmax = hist.GetXaxis().GetXmax()
  ymin = hist.GetYaxis().GetXmin()
  ymax = hist.GetYaxis().GetXmax()
  zmin = hist.GetMinimum()
  zmax = hist.GetMaximum()
  # extra info box parameters
  infoleft = leftmargin+0.05
  infotop = 1-topmargin-0.1

  # create canvas
  c1 = ROOT.TCanvas("c1","c1")
  c1.SetCanvasSize(cwidth,cheight)
  c1.SetTopMargin(topmargin)
  c1.SetBottomMargin(bottommargin)
  c1.SetLeftMargin(leftmargin)
  c1.SetRightMargin(rightmargin)
    
  # set title and label properties
  hist.GetXaxis().SetTitle(xaxtitle)
  hist.GetXaxis().SetTitleOffset(xtitleoffset)
  hist.GetXaxis().SetTitleFont(axtitlefont*10+3)
  hist.GetXaxis().SetTitleSize(axtitlesize)
  hist.GetYaxis().SetTitle(yaxtitle)
  hist.GetYaxis().SetTitleOffset(ytitleoffset)
  hist.GetYaxis().SetTitleFont(axtitlefont*10+3)
  hist.GetYaxis().SetTitleSize(axtitlesize)
  hist.GetZaxis().SetTitle(zaxtitle)
  hist.GetZaxis().SetTitleOffset(ztitleoffset)
  hist.GetZaxis().SetTitleFont(axtitlefont*10+3)
  hist.GetZaxis().SetTitleSize(axtitlesize)

  # set title
  ttitle = ROOT.TLatex()
  ttitle.SetTextFont(titlefont*10+3)
  ttitle.SetTextSize(titlesize)

  # draw histogram
  hist.Draw( 'colz' )
  #ttitle.DrawLatexNDC(leftmargin,0.9,histtitle)
  pt.drawLumi(c1, extratext='Preliminary', 
    cmstext_size_factor=0.8,
    cms_in_grid=True,
    lumitext='138 fb^{-1} (13 TeV)')

  # draw contours
  cont1hist.Draw('cont3 same')
  cont4hist.Draw('cont3 same')

  if args.addinclusiveresult:
    atlas = ROOT.TEllipse(583 / 497.5,296 / 247.9 ,.005,.005)
    atlas.SetLineWidth(3)
    cont1hist_incl.Draw('cont2 same')
    cont4hist_incl.Draw('cont2 same')
    atlas.Draw()

  # make and draw legend
  legend = ROOT.TLegend(leftmargin+0.02,1-topmargin-0.25,leftmargin+0.2,1-topmargin-0.1)
  legend.SetNColumns(1)
  legend.SetFillStyle(0)
  legend.AddEntry(cont1hist, '68% CL', "l")
  legend.AddEntry(cont4hist, '95% CL', "l")

  if args.addinclusiveresult:
    legend.AddEntry(cont1hist_incl, '68% CL TOP-21-011', "l")
    legend.AddEntry(cont4hist_incl, '95% CL TOP-21-011', "l")
    legend.AddEntry(atlas, 'ATLAS best fit' , "l")  

  legend.Draw('same')

  # draw extra info
  tinfo = ROOT.TLatex()
  tinfo.SetTextFont(10*infofont+3)
  tinfo.SetTextSize(infosize)
  #for i,info in enumerate(extrainfos):
  #      vspace = 0.07*(float(infosize)/20)
  #      tinfo.DrawLatexNDC(infoleft,infotop-(i+1)*vspace, info)

  # save the plot
  c1.Update()
  outfilepath = os.path.splitext(args.outputfile)[0]
  c1.SaveAs(outfilepath+'.png')
   
  # close the canvas
  c1.Close()

  ### for testing: make raw plot from TTree ###
  c2 = ROOT.TCanvas("c2","c2")
  varexp = '2*deltaNLL:{}:{}'.format(poiy,poix)
  binexp = 'h({},{},{},{},{},{})'.format(xaxbins[2],xaxbins[0],xaxbins[1],
    yaxbins[2], yaxbins[0], yaxbins[1])
  limit.Draw(varexp+'>>'+binexp, '2*deltaNLL<{}'.format(zmax), 'prof colz')
  c2.Update()
  c2.SaveAs(outfilepath+'_raw.png')
  c2.Close()
