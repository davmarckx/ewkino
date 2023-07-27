########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import argparse

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--variables', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.datacarddir))

  # fixed arguments
  mtypes = ['exp','obs']
  rtypes = ['nocr', 'withcr']

  # loop over configurations
  for mtype in mtypes:
    for rtype in rtypes:
      # define output file
      outputfile = 'summary_{}_{}.json'.format(mtype, rtype)
      outputfile = os.path.join(args.datacarddir, outputfile)
      # make command
      cmd = 'python differential_readoutput.py'
      cmd += ' --datacarddir {}'.format(args.datacarddir)
      cmd += ' --variables {}'.format(args.variables)
      cmd += ' --outputfile {}'.format(outputfile)
      if mtype=='obs': cmd += ' --usedata'
      if rtype=='withcr': cmd += ' --usecr'
      # run the command
      os.system(cmd)
