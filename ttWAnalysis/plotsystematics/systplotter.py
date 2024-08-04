############################################################
# Tools for plotting histograms with systematic variations #
############################################################

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../../plotting/python'))
import plottools as pt
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
sys.path.append(os.path.abspath('../plotting'))
from colors import getcolormap

def findbytitle(histlist, title):
    # find a histogram by its title, error if not found
    indices = []
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: indices.append(i)
    if len(indices)!=1:
        msg = 'ERROR in findbytitle:'
        msg += ' found {} matching histograms'.format(len(indices))
        msg += ' while 1 was expected.'
        raise Exception(msg)
    return indices[0]

def findbyname(histlist, tag):
    # find a histogram by its name, error if not found
    indices = []
    for i,hist in enumerate(histlist):
        if tag in hist.GetName(): indices.append(i)
    if len(indices)!=1:
        msg = 'ERROR in findbyname:'
        msg += ' found {} matching histograms'.format(len(indices))
        msg += ' for tag {}'.format(tag)
        msg += ' while 1 was expected'
        msg += ' ({})'.format([histlist[i].GetName() for i in indices])
        raise Exception(msg)
    return indices[0]

def sethiststyle(hist, systematic, colormap):
    # set color and line properties of a histogram
    hist.SetLineWidth(3)
    sysname = systematic
    if('Up' in systematic):
        hist.SetLineStyle(0)
        sysname = systematic.replace('Up','')
    elif('Down' in systematic):
        hist.SetLineStyle(2)
        sysname = systematic.replace('Down','')
    elif('ShapeVar' in systematic):
        sysname = systematic[:systematic.find('ShapeVar')+8]
    if('JECAll' in sysname): sysname = 'JECAll'
    if('JECGrouped' in sysname): sysname = 'JECGrouped'
    hist.SetLineColor( colormap.get(sysname,ROOT.kBlack) )
    
def getminmax(histlist, witherrors=False):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 0.
    totmin = 1.
    for hist in histlist:
        for i in range(1,hist.GetNbinsX()+1):
            val = hist.GetBinContent(i)
            upval = val
            downval = val
            if witherrors:
                upval = val + hist.GetBinError(i)
                downval = val - hist.GetBinError(i)
            if upval > totmax: totmax = upval
            if downval < totmin: totmin = downval
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    return (totmin-bottommargin,totmax+topmargin)

def histlisttotxt(histlist,txtfile):
    txtfile = txtfile.split('.')[-1]+'.txt'
    with open(txtfile,'w') as txtf:
        for hist in histlist:
            toprint = '{:<15}'.format(hist.GetTitle())
            for i in range(1,hist.GetNbinsX()+1):
                toprint += '\t{:<5}'.format('{0:.3f}'.format(hist.GetBinContent(i)))
                toprint += '\n'
            txtf.write(toprint)

def plotsystematics( nominalhist, syshistlist, figname,
                     colormap=None,
                     xaxtitle=None, xaxtitlesize=None, xaxtitleoffset=None,
                     yaxtitle=None, yaxtitlesize=None, yaxtitleoffset=None,
		     staterrors=False, style='absolute',
                     yaxrange=None, doclip=True,
                     remove_duplicate_labels=False, remove_down_labels=False,changeNominal=False,
                     extrainfos=[], infosize=None, infoleft=None, infotop=None,
		     outtxtfile='' ):
    # input arguments:
    # - nominalhist: nominal histogram
    # - syshistlist: list of histograms with systematic variations
    #   note: the title of each histogram is used for color and styling;
    #         they are expected to be formatted as '[name of systematic][Up or Down]'
    # - staterrors: whether to plot statistical errors
    # - style: choose from 'absolute', 'normalized' or 'relative'
    
    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 800 # width of canvas
    # fonts and sizes:
    labelfont = 4; labelsize = 22
    axtitlefont = 4
    if xaxtitlesize is None: xaxtitlesize = 22
    if yaxtitlesize is None: yaxtitlesize = 22
    infofont = 4
    if infosize is None: infosize = 20
    # title offset
    if yaxtitleoffset is None: yaxtitleoffset = 2
    if xaxtitleoffset is None: xaxtitleoffset = 1
    # margins:
    topmargin = 0.05
    bottommargin = 0.1
    leftmargin = 0.15
    rightmargin = 0.25
    # legend box
    plegendbox = [1-rightmargin+0.03,bottommargin+0.03,1-0.03,1-topmargin-0.03]
    # extra info box parameters
    if infoleft is None: infoleft = leftmargin+0.05
    if infotop is None: infotop = 1-topmargin-0.1
    # colormap
    if colormap is None: colormap = getcolormap('systematics')

    # copy nominal histogram to plot statistical uncertainties
    # using the bin errors (if requested)
    stathist = nominalhist.Clone()

    # clipping and style setting
    # nominal histogram
    if changeNominal: sethiststyle(nominalhist, 'nominal', colormap)
    else: sethiststyle(nominalhist, nominalhist.GetTitle(), colormap)
    if doclip: ht.cliphistogram(nominalhist)
    # statistical histogram
    stathist.SetFillColorAlpha(ROOT.kBlue-7, 0.2)
    if doclip: ht.cliphistogram(stathist)
    # systematic histograms
    for hist in syshistlist:
        sethiststyle(hist, hist.GetTitle(), colormap)
        if doclip: ht.cliphistogram(hist)

    # modify histograms for plot style 'normalized'
    # (divide everything by bin content of nominalhist,
    #  so nominal is at the unit line and systematics are relative to 1.)
    if style=='normalized':
        for hist in syshistlist+[stathist]+[nominalhist]:
            for i in range(0,hist.GetNbinsX()+2):
                if nominalhist.GetBinContent(i)==0:
                    hist.SetBinContent(i,1.)
                    hist.SetBinError(i,0.)
                else:
                    hist.SetBinError(i,hist.GetBinError(i)/nominalhist.GetBinContent(i))
                    hist.SetBinContent(i,hist.GetBinContent(i)/nominalhist.GetBinContent(i))

    # modify histograms for plot style 'relative'
    # (systematics are in absolute value and relative to 0.)
    elif style=='relative':
        for hist in syshistlist+[stathist]+[nominalhist]:
            for i in range(0,hist.GetNbinsX()+2):
                if nominalhist.GetBinContent(i)==0:
                    hist.SetBinContent(i,0.)
                    hist.SetBinError(i,0.)
                else:
                    hist.SetBinError(i,hist.GetBinError(i)/nominalhist.GetBinContent(i))
                    hist.SetBinContent(i,abs(hist.GetBinContent(i)/nominalhist.GetBinContent(i)-1))
        # also replace each pair of up/down by a single bin-per-bin maximum
        newsyshistlist = []
        for uphist in syshistlist:
            # skip down histograms
            if uphist.GetName().endswith('Down'): continue
            # add variations that are neither up or down
            if not uphist.GetName().endswith('Up'):
                newsyshistlist.append(uphist)
                continue
            # for up histograms, find corresponding down histogram and do bin per bin max
            downhist = syshistlist[findbyname(syshistlist, uphist.GetName().replace('Up', 'Down'))]
            maxhist = ht.binperbinmaxvar([uphist,downhist], nominalhist)
            # reset name, title and style
            maxhist.SetTitle(uphist.GetTitle())
            maxhist.SetName(uphist.GetName())
            sethiststyle(maxhist, maxhist.GetTitle(), colormap)
            newsyshistlist.append(maxhist)
        syshistlist = newsyshistlist

    elif style=='absolute': pass
    else:
        msg = 'ERROR: style {} not recognized.'.format(style)
        raise Exception(msg)
    
    ### make legend and add all histograms
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetFillStyle(0)
    nentries = 0
    allJECHasLabel = False
    groupedJECHasLabel = False
    unique_labels = []
    for hist in syshistlist:
        label = hist.GetTitle()
	# avoid drawing a legend entry for all shape variations
        if('ShapeVar0' in label): label = label[:label.find('Var0')]
        elif('ShapeVar' in label): continue
	# avoid drawing a legend entry for all JEC variations
        if('JECAll' in label):
            if not allJECHasLabel: 
                label = 'JECAll'
                allJECHasLabel = True
            else: continue
        if('JECGrouped' in label):
            if not groupedJECHasLabel:
                label = 'JECGrouped'
                groupedJECHasLabel = True
            else: continue
        # avoid drawing duplicate labels if requested
        if( remove_duplicate_labels and label in unique_labels ): continue
        else: unique_labels.append(label)
        # modify down label for better readability
        if label.endswith('Down'): label = '~Down'
        # remove down labels if requested
        if remove_down_labels:
          if label == '~Down': continue
          if label.endswith('Up'): label = label[:-2]
        legend.AddEntry(hist,label,"l")
        nentries += 1
    
    if changeNominal: legend.AddEntry(nominalhist, 'nominal', "l")
    else: legend.AddEntry(nominalhist,nominalhist.GetTitle(), "l")
    legend.SetNColumns(1)

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetTopMargin(topmargin)
    pad1.SetBottomMargin(bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetFrameLineWidth(2)
    pad1.SetGridx(1)
    pad1.SetGridy(1)
    pad1.Draw()
    
    ### make upper part of the plot
    pad1.cd()
    # determine plot range based on minimum and maximum variation
    (rangemin,rangemax) = getminmax(syshistlist)
    # if drawing error bars, also take statistical variation into account
    if staterrors: (srangemin,srangemax) = getminmax([stathist], witherrors=True)
    if style=='absolute': rangemin = 0.
    if style=='relative': rangemin = 0.
    if yaxrange is not None:
        rangemin = yaxrange[0]
        rangemax = yaxrange[1]
    nominalhist.SetMinimum(rangemin)
    nominalhist.SetMaximum(rangemax)
    stathist.SetMinimum(rangemin)
    stathist.SetMaximum(rangemax)
    for hist in syshistlist:
        hist.SetMinimum(rangemin)
        hist.SetMaximum(rangemax)

    # X-axis layout
    xax = nominalhist.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    if xaxtitle is not None:
        xax.SetTitle(xaxtitle)
        xax.SetTitleFont(10*axtitlefont+3)
        xax.SetTitleSize(xaxtitlesize)
        xax.SetTitleOffset(xaxtitleoffset)
    # Y-axis layout
    yax = nominalhist.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    if yaxtitle is not None:
        yax.SetTitle(yaxtitle)
        yax.SetTitleFont(10*axtitlefont+3)
        yax.SetTitleSize(yaxtitlesize)
        yax.SetTitleOffset(yaxtitleoffset)

    # histograms
    nominalhist.Draw("hist")
    if staterrors: stathist.Draw("e2 same")
    for hist in syshistlist:
    	# draw ShapeVar histograms first to put them in background
        if not 'ShapeVar' in hist.GetName(): continue
        hist.Draw("hist same")
    for hist in syshistlist:
	# now draw all other histograms
        if 'ShapeVar' in hist.GetName(): continue
        hist.Draw("hist same")
    # redraw nominal to put it on top
    nominalhist.Draw("hist same")
    legend.SetFillColor(ROOT.kWhite)
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    pt.drawLumi(pad1,lumitext="Simulation")

    # draw extra info
    tinfo = ROOT.TLatex()
    tinfo.SetTextFont(10*infofont+3)
    tinfo.SetTextSize(infosize)
    for i,info in enumerate(extrainfos):
        vspace = 0.05*(float(infosize)/20)
        tinfo.DrawLatexNDC(infoleft,infotop-(i+1)*vspace, info)

    ### save the plot
    figname = os.path.splitext(figname)[0]
    c1.SaveAs(figname+'.png')
    c1.SaveAs(figname+'.pdf')
    ### save txt files with values if requested
    if len(outtxtfile)>0: histlisttotxt([nominalhist]+syshistlist,outtxtfile)
    return
