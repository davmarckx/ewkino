#################################################################
# Perform cross-section measurement of ttW+ and ttW- separately #
#################################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_patch1' # temporary
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt
import listtools as lt
from inclusive_runcombine import cardtochannel

def getcardcombinations(datacarddir, verbose=False):
  ### get a dictionary linking combined names to lists of corresponding datacards
  # note: assumed to be run on a clean data card directory, 
  #       containing only elementary datacards and corresponding histograms
  combineddict = {}
  cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
  cards_all = lt.subselect_strings(cards_all,maynotcontainone=['_out_'])[1]
  cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
  cards_cr = lt.subselect_strings(cards_all,mustcontainone=['controlregion'])[1]
  # combination of signal regions
  combineddict['dc_combined_sr.txt'] = cards_sr
  # total combination  
  combineddict['dc_combined_all.txt'] = cards_all
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
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--method', default='multidimfit', choices=['multidimfit'])
  parser.add_argument('--runelementary', default=False, action='store_true')
  parser.add_argument('--runcombinations', default=False, action='store_true')
  parser.add_argument('--includestatonly', default=False, action='store_true')
  parser.add_argument('--includegridscan', default=False, action='store_true')
  parser.add_argument('--includedata', default=False, action='store_true')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # hard-coded arguments
  signals = ['TTWplus','TTWminus']

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

  # parse pois
  (pois, pomap) = getpomap(signals)
  print('Using following POIs:')
  print(pois)
  print('Using following PO map:')
  print(pomap)

  # initialize all cards to run as empty list
  cardstorun = []

  # add elementary signal region cards if requested
  if args.runelementary:
      cards_all = [f for f in os.listdir(args.datacarddir) if f[-4:]=='.txt']
      cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
      for card in sorted(cards_sr): cardstorun.append(card)

  # add combined cards if requested
  if args.runcombinations:
      combinationdict = getcardcombinations(args.datacarddir, verbose=True)
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
      npoints = 10000
      poiranges = {}
      for poi in pois: poiranges[poi] = (0.5,1.5)
      fastscan = True
      if( args.includegridscan ):
        for c in cbt.get_likelihoodscan_commands(
	  args.datacarddir, card, pois=pois, usedata=False, 
          dostatonly=False, npoints=npoints, fastscan=fastscan,
          poiranges=poiranges):
          commands.append(c)
      if( args.includegridscan and args.includedata ):
        for c in cbt.get_likelihoodscan_commands(
          args.datacarddir, card, pois=pois, usedata=True, 
          dostatonly=False, npoints=npoints, fastscan=fastscan,
          poiranges=poiranges):
          commands.append(c)

      if( args.runmode=='condor' ):
        ct.submitCommandsAsCondorJob( 'cjob_combine_plusminus', commands,
               cmssw_version=CMSSW_VERSION )
      else:
        script_name = 'local_combine_plusminus.sh'
        ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
        with open( script_name, 'a' ) as script:
          for c in commands: script.write(c+'\n')
        os.system('bash {}'.format(script_name))
