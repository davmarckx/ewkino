###############################################
# Perform inclusive cross-section measurement #
###############################################
#python inclusive_runcombine.py --datacarddir datacards_single_inclusive/ --method fitdiagnostics --includesignificance --runcombinations --runelementary --includedata --includestatonly --runmode condor

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_UL3' # temporary
#CMSSW_VERSION = '~/CMSSW_10_2_16_patch1'
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt
import listtools as lt

def cardtochannel( card ):
    return card.replace('datacard_','').replace('dc_combined','').replace('.txt','')

def getcardcombinations(datacarddir, combinations=None, verbose=False):
  ### get a dictionary linking combined names to lists of corresponding datacards
  # note: assumed to be run on a clean data card directory, 
  #       containing only elementary datacards and corresponding histograms
  combineddict = {}
  cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
  cards_all = lt.subselect_strings(cards_all,maynotcontainone=['_out_'])[1]
  cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
  cards_cr = lt.subselect_strings(cards_all,mustcontainone=['controlregion'])[1]
  # no specific combinations
  if combinations is None: pass
  # per year
  if combinations=='peryear':
    combineddict['dc_combined_2016PreVFP.txt'] = [c for c in cards_all if '2016PreVFP' in c]
    combineddict['dc_combined_2016PostVFP.txt'] = [c for c in cards_all if '2016PostVFP' in c]
    combineddict['dc_combined_2017.txt'] = [c for c in cards_all if '2017' in c]
    combineddict['dc_combined_2018.txt'] = [c for c in cards_all if '2018' in c]
  # per channel
  elif combinations=='perchannel':
    combineddict['dc_combined_ee.txt'] = [c for c in cards_sr if '_ee' in c] + cards_cr
    combineddict['dc_combined_em.txt'] = [c for c in cards_sr if '_em' in c] + cards_cr
    combineddict['dc_combined_me.txt'] = [c for c in cards_sr if '_me' in c] + cards_cr
    combineddict['dc_combined_mm.txt'] = [c for c in cards_sr if '_mm' in c] + cards_cr
    combineddict['dc_combined_trilepton.txt'] = [c for c in cards_sr if '_trilepton' in c] + cards_cr
  # per sign
  elif combinations=='persign':
    combineddict['dc_combined_plus.txt'] = [c for c in cards_sr if '_plus' in c] + cards_cr
    combineddict['dc_combined_minus.txt'] = [c for c in cards_sr if '_minus' in c] + cards_cr
  else:
    raise Exception('ERROR: combinations specifier {} not recognized.'.format(combinations))
  # all signal regions
  combineddict['dc_combined_signalregions.txt'] = cards_sr[:]
  # all control regions
  combineddict['dc_combined_controlregions.txt'] = cards_cr[:]
  # total combination  
  combineddict['dc_combined_all.txt'] = cards_all[:]
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

  # ask for confirmation
  print('Continue? (y/n)')
  go = raw_input()
  #go = input()
  if not go=='y': sys.exit()

  # return the result
  return combineddict


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Inclusive combine measurement')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--method', default='multidimfit', 
    choices=['multidimfit','fitdiagnostics','initimpacts'])
  parser.add_argument('--runelementary', default=False, action='store_true')
  parser.add_argument('--runcombinations', default=False, action='store_true')
  parser.add_argument('--combinations', default=None,
    choices=[None, 'peryear', 'perchannel', 'persign'])
  parser.add_argument('--includesignificance', default=False, action='store_true')
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
  if not cleansucces: sys.exit()

  # initialize all cards to run as empty list
  cardstorun = []

  # add elementary signal region cards if requested
  if args.runelementary:
    cards_all = [f for f in os.listdir(args.datacarddir) if f[-4:]=='.txt']
    #cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
    cards_sr = cards_all # for small tests with other naming convention
    for card in sorted(cards_sr): cardstorun.append(card)

  # add combined cards if requested
  if args.runcombinations:
    combinationdict = getcardcombinations(args.datacarddir, combinations=args.combinations, verbose=True)
    combinedcards = cbt.makecombinedcards(args.datacarddir, combinationdict,
                      cmssw_version=CMSSW_VERSION)
    for card in sorted(combinedcards): cardstorun.append(card)

  # run all requested cards
  for card in cardstorun:
    print('running combine for '+card)
    commands = cbt.get_default_commands( args.datacarddir, card, 
                        method=args.method,
                        includesignificance=args.includesignificance,
                        includestatonly=args.includestatonly, 
                        includedata=args.includedata )

    if( args.runmode=='condor' ):
      ct.submitCommandsAsCondorJob( 'cjob_combine_inclusive', commands,
             cmssw_version=CMSSW_VERSION )
    else:
      script_name = 'local_combine_inclusive.sh'
      ct.initJobScript(script_name, cmssw_version=CMSSW_VERSION)
      with open( script_name, 'a' ) as script:
        for c in commands: script.write(c+'\n')
      os.system('bash {}'.format(script_name))
