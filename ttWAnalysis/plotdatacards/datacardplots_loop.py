#########################################
# Make plots on the basis of a datacard #
#########################################
# Looper: run for all elementary datacards in a directory


import sys
import os
import argparse


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make datacard plots')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--colormap', default='default')
  parser.add_argument('--signals', default=None, nargs='+')
  parser.add_argument('--extracmstext', default='Preliminary')
  parser.add_argument('--unblind', default=False, action='store_true')
  parser.add_argument('--dolog', default=False, action='store_true')
  parser.add_argument('--includesys', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # check input directory
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: datacard directory{} does not exist.'.format(args.datacarddir))

  # make the output directory
  if not os.path.exists(args.outputdir):
    os.makedirs(args.outputdir)

  # find input files
  histfiles = sorted([ el for el in os.listdir(args.datacarddir)
                if( el.startswith('histograms_') and el.endswith('.root') )])
  cards = (['datacard_{}.txt'.format(histfile.replace('histograms_','').replace('.root',''))
             for histfile in histfiles])
  outputfiles = [card.replace('.txt','') for card in cards]

  # loop
  for card, histfile, outputfile in zip(cards, histfiles, outputfiles):
    # make the command for regular plot
    cmd = 'python datacardplots.py'
    cmd += ' --datacard {}'.format(os.path.join(args.datacarddir,card))
    cmd += ' --histfile {}'.format(os.path.join(args.datacarddir,histfile))
    cmd += ' --outputfile {}'.format(os.path.join(args.outputdir,outputfile))
    if args.colormap is not None: cmd += ' --colormap {}'.format(args.colormap)
    if args.signals is not None: 
      cmd += ' --signals'
      for s in args.signals: cmd += ' {}'.format(s)
    if args.extracmstext is not None: cmd += ' --extracmstext {}'.format(args.extracmstext)
    if args.unblind: cmd += ' --unblind'
    if args.dolog: cmd += ' --dolog'
    # run the command
    os.system(cmd)
    # make the command for systematics plot
    sysoutputfile = outputfile+'_sys'
    cmd = 'python datacardsysplots.py'
    cmd += ' --datacard {}'.format(os.path.join(args.datacarddir,card))
    cmd += ' --histfile {}'.format(os.path.join(args.datacarddir,histfile))
    cmd += ' --outputfile {}'.format(os.path.join(args.outputdir,sysoutputfile))
    cmd += ' --group'
    cmd += ' --includetotal'
    # run the command
    os.system(cmd)
