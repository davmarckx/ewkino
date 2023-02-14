####################################################
# special tools for dealing with response matrices #
####################################################

import sys
import os
import ROOT
import numpy as np

def normalize_columns( hist, include_outerflow=False ):
  ### normalize the columns of a TH2D (the y-direction)
  # (i.e. make sure that the sum of each column is 1)
  res = hist.Clone()
  nbins = hist.GetNbinsY()
  for i in range(0, nbins+2):
    colsum = 0
    beginindex = 1
    endindex = nbins+2
    if include_outerflow:
      beginindex = 0
      endindex = nbins+1
    for j in range(beginindex, endindex):
      colsum += hist.GetBinContent(i,j)
    if abs(colsum)<1e-12: continue
    for j in range(beginindex, endindex):
      res.SetBinContent(i,j,hist.GetBinContent(i,j)/colsum)
      res.SetBinError(i,j,hist.GetBinError(i,j)/colsum)
  return res

def normalize_rows( hist, include_outerflow=False ):
  ### normalize the rows of a TH2D (the x-direction)
  # (i.e. make sure that the sum of each row is 1)
  res = hist.Clone()
  nbins = hist.GetNbinsX()
  for j in range(0, nbins+2):
    rowsum = 0
    beginindex = 1
    endindex = nbins+2
    if include_outerflow:
      beginindex = 0
      endindex = nbins+1
    for i in range(beginindex, endindex):
      rowsum += hist.GetBinContent(i,j)
    if abs(rowsum)<1e-12: continue
    for i in range(beginindex, endindex):
      res.SetBinContent(i,j,hist.GetBinContent(i,j)/rowsum)
      res.SetBinError(i,j,hist.GetBinError(i,j)/rowsum)
  return res

def get_stability( hist, include_outerflow=False ):
  ### calculate the stability,
  # i.e. the fraction of events per gen bin to end up in the corresponding reco bin
  # note: this assumes gen level is on the x-axis
  #       and reco level is on the y-axis
  colnormhist = normalize_columns( hist, include_outerflow=include_outerflow )
  nbins = hist.GetNbinsX()
  stability = hist.ProjectionX('stability')
  stability.Reset()
  for i in range(0, nbins+2):
    stability.SetBinContent(i, colnormhist.GetBinContent(i,i))
    stability.SetBinError(i, colnormhist.GetBinError(i,i))
  return stability

def get_purity( hist, include_outerflow=False ):
  ### calculate the purity,
  # i.e. the fraction of events per reco bin to end up in the corresponding gen bin
  # note: this assumes gen level is on the x-axis
  #       and reco level is on the y-axis
  rownormhist = normalize_rows( hist, include_outerflow=include_outerflow )
  nbins = hist.GetNbinsX()
  purity = hist.ProjectionX('purity')
  purity.Reset()
  for i in range(0, nbins+2):
    purity.SetBinContent(i, rownormhist.GetBinContent(i,i))
    purity.SetBinError(i, rownormhist.GetBinError(i,i))
  return purity


def get_efficiency( hist):
  ### calculate the efficiency,
  # i.e. the fraction of gen-level events that are reconstructed by the detector
  # note: this assumes gen level is on the x-axis
  #       and reco level is on the y-axis

  # create two 1D histograms
  #rownormhist = normalize_rows( hist, include_outerflow=True )
  nbins = hist.GetNbinsX()
  Passed = hist.ProjectionX('Passed')
  Passed.Reset()
  All = hist.ProjectionX('All')
  All.Reset()
  
  for i in range(0, nbins+2):
    pass_val = 0
    pass_err = 0
    for j in range(1, nbins+1):
        pass_val += hist.GetBinContent(i,j)
        pass_err += hist.GetBinError(i,j)

    all_val = pass_val + hist.GetBinContent(i,0) + hist.GetBinContent(i,nbins+1)
    all_err = pass_err + hist.GetBinError(i,0) + hist.GetBinError(i,nbins+1)
    
    Passed.SetBinContent(i, pass_val)
    Passed.SetBinError(i, pass_err)
    All.SetBinContent(i, all_val)
    All.SetBinError(i, all_err)


  efficiency = ROOT.TGraphAsymmErrors(Passed,All) 
    
  return efficiency



def AddUnderflowBins( hist, binedges):

    Xshape = hist.ProjectionX('shape')
    # get some info of the histogram
    nbins = hist.GetNbinsX()
    Ybinedges = binedges
        
    # get average bin width
    avrg = (Xshape.GetBinLowEdge(nbins+1) - Xshape.GetBinLowEdge(1)) / nbins
    Ybinedges.append(binedges[-1] + avrg)
    
    binedges = np.array(binedges)
    Ybinedges = np.array(Ybinedges)
        
    hist2 = ROOT.TH2F("underflowhist","hist with underflow bins",nbins,binedges ,nbins + 1, Ybinedges)
    
    # fill normal bins
    for i in range(1, nbins + 1):
        for j in range(1, nbins + 1):
            hist2.SetBinContent( hist2.GetBin(i,j), hist.GetBinContent(i,j) )
    # fill underflow bins
    for i in range(1, nbins + 1):
        hist2.SetBinContent( hist2.GetBin(i,nbins+1), hist.GetBinContent(i,0) )
        

    for k in range(1, nbins + 1):
        hist2.GetYaxis().SetBinLabel(k,str(Xshape.GetBinCenter(k)))
    hist2.GetYaxis().SetBinLabel(nbins + 1,str("o.a."))

    return hist2







