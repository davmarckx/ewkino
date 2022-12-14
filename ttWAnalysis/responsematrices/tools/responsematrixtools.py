####################################################
# special tools for dealing with response matrices #
####################################################

import sys
import os


def normalize_columns( hist, include_outerflow=False ):
  ### normalize the columns of a TH2D (the y-direction)
  # (i.e. make sure that the sum of each column is 1)
  res = hist.Clone()
  nbins = hist.GetNbinsX()
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
