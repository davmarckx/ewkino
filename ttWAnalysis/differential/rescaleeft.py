####################################
# Script to rescale EFT histograms #
####################################
# Run on a fully merged file.
# The input file is supposed to contain EFT histograms named as follows:
# TTW2018EFTcQq83_signalregion_dilepton_inclusive_particlelevel__nMuons_nominal,
# as well as corresponding EFT-SM histograms (i.e. standard model in EFT sample) of the form:
# TTW2018EFTsm_signalregion_dilepton_inclusive_particlelevel__nMuons_nominal,
# as well as corresponding non-EFT-SM histograms (i.e. standard model in non-EFT sample) of the form:
# TTW2018_signalregion_dilepton_inclusive_particlelevel__nMuons_nominal.
# This script will calculate the ratio of the integral of non-EFT-SM to EFT-SM histograms,
# and apply it as a scaling factor to all EFT histograms.

import sys
import os
import ROOT
import argparse


def getscalefactors(histnum, histdenom, mode='integral'):
  if mode=='integral': return float(histnum.Integral())/float(histdenom.Integral())
  elif mode=='binperbin':
    scalefactors = []
    for i in range(1, histnum.GetNbinsX()+1):
      scalefactors.append(float(histnum.GetBinContent(i))/float(histdenom.GetBinContent(i)))
    return scalefactors
  else: raise Exception('Mode {} not recognized.'.format(mode))

def applyscalefactors(hist, scalefactors, mode='integral'):
  if mode=='integral': hist.Scale(scalefactors)
  elif mode=='binperbin':
    if len(scalefactors)!=hist.GetNbinsX():
      print('WARNING: invalid length of scalefactors list for hist {}.'.format(hist.GetName()))
      scalefactors = [1.]*hist.GetNbinsX()
    for i in range(1, hist.GetNbinsX()+1):
      hist.SetBinContent(i, hist.GetBinContent(i)*scalefactors[i-1])
      hist.SetBinError(i, hist.GetBinError(i)*scalefactors[i-1])
  else: raise Exception('Mode {} not recognized.'.format(mode))


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Rescale EFT histograms')
  parser.add_argument('-i', '--inputfiles', required=True, type=os.path.abspath, nargs='+')
  parser.add_argument('-m', '--mode', choices=['integral', 'binperbin'], default='integral')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
  
  # loop over files
  for inputfile in args.inputfiles:
    f = ROOT.TFile.Open(inputfile,'update')
    keylist = f.GetListOfKeys()
    keynamelist = [key.GetName() for key in keylist]
    # find all EFTsm histograms and corresponding SM histograms,
    # and calculate the ratios
    ratios = {}
    for keyname in keynamelist:
      pname,rem = keyname.split('_',1)
      if 'EFTsm' not in pname: continue
      smkeyname = keyname.replace('EFTsm','')
      if not smkeyname in keynamelist:
        msg = 'Expected SM histogram {} not found.'.format(smkeyname)
        raise Exception(msg)
      hist = f.Get(keyname)
      hist.SetDirectory(0)
      smhist = f.Get(smkeyname)
      smhist.SetDirectory(0)
      ratios[keyname] = getscalefactors(smhist, hist, mode=args.mode)
    # do the same for hCounters
    for keyname in keynamelist:
      pname,rem = keyname.split('_',1)
      if 'EFT' not in pname: continue
      if not keyname.endswith('hCounter'): continue
      smkeyname = keyname.replace('EFT','')
      if not smkeyname in keynamelist:
        msg = 'Expected SM histogram {} not found.'.format(smkeyname)
        raise Exception(msg)
      hist = f.Get(keyname)
      hist.SetDirectory(0)
      smhist = f.Get(smkeyname)
      smhist.SetDirectory(0)
      ratio = getscalefactors(smhist, hist, mode=args.mode)
      ratios[keyname] = ratio
    # loop over histograms
    for keyname in keynamelist:
      pname,rem = keyname.split('_',1)
      # select EFT histograms and ignore already scaled histograms
      if not 'EFT' in pname: continue
      if 'EFTScaled' in pname: continue
      # retrieve correct ratio
      if keyname.endswith('hCounter'): ratiokeyname = keyname
      else: ratiokeyname = pname.split('EFT',1)[0]+'EFTsm' + '_' + rem
      if not ratiokeyname in ratios.keys():
        msg = 'Exptected ratio key {} not found.'.format(ratiokeyname)
        raise Exception(msg)
      # get the histogram and scale
      hist = f.Get(keyname).Clone()
      hist.SetDirectory(0)
      hist.SetName(keyname.replace('EFT', 'EFTScaled'))
      applyscalefactors(hist, ratios[ratiokeyname], mode=args.mode)
      # write scaled histogram
      hist.Write()
    f.Close()
