########################################################################
# some small tools for working with histograms and lists of histograms #
########################################################################

import os
import math
import numpy as np
from array import array
import ROOT
import listtools as lt


### histogram reading and loading ###

def loadallhistograms(histfile, allow_tgraphs=False, suppress_warnings=False):
    ### read a root file containing histograms and load all histograms to a list
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        # check the object type
        ishist = ( isinstance(hist,ROOT.TH1)
                   or isinstance(hist,ROOT.TH2) )
        isgraph = ( isinstance(hist,ROOT.TGraph) )
	if not suppress_warnings:
	    if( not allow_tgraphs and not ishist ):
		print('WARNING in histtools.loadallhistograms:'
                      +' key "'+str(key.GetName())+'" is not a valid histogram.')
                continue
            if( allow_tgraphs and not (ishist or isgraph) ):
		print('WARNING in histtools.loadallhistograms:'
                      +' key "'+str(key.GetName())+'" is not a valid histogram or graph.')
                continue
	hist.SetName(key.GetName())
	if ishist: hist.SetDirectory(ROOT.gROOT)
	histlist.append(hist)
    f.Close()
    return histlist

def loadhistograms(histfile, 
                   mustcontainall=[], mustcontainone=[],
	           maynotcontainall=[],maynotcontainone=[],
                   allow_tgraphs=False, suppress_warnings=False ):
    ### read a root file containing histograms and load histograms to a list.
    # note: selection already included at this stage
    #       (instead of loading all and then selecting with methods below)
    #       (useful in case of many histograms of which only a few are needed)
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
	if not lt.subselect_string(key.GetName(),
	    mustcontainone=mustcontainone,mustcontainall=mustcontainall,
	    maynotcontainone=maynotcontainone,maynotcontainall=maynotcontainall): continue
        hist = f.Get(key.GetName())
	# check the object type
        ishist = ( isinstance(hist,ROOT.TH1)
                   or isinstance(hist,ROOT.TH2) )
        isgraph = ( isinstance(hist,ROOT.TGraph) )
        if not suppress_warnings:
            if( not allow_tgraphs and not ishist ):
                print('WARNING in histtools.loadhistograms:'
                      +' key "'+str(key.GetName())+'" is not a valid histogram.')
                continue
            if( allow_tgraphs and not (ishist or isgraph) ):
                print('WARNING in histtools.loadhistograms:'
                      +' key "'+str(key.GetName())+'" is not a valid histogram or graph.')
                continue
        hist.SetName(key.GetName())
        if ishist: hist.SetDirectory(ROOT.gROOT)
        histlist.append(hist)
    f.Close()
    return histlist

def loadhistogramlist(histfile, histnames, 
		      allow_tgraphs=False, suppress_warnings=False):
    ### load histograms specified by name from a file
    f = ROOT.TFile.Open(histfile)
    histlist = []
    for histname in histnames:
        hist = f.Get(histname)
        # check the object type
        ishist = ( isinstance(hist,ROOT.TH1)
                   or isinstance(hist,ROOT.TH2) )
        isgraph = ( isinstance(hist,ROOT.TGraph) )
        if not suppress_warnings:
            if( not allow_tgraphs and not ishist ):
                print('WARNING in histtools.loadhistograms:'
                      +' key "'+str(histname.GetName())+'" is not a valid histogram.')
                continue
            if( allow_tgraphs and not (ishist or isgraph) ):
                print('WARNING in histtools.loadhistograms:'
                      +' key "'+str(histname.GetName())+'" is not a valid histogram or graph.')
                continue
        hist.SetName(histname)
        if ishist: hist.SetDirectory(ROOT.gROOT)
        histlist.append(hist)
    f.Close()
    return histlist

def loadallhistnames(histfile):
    ### read a root file containing histograms and make a list of histogram names.
    # note: objects are not loaded (for speed), only a list of names is retrieved.
    f = ROOT.TFile.Open(histfile)
    histnames = []
    keylist = f.GetListOfKeys()
    for key in keylist: histnames.append(key.GetName())
    f.Close()
    return histnames

def loadhistnames(histfile,
                  mustcontainall=[], mustcontainone=[],
                  maynotcontainall=[],maynotcontainone=[]):
    ### read a root file containing histograms and make a list of histogram names.
    # note: objects are not loaded (for speed), only a list of names is retrieved.
    f = ROOT.TFile.Open(histfile)
    histnames = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        if not lt.subselect_string(key.GetName(),
            mustcontainone=mustcontainone,mustcontainall=mustcontainall,
            maynotcontainone=maynotcontainone,maynotcontainall=maynotcontainall): continue
        histnames.append(key.GetName())
    f.Close()
    return histnames


### histogram subselection ###

def selecthistograms(histlist,mustcontainone=[],mustcontainall=[],
		    maynotcontainone=[],maynotcontainall=[]):
    idlist = [hist.GetName() for hist in histlist]
    (indlist,selhistlist) = lt.subselect_objects(histlist,idlist,
	mustcontainone=mustcontainone,mustcontainall=mustcontainall,
	maynotcontainone=maynotcontainone,maynotcontainall=maynotcontainall)
    return (indlist,selhistlist)

def findhistogram(histlist, name):
    ### find a histogram with a given name in a list
    # returns None if no histogram with the requested name is found
    for hist in histlist:
	if hist.GetName()==name: return hist
    return None


### histogram clipping ###

def cliphistogram(hist,clipboundary=0):
    ### clip a histogram to minimum zero
    # also allow a clipboundary different from zero, useful for plotting 
    # (e.g. to ignore artificial small values such as the one at the end of this function)
    for i in range(0,hist.GetNbinsX()+2):
        if hist.GetBinContent(i)<clipboundary:
            hist.SetBinContent(i,0)
            hist.SetBinError(i,0)
    # check if histogram is empty after clipping and if so, fill it with dummy value
    if hist.GetSumOfWeights()<1e-12: hist.SetBinContent(1,1e-6)

def cliphistograms(histlist,clipboundary=0):
    ### apply cliphistogram on all histograms in a list
    for hist in histlist: cliphistogram(hist,clipboundary=clipboundary)

def clipallhistograms(histfile,mustcontainall=[],clipboundary=0):
    ### apply cliphistogram on all histograms in a file
    histlist = loadallhistograms(histfile)
    if len(mustcontainall)==0:
	cliphistograms(histlist,clipboundary=clipboundary)
    else:
	(indlist,_) = selecthistograms(histlist,mustcontainall=mustcontainall)
	for index in indlist: cliphistogram(histlist[index],clipboundary=clipboundary)
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
        hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+histfile)


### take absolute value ###

def absolute(hist):
    ### take absolute value of each bin
    for i in range(0,hist.GetNbinsX()+2):
        if hist.GetBinContent(i)<0:
            hist.SetBinContent(i,-hist.GetBinContent(i))


### finding minimum and maximum ###

def getminmax(histlist, includebinerror=False):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 0.
    totmin = 1e12
    for hist in histlist:
        for i in range(1,hist.GetNbinsX()+1):
            val = hist.GetBinContent(i)
            err = 0.
            if includebinerror: err = hist.GetBinError(i)
            if val+err > totmax: totmax = val+err
            if val-err < totmin: totmin = val-err
    return (totmin,totmax)

def getminmaxmargin(histlist, includebinerror=False, clip=False):
    (totmin,totmax) = getminmax(histlist, includebinerror=includebinerror)
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    minv = totmin-bottommargin
    maxv = totmax+topmargin
    if( clip and minv<0 ): minv = 0
    return (minv,maxv)


### histogram conversion ###

def histtoarray( hist ):
    ### get numpy array with bin contents (bin errors are ignored)
    nbins = hist.GetNbinsX()
    res = np.zeros( nbins+2 )
    for i in range(0, nbins+2):
	res[i] = hist.GetBinContent(i)
    return res

def histtoarray2d( hist, keepouterflow=True ):
    ### same as above but for 2D histogram
    nxbins = hist.GetNbinsX()
    nybins = hist.GetNbinsY()
    if keepouterflow:
	res = np.zeros( (nxbins+2, nybins+2) )
	for i in range(0, nxbins+2):
	    for j in range(0, nybins+2):
		res[i,j] = hist.GetBinContent(i,j)
    else:
	res = np.zeros( (nxbins, nybins) )
        for i in range(1, nxbins+1):
            for j in range(1, nybins+1):
                res[i-1,j-1] = hist.GetBinContent(i,j)
    return res

def tgraphtohist( graph ):

    # get list of x values and sort them
    xvals = []
    for i in range(graph.GetN()): xvals.append(graph.GetX()[i])
    xvals = np.array(xvals)
    sortedindices = np.argsort(xvals)
    # make bins
    bins = []
    for i in sortedindices: bins.append(graph.GetX()[i]-graph.GetErrorXlow(i))
    bins.append(graph.GetX()[i]+graph.GetErrorXhigh(i))
    # make histogram
    hist = ROOT.TH1D("","",len(bins)-1,array('f',bins))
    # set bin contents
    for i in range(1,hist.GetNbinsX()+1):
        bincontent = graph.GetY()[sortedindices[i-1]]
        binerror = max(graph.GetErrorYlow(sortedindices[i-1]),
                        graph.GetErrorYhigh(sortedindices[i-1]))
        hist.SetBinContent(i,bincontent)
        hist.SetBinError(i,binerror)
    hist.SetName(graph.GetName())
    hist.SetTitle(graph.GetTitle())
    return hist


### histogram calculations ###

def binperbinmaxvar( histlist, nominalhist ):
    ### get the bin-per-bin maximum variation (in absolute value) of histograms in histlist 
    ### wrt nominalhist.
    maxhist = nominalhist.Clone()
    maxhist.Reset()
    nbins = maxhist.GetNbinsX()
    for i in range(0,nbins+2):
	nomval = nominalhist.GetBinContent(i)
	varvals = np.zeros(len(histlist))
	for j in range(len(histlist)):
	    varvals[j] = abs(histlist[j].GetBinContent(i)-nomval)
	maxhist.SetBinContent(i,np.amax(varvals))
    return maxhist

def envelope( histlist, returntype='tuple' ):
    ### return two histograms that form the envelope of all histograms in histlist.
    # arguments:
    # - returntype: if 'tuple', returns tuple of two histograms (lower bound and upper bound);
    #               if 'hist', returns a single histogram with same lower and upper bounds
    #               (and bin contents chosen symmetrically between them).
    if( len(histlist)<2 ):
	msg = 'ERROR in histtools.envelope: at least two histograms required.'
        raise Exception(msg)
    nbins = histlist[0].GetNbinsX()
    minhist = histlist[0].Clone()
    maxhist = histlist[0].Clone()
    for hist in histlist:
        if( hist.GetNbinsX()!=nbins ):
	    msg = 'ERROR in histtools.envelope: '
	    msg += ' provided histograms have different number of bins.'
            raise Exception(msg)
        for i in range(0,nbins+2):
	    bincontent = hist.GetBinContent(i)
	    if bincontent < minhist.GetBinContent(i):
              minhist.SetBinContent(i, bincontent)
            if bincontent > maxhist.GetBinContent(i):
              maxhist.SetBinContent(i, bincontent)
    for i in range(0,nbins+2):
        minhist.SetBinError(i,0)
        maxhist.SetBinError(i,0)
    if returntype=='tuple': return (minhist,maxhist)
    elif returntype=='hist':
	res = minhist.Clone()
	res.Reset()
	for i in range(0,nbins+2):
	    minc = minhist.GetBinContent(i)
	    maxc = maxhist.GetBinContent(i)
	    binc = (maxc+minc)/float(2)
	    err = (maxc-minc)/float(2)
	    res.SetBinContent(i, binc)
	    res.SetBinError(i, err)
	return res
    else:
	msg = 'ERROR in histtools.envelope:'
	msg += ' return type {} not recognized.'.format(returntype)
	raise Exception(msg)
    
def rootsumsquare( histlist ):
    ### return a histogram that is the root-sum-square of all histograms in histlist.
    # check the input list
    if( len(histlist)<1 ):
	print('### ERROR ###: at least one histogram required for rootsumsquare')
	return None
    res = histlist[0].Clone()
    res.Reset()
    nbins = res.GetNbinsX()
    bincontents = np.zeros(nbins+2)
    for hist in histlist:
	if( hist.GetNbinsX()!=nbins ):
	    print('### ERROR ###: histograms are not compatible for summing in quadrature')
	    return None
	thisbincontents = np.zeros(nbins+2)
	for i in range(0,nbins+2): thisbincontents[i] = hist.GetBinContent(i)
	bincontents += np.power(thisbincontents,2)
    bincontents = np.sqrt(bincontents)
    for i in range(0,nbins+2):
	res.SetBinContent(i,bincontents[i])
    return res


### printing ###

def printhistogram(hist,naninfo=False,returnstr=False):

    infostr = '### {} ###\n'.format(hist.GetName())
    for i in range(0,hist.GetNbinsX()+2):
        bininfo = '  -----------\n'
        bininfo += '  bin: {} -> {}\n'.format(hist.GetBinLowEdge(i),
                                            hist.GetBinLowEdge(i)+hist.GetBinWidth(i))
        bininfo += '  content: {}\n'.format(hist.GetBinContent(i))
	if naninfo:
	    bininfo += '    (isnan: {})\n'.format(math.isnan(hist.GetBinContent(i)))
	    bininfo += '    (isinf: {})\n'.format(math.isinf(hist.GetBinContent(i)))
        bininfo += '  error: {}\n'.format(hist.GetBinError(i))
	infostr += bininfo
    if returnstr: return infostr
    else: print(infostr)
    
def printhistograms( histfile, mustcontainall=[], mustcontainone=[],
		maynotcontainall=[], maynotcontainone=[],
		naninfo=False ):
    allhists = loadallhistograms(histfile)
    selhists = selecthistograms(allhists,mustcontainone=mustcontainone,
		    mustcontainall=mustcontainall,
		    maynotcontainone=maynotcontainone,
		    maynotcontainall=maynotcontainall)[1]
    for hist in selhists: printhistogram(hist,naninfo=naninfo)
