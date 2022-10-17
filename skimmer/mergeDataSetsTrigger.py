####################################################
# Merge datasets for trigger efficiency measurment #
####################################################

import sys
import os
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
import argparse

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Merge datasets for trigger efficiency measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))
  exe = './mergeDataSets'
  # check if executable exists
  if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not exist'.format(exe))

  # make merging dict
  mergedict = {}
  for f in os.listdir(args.inputdir):
    if f[-5:]!='.root': continue
    (dataset,version) = f.split('_',1)
    merged = '_'.join(['Merged',version])
    if merged in mergedict.keys():
      mergedict[merged].append(os.path.join(args.inputdir,f))
    else:
      mergedict[merged] = [os.path.join(args.inputdir,f)]

  # print and ask for confirmation
  print('Will merge the following files:')
  for key,val in sorted(mergedict.items()):
    for el in val: print(' - {}'.format(el))
    print('  --> {}'.format(key))
  print('Continue? (y/n)')
  go = raw_input()
  if go != 'y': sys.exit()

  # continue with submission
  for outputfile, inputfiles in mergedict.items():
    cmd = '{} {}'.format(exe,os.path.join(args.inputdir,outputfile))
    for f in inputfiles: cmd += ' {}'.format(f)
    # submit the command
    ct.submitCommandAsCondorJob( 'cjob_mergeDataSetsTrigger', cmd,
                                  cmssw_version=CMSSW_VERSION ) 
