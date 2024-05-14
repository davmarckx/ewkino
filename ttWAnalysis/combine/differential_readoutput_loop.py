########################################################
# Gather and parse the results in a datacard directory #
########################################################

import sys
import os
import argparse

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Parse combine output')
  parser.add_argument('-d', '--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath)
  parser.add_argument('--plotsignalstrengths', default=False, action='store_true')
  parser.add_argument('--plotcorrelations', default=False, action='store_true')
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
      # optionally plot the signal strengths
      if args.plotsignalstrengths:
        cmd = 'python differential_plotoutput.py'
        cmd += ' --signalstrengths {}'.format(outputfile)
        cmd += ' --outputfile {}'.format(outputfile.replace('.json', '.png'))
        os.system(cmd)
      # optionally plot correlations
      if args.plotcorrelations:
        outputdir = outputfile.replace('.json','_correlations')
        cmd = 'python differential_plotcorrelations.py'
        cmd += ' --inputfile {}'.format(outputfile)
        cmd += ' --outputdir {}'.format(outputdir)
        os.system(cmd)
