#####################################################
# dedicated plotting function for response matrices #
#####################################################
# note: similar to ewkino/plotting/python/hist2dplotter,
#       but adding additional pad for stability and purity.

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../../../Tools/python'))
import histtools as ht
sys.path.append(os.path.abspath('../../../plotting/python'))
import plottools as pt
import hist2dplotter as h2dp

def plotresponsematrix( hist, efficiency, stability, purity, outfilepath, 
                    outfmts=['.png'],
		    histtitle=None, logx=False, logy=False,
                    xtitle=None, ytitle=None, ztitle=None,
                    axtitlefont=None, axtitlesize=None,
                    titlefont=None, titlesize=None,
		    drawoptions='colztexte', cmin=None, cmax=None,
		    bintextoptions=None,
		    extracmstext='', lumitext='',
		    extrainfos=[], infofont=None, infosize=None, 
                    infoleft=None, infotop=None ):
    # options:
    # - cmin and cmax: minimum and maximum values for the color scales
    #   note: in default "colz" behaviour, bins above cmax are colored as cmax,
    #         while bins below cmin are left blank.
    #         they can be colored as cmin by using drawoption "col0z" instead of "colz",
    #         but "col0ztexte" does not seem to write the bin contents for those bins...

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    
    # set global properties
    cheight = 700 # height of canvas
    cwidth = 700 # width of canvas
    if titlefont is None: titlefont = 4
    if titlesize is None: titlesize = 25
    if axtitlefont is None: axtitlefont = 4
    if axtitlesize is None: axtitlesize = 25
    if infofont is None: infofont = 4
    if infosize is None: infosize = 15
    labelfont = 4
    labelsize = 22
    # margins
    p1topmargin = 0.07
    p1bottommargin = 0.05
    p2topmargin = 0.02
    p2bottommargin = 0.35
    p3topmargin = 0.02
    p3bottommargin = 0.05
    leftmargin = 0.15
    rightmargin = 0.2
    p1titleoffset = 1.5
    p2titleoffset = 2
    p3titleoffset = 2
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    # get the height of the underflow bin to draw a separating line
    ybins = hist.GetNbinsY()
    YShape = hist.ProjectionY('shape') 
    lowedge = YShape.GetBinLowEdge(ybins)
    zmin = hist.GetMinimum()
    zmax = hist.GetMaximum()
    if cmin is not None: zmin = cmin
    if cmax is not None: zmax = cmax
    hist.SetMinimum(zmin)
    hist.SetMaximum(zmax)
    # extra info box parameters
    if infoleft is None: infoleft = leftmargin+0.05
    if infotop is None: infotop = 1-p1topmargin-0.1
    # options for bin texts
    if bintextoptions is not None: ROOT.gStyle.SetPaintTextFormat( bintextoptions )
    # legend boxes
    p2legendbox = ([leftmargin+0.03, 1-p2topmargin-0.13,
                    1-rightmargin-0.03, 1-p2topmargin-0.03])
    p3legendbox = ([leftmargin+0.03, 1-p3topmargin-0.13,
                    0.3, 1-p3topmargin-0.03])
    # create canvas
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth, cheight)
    
    # create pad for response matrix
    pad1 = ROOT.TPad("pad1", "pad1", 0., 0.35, 1., 1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(p1bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetTicks(1,1)
    pad1.SetFrameLineWidth(2)
    pad1.Draw()


    # create pad for efficiency
    pad3 = ROOT.TPad("pad3", "pad3", 0., 0.175, 1., 0.35)
    pad3.SetTopMargin(p2topmargin)
    pad3.SetBottomMargin(p2bottommargin)
    pad3.SetLeftMargin(leftmargin)
    pad3.SetRightMargin(rightmargin)
    pad3.SetTicks(1,1)
    pad3.SetFrameLineWidth(2)
    pad3.SetGrid()
    pad3.Draw()

    # create pad for stability and purity
    pad2 = ROOT.TPad("pad2", "pad2", 0., 0., 1., 0.175)
    pad2.SetTopMargin(p2topmargin)
    pad2.SetBottomMargin(p2bottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.SetTicks(1,1)
    pad2.SetFrameLineWidth(2)
    pad2.SetGrid()
    pad2.Draw()
    
    ### operations on response histogram ###

    # create copy of response histogram with adapted outerflow bins if needed
    if( logx or logy ):
        hist = h2dp.getsmallouterflowhist( hist )
        hist.SetMinimum(zmin)
        hist.SetMaximum(zmax)

    # set title and label properties of response histogram
    if xtitle is not None: 
      hist.GetXaxis().SetTitleSize(0)
      hist.GetXaxis().SetLabelSize(0)
    if ytitle is not None: 
      hist.GetYaxis().SetTitle(ytitle)
      hist.GetYaxis().SetTitleOffset(p1titleoffset)
      hist.GetYaxis().SetTitleFont(axtitlefont*10+3)
      hist.GetYaxis().SetTitleSize(axtitlesize)
      hist.GetYaxis().SetLabelFont(labelfont*10+3)
      hist.GetYaxis().SetLabelSize(labelsize)
    if ztitle is not None: 
      hist.GetZaxis().SetTitle(ztitle)
      hist.GetZaxis().SetTitleOffset(p1titleoffset)
      hist.GetZaxis().SetTitleFont(axtitlefont*10+3)
      hist.GetZaxis().SetTitleSize(axtitlesize)

    ### operations on stability and purity ###
   
    # style operations
    stability.SetLineWidth(3)
    stability.SetLineColor(ROOT.kBlue)
    stability.SetMarkerSize(0)
    purity.SetLineWidth(3)
    purity.SetLineColor(ROOT.kRed)
    purity.SetMarkerSize(0)
    efficiency.SetLineWidth(3)
    efficiency.SetLineColor(ROOT.kGray+3)
    efficiency.SetMarkerSize(0)

    # minimum and maximum
    (totmin, totmax) = ht.getminmax([stability,purity],
                       includebinerror=True)
    margin = (totmax-totmin)/5.
    stability.SetMaximum(totmax + 3*margin)
    stability.SetMinimum(totmin - margin)

    effmin = efficiency.GetMinimum()
    effmax = efficiency.GetMaximum()
    margin = (effmax-effmin)/5.
    efficiency.SetMaximum(effmax + 3*margin)
    efficiency.SetMinimum(effmin - margin)

    # axes
    stability.GetXaxis().SetTitle(xtitle)
    stability.GetXaxis().SetTitleOffset(p2titleoffset*2)
    stability.GetXaxis().SetTitleFont(axtitlefont*10+3)
    stability.GetXaxis().SetTitleSize(axtitlesize)
    stability.GetXaxis().SetLabelFont(labelfont*10+3)
    stability.GetXaxis().SetLabelSize(labelsize)
    stability.GetYaxis().SetLabelFont(labelfont*10+3)
    stability.GetYaxis().SetLabelSize(labelsize)

    efficiency.GetXaxis().SetLabelFont(labelfont*10+3)
    efficiency.GetXaxis().SetLabelSize(labelsize)
    efficiency.GetYaxis().SetLabelFont(labelfont*10+3)
    efficiency.GetYaxis().SetLabelSize(labelsize)

    # make legend
    p2legend = ROOT.TLegend(p2legendbox[0], p2legendbox[1],
                 p2legendbox[2], p2legendbox[3] )
    p2legend.SetNColumns(2)
    p2legend.SetFillColor(ROOT.kWhite)
    p2legend.SetTextFont(10*infofont+3)
    p2legend.SetBorderSize(1)
    p2legend.AddEntry(stability, 'Stability', "l")
    p2legend.AddEntry(purity, 'Purity', "l")

    p3legend = ROOT.TLegend(p3legendbox[0], p3legendbox[1],
                 p3legendbox[2], p3legendbox[3] )
    p3legend.SetNColumns(2)
    p3legend.SetFillColor(ROOT.kWhite)
    p3legend.SetTextFont(10*infofont+3)
    p3legend.SetBorderSize(1)
    p3legend.AddEntry(efficiency, 'Efficiency', "l")

    ### make additional objects ###

    # set log scale if requested
    if logx:
        pad1.SetLogx()
        hist.GetXaxis().SetMoreLogLabels()
        pad2.SetLogx()
	
    if logy:
        pad1.SetLogy()
        hist.GetYaxis().SetMoreLogLabels()

    # make title
    ttitle = ROOT.TLatex()
    ttitle.SetTextFont(titlefont*10+3)
    ttitle.SetTextSize(titlesize)

    ### drawing ###

    # draw all objects in the response histogram pad
    pad1.cd()
    hist.Draw( drawoptions )
    myline = ROOT.TLine(xmin,lowedge,xmax,lowedge)
    myline.SetLineColorAlpha(2, 1) # black 1, red 2, blue 4, darkgray 12-14
    myline.SetLineWidth(3)
    myline.SetLineStyle(9)    
    myline.Draw("same")
    if histtitle is not None: ttitle.DrawLatexNDC(leftmargin,0.9,histtitle)
    pt.drawLumi(c1, extratext=extracmstext, cmstext_size_factor=0.6,
        cms_in_grid=True, lumitext=lumitext)

    # draw extra info
    tinfo = ROOT.TLatex()
    tinfo.SetTextFont(10*infofont+3)
    tinfo.SetTextSize(infosize)
    for i,info in enumerate(extrainfos):
        vspace = 0.07*(float(infosize)/20)
        tinfo.DrawLatexNDC(infoleft,infotop-(i+1)*vspace, info)

    # draw all objects in the stability/purity pad
    pad2.cd()
    stability.Draw()
    purity.Draw('same')
    p2legend.Draw('same')

   # draw all objects in the efficiency pad
    pad3.cd()
    efficiency.GetXaxis().SetLimits(xmin,xmax)
    efficiency.Draw()
    p3legend.Draw('same')

    # save the plot
    c1.Update()
    outfilepath = os.path.splitext(outfilepath)[0]
    for outfmt in outfmts: c1.SaveAs(outfilepath+outfmt)
   
    # close the canvas
    # (appears to be needed when generating a large number of plots)
    c1.Close()
