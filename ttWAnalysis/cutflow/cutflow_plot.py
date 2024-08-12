############################################################
# make plots of the histograms filled with cutflow_fill.py #
############################################################

import sys
import os
import ROOT
import argparse
sys.path.append('../../Tools/python')
import histtools as ht
sys.path.append('../../plotting/python')
import singlehistplotter as psh
import multihistplotter as pmh

### help functions for creating alternative views of the cutflow histogram
# note: the input histogram for each of these functions is the output of cutflow_fill.py.
#       (in each bin, absolute number of events that fail a given cut,
#        last bin: number of passing events)

def make_cumulative_absolute( hist ):
    ### output histograms: in each bin, total amount of remaining events after each cut

    # copy original histogram skeleton
    # (but with one less bin, skip last one)
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    chist = ROOT.TH1D( "", title, nbins-1, 0.5, nbins-0.5 )
    # copy first two bins (hCounter and nEntries)
    chist.SetBinContent(1, hist.GetBinContent(1))
    chist.SetBinError(1,0)
    chist.GetXaxis().SetBinLabel(1, 'Full sample')
    chist.SetBinContent(2, hist.GetBinContent(2))
    chist.SetBinError(2,0)
    chist.GetXaxis().SetBinLabel(2, 'Pass 2 loose leptons') # hardcoded label
    #chist.GetXaxis().SetBinLabel(2, 'Particle level selection') # hardcoded label
    nevents = hist.GetBinContent(2)
    # fill further bins
    for i in range(3, chist.GetNbinsX()+1):
        nevents = nevents - hist.GetBinContent(i)
        chist.SetBinContent(i, nevents)
        chist.SetBinError(i,0)
        chist.GetXaxis().SetBinLabel(i, hist.GetXaxis().GetBinLabel(i).replace('Fail', 'Pass'))
    # check that number of remaining events corresponds to number of passing events
    if( nevents!=hist.GetBinContent(nbins) ):
        msg = 'ERROR: histogram does not seem filled as expected.'
        raise Exception(msg)
    return chist

def make_cumulative_relative( hist ):
    ### output histogram: same as make_cumulative_absolute_pass but with y-axis in per cent
    chist = make_cumulative_absolute( hist ).Clone()
    scale = 1./chist.GetBinContent(1)
    chist.Scale(scale)
    chist.SetMaximum(1.2)
    return chist

def make_relative_fail( hist ):
    ### output histogram: in each bin, relative fraction of events that fail given cut
    #                          (relative w.r.t. number of events that pass all previous cuts!)
    # copy original histogram skeleton
    # (but with first and last bin removed)
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    rhist = ROOT.TH1D( "", title, nbins-2, 0.5, nbins-1.5 )
    # fill first bin
    rhist.SetBinContent(1, 1-hist.GetBinContent(2)/hist.GetBinContent(1))
    rhist.SetBinError(1,0)
    rhist.GetXaxis().SetBinLabel(1, 'Fail 2 loose leptons') # hardcoded label
    #rhist.GetXaxis().SetBinLabel(1, 'Fail particle level selection') # hardcoded label
    # fill other bins
    nevents = hist.GetBinContent(2)
    for i in range(2, rhist.GetNbinsX()+1):
        nfail = float(hist.GetBinContent(i+1))
        rhist.SetBinContent(i, nfail/nevents)
        rhist.SetBinError(i,0)
        rhist.GetXaxis().SetBinLabel(i, hist.GetXaxis().GetBinLabel(i+1))
        nevents -= nfail
    rhist.SetMaximum(1.2)
    return rhist

def make_relative_pass( hist ):
    ### output histogram: same as make_relative_fail but with passing fraction instead of failing
    rhist = make_relative_fail( hist ).Clone()
    for i in range(1, rhist.GetNbinsX()+1 ):
        rhist.SetBinContent(i, 1-rhist.GetBinContent(i))
        rhist.GetXaxis().SetBinLabel(i, 
            rhist.GetXaxis().GetBinLabel(i).replace('Fail','Pass'))
    rhist.SetMaximum(1.2)
    return rhist
  

if __name__=='__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfiles', type=os.path.abspath, required=True, nargs='+')
    parser.add_argument('-t', '--tags', default=None, nargs='+')
    parser.add_argument('-o', '--outputdir', type=os.path.abspath, required=True)
    parser.add_argument('--extrainfos', default=None)
    args = parser.parse_args()

    # parse arguments
    if not os.path.exists(args.outputdir):
        os.makedirs(args.outputdir)
    dolegend = True
    if args.tags is None:
        args.tags = ['']*len(args.inputfiles)
        dolegend = False

    # make a color list
    colors = ([
      ROOT.kAzure-6,
      ROOT.kAzure-8,
      ROOT.kAzure-4,
      ROOT.kAzure+6,
      ROOT.kViolet,
      ROOT.kMagenta-9
    ])
    colorlist = []
    for i, inputfile in enumerate(args.inputfiles):
        if 'TTW' in os.path.basename(inputfile):
            colorlist.append(ROOT.kRed)
            i -= 1
        else: colorlist.append(colors[i])

    # get the histograms
    rawhists = []
    for inputfile, tag in zip(args.inputfiles, args.tags):
        histlist = ht.loadallhistograms(inputfile)
        if len(histlist)!=1:
            raise Exception('ERROR: unexpected number of histograms found: {}'.format(len(histlist)))
        hist = histlist[0]

        # print the histogram to get numerical values
        #ht.printhistogram(hist)

        # modify the angle of the labels
        #for i in range(1,hist.GetNbinsX()+1):
        #        label = hist.GetXaxis().GetBinLabel(i)
        #        hist.GetXaxis().ChangeLabel(i,45,-1,-1,-1,-1,label)
        # does not seem to work in combination with SetBinLabel (only defined for numeric axis)...
        # workaround:
        hist.GetXaxis().LabelsOption('v')

        # the histogram is assumed to be filled as follows:
        # bin 1 = number of entries in hCounter (i.e. full sample before selections)
        # bin 2 = number of entries in sample (might be after skimming)
        # bin 3 - ... = number of events failing a cut or passing all cuts
        # here, we check that the sum of bins 3 - ... is equal to bin 2.
        nentries = 0
        for i in range(3, hist.GetNbinsX()+1): nentries += hist.GetBinContent(i)
        if nentries!=hist.GetBinContent(2):
            msg = 'ERROR: histogram does not seem to be filled as expected.'
            raise Exception(msg)

        # add histogram to the list
        rawhists.append(hist.Clone())

    # common plot settings
    extracmstext = 'Private work'
    extrainfos = ''
    if args.extrainfos is not None: extrainfos = args.extrainfos.split(',')
    
    # make raw plot
    xaxtitle = ''
    yaxtitle = 'Number of events'
    drawoptions = 'hist e'
    outputfile = os.path.join(args.outputdir,'raw.png')
    if len(rawhists)>1:
        pmh.plotmultihistograms(rawhists, figname=outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            dolegend=dolegend, labellist=args.tags, colorlist=colorlist,
            lumitext='', extracmstext=extracmstext,
            drawoptions=drawoptions,
            extrainfos=extrainfos,
            p1bottommargin=0.45)
    else:
        psh.plotsinglehistogram(rawhists[0], outputfile, title=None, 
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            label=None, color=None,
            drawoptions=drawoptions,
            lumitext='', extracmstext=extracmstext,
            extrainfos=extrainfos,
            bottommargin=0.45)

    # make cumulative absolute plot
    cumabshists = [make_cumulative_absolute( hist ) for hist in rawhists]
    for hist in cumabshists: hist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Number of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(args.outputdir,'cumulative_absolute.png')
    if len(cumabshists)>1:
        pmh.plotmultihistograms(cumabshists, figname=outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            dolegend=dolegend, labellist=args.tags, colorlist=colorlist,
            lumitext='', extracmstext=extracmstext,
            logy=True,
            drawoptions=drawoptions,
            extrainfos=extrainfos,
            p1bottommargin=0.45)
    else:
        psh.plotsinglehistogram(cumabshists[0], outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            label=None, color=None,
            logy=True,
            drawoptions=drawoptions,
            lumitext='', extracmstext=extracmstext,
            extrainfos=extrainfos,
            bottommargin=0.45)

    # make cumulative relative plot
    cumrelhists = [make_cumulative_relative( hist ) for hist in rawhists]
    for hist in cumrelhists: hist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(args.outputdir,'cumulative_relative.png')
    if len(cumrelhists)>1:
        pmh.plotmultihistograms(cumrelhists, figname=outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            dolegend=dolegend, labellist=args.tags, colorlist=colorlist,
            lumitext='', extracmstext=extracmstext,
            logy=True, ylims=(1e-5,500),
            drawoptions=drawoptions,
            extrainfos=extrainfos,
            p1bottommargin=0.45)
    else:
        psh.plotsinglehistogram(cumrelhists[0], outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            label=None, color=None,
            logy=True, yaxmin=1e-2, yaxmax=5,
            drawoptions=drawoptions,
            lumitext='', do_cms_text=True, extracmstext=extracmstext,
            extrainfos=extrainfos, infoleft=0.7, infotop=0.95,
            bottommargin=0.45,
            writebincontent=True, bincontentfmt='{:.1e}')

    # make relative fail plot
    relfailhists = [make_relative_fail( hist ) for hist in rawhists]
    for hist in relfailhists: hist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of failing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(args.outputdir,'relative_fail.png')
    if len(relfailhists)>1:
        pmh.plotmultihistograms(relfailhists, figname=outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            dolegend=dolegend, labellist=args.tags, colorlist=colorlist,
            lumitext='', extracmstext=extracmstext,
            logy=False, ylims=None,
            drawoptions=drawoptions,
            extrainfos=extrainfos,
            p1bottommargin=0.45)
    else:
        psh.plotsinglehistogram(relfailhists[0], outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            label=None, color=None, logy=False, drawoptions=drawoptions,
            lumitext='', extracmstext=extracmstext,
            extrainfos=extrainfos,
            bottommargin=0.45,
            writebincontent=True)

    # make relative pass plot
    relpasshists = [make_relative_pass( hist ) for hist in rawhists]
    for hist in relpasshists: hist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of passing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(args.outputdir,'relative_pass.png')
    if len(relpasshists)>1:
        pmh.plotmultihistograms(relpasshists, figname=outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            dolegend=dolegend, labellist=args.tags, colorlist=colorlist,
            lumitext='', extracmstext=extracmstext,
            logy=False, ylims=None,
            drawoptions=drawoptions,
            extrainfos=extrainfos,
            p1bottommargin=0.45)
    else:
        psh.plotsinglehistogram(relpasshists[0], outputfile, title=None,
            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
            label=None, color=None, logy=False, drawoptions=drawoptions,
            lumitext='', extracmstext=extracmstext,
            extrainfos=extrainfos,
            bottommargin=0.45,
            writebincontent=True)
