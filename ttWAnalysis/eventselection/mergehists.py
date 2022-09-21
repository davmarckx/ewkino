#######################################
# merge output files from eventbinner #
#######################################
# note: corresponds to new convention with one file per sample
#       (inclusve in event selections and selection types)

import sys
import os
import ROOT
import argparse
sys.path.append('../../Tools/python')
import histtools as ht


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Merge histograms')
  parser.add_argument('--directory', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--npmode', required=True, choices=['npfromsim','npfromdata'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.directory))

  # get files to merge
  selfiles = ([os.path.join(args.directory,f) for f in os.listdir(args.directory) 
               if f[-5:]=='.root'])

  # printouts
  print('Will merge the following files:')
  for f in selfiles: print('  - {}'.format(f))
  print('into {}'.format(args.outputfile))

  # make output directory
  outputdirname, outputbasename = os.path.split(args.outputfile)
  if not os.path.exists(outputdirname):
    os.makedirs(outputdirname)

  # first do hadd to merge histograms
  # with same process, event selection, selection type and variable.
  cmd = 'hadd -f {}'.format(args.outputfile)
  for f in selfiles: cmd += ' {}'.format(f)
  os.system(cmd)

  # load the histograms
  histlist = ht.loadallhistograms(args.outputfile)
    
  # select histograms to keep in the output
  # and remove selection type tag, as it is not needed anymore.
  if args.npmode=='npfromsim':
    histlist = ht.selecthistograms(histlist, mustcontainall=['_tight_'])[1]
    if len(histlist)==0:
      print('WARNING: list of selected histograms is empty!')
    for hist in histlist: 
      hist.SetName( hist.GetName().replace('_tight_','_') )
  if args.npmode=='npfromdata':
    histlist = ht.selecthistograms(histlist, mustcontainone=['_prompt_','_fakerate_'])[1]
    if len(histlist==0):
      print('WARNING: list of selected histograms is empty!')
    for hist in histlist: 
      hist.SetName( hist.GetName().replace('_prompt_','_').replace('_fakerate_','_') )

  # clip all resulting histograms to minimum zero
  ht.cliphistograms(histlist)

  # re-write output file
  f = ROOT.TFile.Open(args.outputfile,'recreate')
  for hist in histlist:
    hist.Write()
  f.Close()
