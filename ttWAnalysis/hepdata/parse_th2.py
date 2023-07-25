######################################################################
# some utility functions to parse a TH2D with non-numeric bin labels #
######################################################################
# COPIED FROM TZQ
# use case: covariance matrix in fitDiagnostics output file
#           contains the covariance between all nuisance parameters,
#           but when loaded via standard hepdata_lib methods, 
#           the bins are numbered as integers instead of with the parameter name

import sys
import hepdata_lib
from hepdata_lib import RootFileReader
import ROOT

def read_hist_2d_labeled( filepath, th2path ):

    # read the histogram via standard hepdata_lib methods
    reader = RootFileReader( filepath )
    hist = reader.read_hist_2d( th2path )

    # read the same histogram via ROOT
    f = ROOT.TFile.Open(filepath,'read')
    roothist = f.Get( th2path )
    roothist.SetDirectory(0)
    f.Close()

    # get the bin labels
    xbinlabels = []
    for i in range(1,roothist.GetNbinsX()+1):
        xbinlabels.append( roothist.GetXaxis().GetBinLabel(i) )
    ybinlabels = []
    for i in range(1,roothist.GetNbinsY()+1):
        ybinlabels.append( roothist.GetYaxis().GetBinLabel(i) )

    # temp for testing
    #xbinlabels = xbinlabels[:5]
    #ybinlabels = ybinlabels[:5]
    #print(xbinlabels)
    #print(ybinlabels)

    # reshape the bin labels to correct format
    nxbins = len(xbinlabels)
    nybins = len(ybinlabels)
    ybinlabels = ybinlabels*nxbins
    xbinlabels = [xbl for xbl in xbinlabels for i in range(nybins)]
    #print(xbinlabels)
    #print(ybinlabels)

    # replace the bin labels in the hepdata_lib hist object
    hist['x'] = xbinlabels
    hist['y'] = ybinlabels

    return hist

if __name__=='__main__':

    ### testing section
    
    filepath = sys.argv[1]
    th2path = sys.argv[2]
    hist = read_hist_2d_labeled( filepath, th2path )

    # check consistency between root histogram and hepdata_lib histogram
    binx = 16
    biny = 10
    f = ROOT.TFile.Open(filepath,'read')
    roothist = f.Get( th2path )
    roothist.SetDirectory(0)
    f.Close()
    nxbins = roothist.GetNbinsX()
    nybins = roothist.GetNbinsY()
    print('ROOT:')
    print('x-label: {}'.format(roothist.GetXaxis().GetBinLabel(binx)))
    print('y-label: {}'.format(roothist.GetYaxis().GetBinLabel(biny)))
    print('content: {}'.format(roothist.GetBinContent(binx,biny)))
    idx = nybins*(binx-1) + biny-1
    print('hebdata_lib:')
    print('x-label: {}'.format(hist['x'][idx]))
    print('y-label: {}'.format(hist['y'][idx]))
    print('content: {}'.format(hist['z'][idx]))
