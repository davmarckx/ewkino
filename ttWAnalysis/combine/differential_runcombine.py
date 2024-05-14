##################################################
# Perform differential cross-section measurement #
##################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
#CMSSW_VERSION = '/user/llambrec/CMSSW_10_2_16_patch1' # temporary
CMSSW_VERSION = '~/CMSSW_10_2_13_combine/CMSSW_10_2_13'
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt
import listtools as lt
from variabletools import read_variables
from inclusive_runcombine import cardtochannel

def getcardcombinations(datacarddir, varname, verbose=False):
  ### get a dictionary linking combined names to lists of corresponding datacards
  # note: assumed to be run on a clean data card directory, 
  #       containing only elementary datacards and corresponding histograms
  combineddict = {}
  cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
  cards_all = lt.subselect_strings(cards_all,maynotcontainone=['_out_'])[1]
  cards_sr = [f for f in cards_all if ('signalregion' in f and f.endswith(varname+'.txt'))]
  cards_cr = [f for f in cards_all if 'controlregion' in f]
  # total combination  
  combineddict['dc_combined_{}.txt'.format(varname)] = cards_cr + cards_sr
  # convert list of cards to dictionary of cards to channel names
  for combcard,elcards in combineddict.items():
    elcarddict = {}
    for card in elcards: elcarddict[card] = cardtochannel(card)
    combineddict[combcard] = elcarddict

  # print out which combinations will be made
  if verbose:
    print('INFO in getcardcombinations:'
          +' found following card combinations:')
    for combcard in sorted(list(combineddict.keys())):
      print('  '+combcard)
      for card,ch in combineddict[combcard].items():
        print('    {} (channel {})'.format(card,ch))

  # return the result
  return combineddict

def getpomap( signals ):
  ### define the mapping of signal strengths
  # input arguments:
  # - list of signal process names
  pomap = '-P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO verbose'
  parameters = ['r_{}'.format(s) for s in signals]
  for s,p in zip(signals, parameters):
    prange = '[1,0,5]'
    pomap += ' --PO \'map=.*/{}:{}{}\''.format(s,p,prange)
  return (parameters,pomap)


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Differential combine measurement')
  parser.add_argument('-d', '--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath)
  parser.add_argument('-m', '--method', default='multidimfit', choices=['multidimfit'])
  parser.add_argument('-s', '--signals', default='auto')
  parser.add_argument('--runelementary', default=False, action='store_true')
  parser.add_argument('--runcombinations', default=False, action='store_true')
  parser.add_argument('--includestatonly', default=False, action='store_true')
  parser.add_argument('--includedata', default=False, action='store_true')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.datacarddir))

  # clean the data card dir
  cleansucces = cbt.cleandatacarddir( args.datacarddir )
  if not cleansucces:
    sys.exit()

  # read variables
  variables = read_variables( args.variables )
  varnames = [var.name for var in variables]

  # loop over variables
  for varname in varnames:
    print('Running on variable {}...'.format(varname))

    # initialize all cards to run as empty list
    cardstorun = []

    # parse pois
    pois = ['r']
    pomap = ''
    if args.signals is not None:
      if args.signals=='auto':
        # read signals from one of the datacards
        card = ([f for f in os.listdir(args.datacarddir) 
                 if ('signalregion' in f and f.endswith('{}.txt'.format(varname)))])
        if len(card)!=1:
            msg = 'ERROR: could not extract POIs from datacards,'
            msg += ' as an unexpected number of datacards was found: {}'.format(len(card))
            msg += ' (while expecting 1);'
            msg += ' skipping this variable...'
            print(msg)
        card = card[0]
        card = os.path.join(args.datacarddir,card)
        with open(card,'r') as f:
          lines = f.readlines()
        for i,line in enumerate(lines):
          if line.startswith('process'):
            idx = i
            break
        processes = [p for p in lines[idx].strip('\n').split(' ')[1:] if len(p)>0]
        pids = [pid for pid in lines[idx+1].strip('\n').split('  ')[1:] if len(pid)>0]
        signals = []
        for p,pid in zip(processes,pids):
          if int(pid)<=0: signals.append(p)
      else:
        # pre-defined signals
        signals = args.signals.split(',')
      (pois, pomap) = getpomap(signals)
    print('Using following POIs:')
    print(pois)
    print('Using following PO map:')
    print(pomap)

    # add elementary signal region cards if requested
    if args.runelementary:
      cards_all = [f for f in os.listdir(args.datacarddir) if f[-4:]=='.txt']
      cards_sr = [f for f in cards_all if ('signalregion' in f and f.endswith(varname+'.txt'))]
      for card in sorted(cards_sr): cardstorun.append(card)

    # add combined cards if requested
    if args.runcombinations:
      combinationdict = getcardcombinations(args.datacarddir, varname, verbose=True)
      combinedcards = cbt.makecombinedcards(args.datacarddir, combinationdict,
                      cmssw_version=CMSSW_VERSION)
      for card in sorted(combinedcards): cardstorun.append(card)

    # run all requested cards
    for card in cardstorun:
      print('running combine for '+card)

      commands = []
      # get workspace commands
      for c in cbt.get_workspace_commands( args.datacarddir, card, options=pomap ): 
        commands.append(c)
      # get fit commands
      func = cbt.get_multidimfit_commands
      for c in func( args.datacarddir, card,
        pois=pois, usedata=False, dostatonly=False):
        commands.append(c)
      if args.includestatonly:
        for c in func( args.datacarddir, card,
          pois=pois, usedata=False, dostatonly=True):
          commands.append(c)
      if args.includedata:
        for c in func( args.datacarddir, card,
          pois=pois, usedata=True, dostatonly=False):
          commands.append(c)
      if( args.includedata and args.includestatonly):
        for c in func( args.datacarddir, card,
          pois=pois, usedata=True, dostatonly=True):
          commands.append(c)

      if( args.runmode=='condor' ):
        ct.submitCommandsAsCondorJob( 'cjob_combine_differential', commands,
               cmssw_version=CMSSW_VERSION )
      else:
        script_name = 'local_combine_differential.sh'
        ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
        with open( script_name, 'a' ) as script:
          for c in commands: script.write(c+'\n')
        os.system('bash {}'.format(script_name))
