#########################################
# plot the results of a likelihood scan #
#########################################

import os
import sys
import ROOT
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_patch1' # temporary
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Make plots of a likelihood scan')
  parser.add_argument('--datacarddir', required=True)
  parser.add_argument('--cards', required=True, nargs='+')
  parser.add_argument('--pois', default=['r'], nargs='+')
  parser.add_argument('--doexp', default=False, action='store_true')
  parser.add_argument('--doexpstat', default=False, action='store_true')
  parser.add_argument('--doobs', default=False, action='store_true')
  parser.add_argument('--doobsstat', default=False, action='store_true')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # loop over cards and pois
  commands = []
  cwd = os.getcwd()
  for card in args.cards:
    for poi in args.pois:
      print('Making command for card {} POI {}...'.format(card,poi))

      # find all relevant files
      fls = []
      labels = []
      colors = []
      poitag = '' if poi=='r' else '_{}'.format(poi)
      basename = 'higgsCombine{}_out_likelihoodscan_{}.MultiDimFit.mH120.root'
      obs = basename.format(card,'obs',poitag)
      if args.doobs:
        if os.path.exists(os.path.join(args.datacarddir,obs)):
          fls.append(obs)
          labels.append('Observed')
          colors.append(ROOT.kBlue+1)
          print('  adding file {}'.format(obs))
        else:
          msg = 'WARNING: requested to plot observed likelihood'
          msg += ' but file {} could not be found in {}'.format(obs,args.datacarddir)
          print(msg)
      obsstat = basename.format(card,'obs_stat',poi)
      if args.doobsstat:
        if os.path.exists(os.path.join(args.datacarddir,obsstat)):
          fls.append(obsstat)
          labels.append('Observed (stat. only)')
          colors.append(ROOT.kAzure-4)
          print('  adding file {}'.format(obsstat))
        else:
          msg = 'WARNING: requested to plot observed stat-only likelihood'
          msg += ' but file {} could not be found in {}'.format(obsstat,args.datacarddir)
          print(msg)
      exp = basename.format(card,'exp',poi)
      if args.doexp:
        if os.path.exists(os.path.join(args.datacarddir,exp)):
          fls.append(exp)
          labels.append('Expected')
          colors.append(ROOT.kMagenta+2)
          print('  adding file {}'.format(exp))
        else:
          msg = 'WARNING: requested to plot expected likelihood'
          msg += ' but file {} could not be found in {}'.format(exp,args.datacarddir)
          print(msg)
      expstat = basename.format(card,'exp_stat',poi)
      if args.doexpstat:
        if os.path.exists(os.path.join(args.datacarddir,expstat)):
          fls.append(expstat)
          labels.append('Expected (stat. only)')
          colors.append(ROOT.kViolet-4)
          print('  adding file {}'.format(expstat))
        else:
          msg = 'WARNING: requested to plot expected stat-only likelihood'
          msg += ' but file {} could not be found in {}'.format(expstat,args.datacarddir)
          print(msg)

      if len(fls)==0:
        print('No files found for this card/POI combination, skipping...')
        continue
      # set the output file
      outputfile = card + '_out_likelihoodscan_{}'.format(poi)
      # make the command
      exe = os.path.abspath('../../plotting/python/plot1dscan.py')
      cmd = exe + ' ' + fls[0]
      cmd += ' --output ' + outputfile
      cmd += ' --POI ' + poi
      cmd += ' --main_label ' + labels[0]
      cmd += ' --main_color ' + str(colors[0])
      cmd += ' --others ' + ' '.join(['\'{}:{}:{}\''.format(f,l,c) 
        for f,l,c in zip(fls[1:],labels[1:],colors[1:])])
      commands.append(cmd)

  # set directory
  commands = ['cd {}'.format(args.datacarddir)] + commands
  commands.append('cd {}'.format(cwd))

  # run the commands
  if( args.runmode=='condor' ):
    ct.submitCommandsAsCondorJob( 'cjob_combine_likelihoodscanplot', 
                                  commands, cmssw_version=CMSSW_VERSION )
  else:
    script_name = 'local_combine_likelihoodscanplot.sh'
    ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
    with open( script_name, 'a' ) as script:
      for c in commands: script.write(c+'\n')
    os.system('bash {}'.format(script_name))

