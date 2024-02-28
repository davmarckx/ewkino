######################################
# Merge output files from filltheory #
######################################

import sys
import os
import ROOT
import argparse
import json
sys.path.append('../../Tools/python')
import histtools as ht
import listtools as lt
import argparsetools as apt
sys.path.append('../eventselection/')
from mergehists import mergehists


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Merge histograms')
  parser.add_argument('-d', '--directory', required=True, type=os.path.abspath)
  parser.add_argument('-o', '--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('-r', '--rename', default=None, type=apt.path_or_none)
  parser.add_argument('--renamemode', default='fast', choices=['custom','rootmv','fast'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.directory))
  if( args.rename is not None and not os.path.exists(args.rename) ):
    raise Exception('ERROR: rename file {} does not exist.'.format(args.rename))

  # set hard-coded arguments needed in called function
  args.npmode = 'merging' # only used for naming temp directory
  args.cfmode = 'dir' # only used for naming temp directory
  args.decorrelate = None
  args.selectmode = None
  args.doclip = False

  # get files to merge
  selfiles = ([os.path.join(args.directory,f) for f in os.listdir(args.directory) 
               if f[-5:]=='.root'])

  # call main function
  mergehists( selfiles, args )
