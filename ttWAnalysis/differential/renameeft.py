###################################
# Script to rename EFT histograms #
###################################
# Need to convert histogram names from something of the form
# TTW2018_signalregion_dilepton_inclusive_particlelevel__nMuons_EFTcQq83
# to something of the form
# TTW2018EFTcQq83_signalregion_dilepton_inclusive_particlelevel__nMuons_nominal
# so they are correctly recognized by plotdifferential.py as separate processes


import sys
import os
import ROOT
import argparse

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Rename EFT histograms')
  parser.add_argument('-i', '--inputfiles', required=True, type=os.path.abspath, nargs='+')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
  
  # loop over files
  for inputfile in args.inputfiles:
    f = ROOT.TFile.Open(inputfile,'update')
    keylist = f.GetListOfKeys()
    # loop over histograms
    for key in keylist:
      keyname = key.GetName()
      pname,rem = keyname.split('_',1)
      rem,eftname = rem.rsplit('_',1)
      # rename hCounter to have EFT in the name
      # (to distinguish from nominal when merging with non-EFT sample)
      if eftname=='hCounter':
        if 'EFT' in pname: continue
        newpname = pname + 'EFT'
        newkeyname = '_'.join([newpname,rem,'hCounter'])
        key.SetName(newkeyname)
      # rename nominal histogram to have EFT in the name
      # (to distinguish from nominal when merging with non-EFT sample)
      if eftname=='nominal':
        if 'EFT' in pname: continue
        newpname = pname + 'EFT'
        newkeyname = '_'.join([newpname,rem,eftname])
        key.SetName(newkeyname)
      # general case
      if not 'EFT' in eftname: continue
      newpname = pname + eftname
      newkeyname = '_'.join([newpname,rem,'nominal'])
      key.SetName(newkeyname)
    f.Close()
