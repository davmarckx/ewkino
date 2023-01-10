######################################
# Rebin a variable in an output file #
######################################
# note: many hard-coded things for now,
#       maybe generalize later

import sys
import os
import argparse
import ROOT
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
import argparsetools as apt


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Rebin a variable')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--oldvariable', required=True)
  parser.add_argument('--newvariable', required=True)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputfile))

  # find required histograms
  histnames = ht.loadallhistnames(args.inputfile)
  tag = '_{}_'.format(args.oldvariable)
  histnames = lt.subselect_strings(histnames, mustcontainall=[tag])[1]

  # load required histograms 
  hists = ht.loadhistogramlist(args.inputfile, histnames)

  # do rebinning
  newhists = []
  newkeys = []
  for histname, hist in zip(histnames, hists):
    if args.oldvariable=='_testDoubleVar':
      name = hist.GetName().replace(args.oldvariable, args.newvariable)
      title = hist.GetTitle().replace(args.oldvariable, args.newvariable)
      key = histname.replace(args.oldvariable, args.newvariable)
      newhist = ROOT.TH1D(name, title, 12, 0.5, 12.5)
      newhist.SetDirectory(0)
      mapping = [3,4,5,6, 9,10,11,12, 15,16,17,18]
      for newbin, oldbin in enumerate(mapping):
        newbin = newbin+1
        content = hist.GetBinContent(oldbin)
        error = hist.GetBinError(oldbin)
        newhist.SetBinContent(newbin, content)
        newhist.SetBinError(newbin, error)
      newhists.append(newhist)
      newkeys.append(key)

  # write new histograms to output file
  outf = ROOT.TFile.Open(args.outputfile,'recreate')
  for histname, hist in zip(newkeys, newhists):
    hist.Write(histname)
  outf.Clone()
