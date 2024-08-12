#########################################
# Make plots on the basis of a datacard #
#########################################
# Looper 2: run over multiple datacard directories


import sys
import os
import argparse


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make datacard plots')
  parser.add_argument('-d', '--datacarddirs', required=True, type=os.path.abspath, nargs='+')
  parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('-c', '--colormap', default='default')
  parser.add_argument('-s', '--signals', default=None, nargs='+')
  parser.add_argument('--extracmstext', default='Preliminary')
  parser.add_argument('--unblind', default=False, action='store_true')
  parser.add_argument('--dolog', default=False, action='store_true')
  parser.add_argument('--includesys', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # check input directories
  for datacarddir in args.datacarddirs:
    if not os.path.exists(datacarddir):
      raise Exception('ERROR: datacard directory{} does not exist.'.format(datacarddir))

  # loop over input directories
  for datacarddir in args.datacarddirs:

    # define output directory
    outputdir = os.path.join(args.outputdir, os.path.basename(datacarddir))

    # make the command
    cmd = 'python datacardplots_loop.py'
    cmd += ' -d {}'.format(datacarddir)
    cmd += ' -o {}'.format(outputdir)
    cmd += ' -c {}'.format(args.colormap)
    cmd += ' -s {}'.format(' '.join(args.signals))
    cmd += ' --extracmstext {}'.format(args.extracmstext)
    if args.unblind: cmd += ' --unblind'
    if args.dolog: cmd += ' --dolog'
    if args.includesys: cmd += ' --includesys'

    # run the command
    os.system(cmd)
