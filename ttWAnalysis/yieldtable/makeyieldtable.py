##############################################################
# Make a yield table from a file containing yield histograms #
##############################################################

import sys
import os
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Make a yield table')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--region', required=True)
  parser.add_argument('--datatag', default='Data')
  parser.add_argument('--isnominalonly', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))

  # get all relevant histograms
  print('Loading histogram names from input file...')
  mustcontainall = []
  variablename = '_yield'
  nominaltag = '_nominal'
  if args.isnominalonly: nominaltag = ''
  # requirement: histogram key must contain yield variable and selected region
  mustcontainall.append(variablename)
  mustcontainall.append(args.region)
  # requirement: histogram must be a nominal one
  mustcontainall.append(nominaltag)
  # do loading and initial selection
  histnames = ht.loadhistnames(args.inputfile, mustcontainall=mustcontainall)
  print('Resulting number of histograms: {}'.format(len(histnames)))

  # make a ProcessInfoCollection to extract information
  PIC = ProcessInfoCollection.fromhistlist( histnames, args.region+'_'+variablename, 
                                            datatag=args.datatag, nominaltag=nominaltag )
  print('Constructed following ProcessInfoCollection from histogram list:')
  print(PIC)

  # get valid processes
  processes = PIC.plist
  print('Extracted following valid process tags from input file:')
  for process in processes: print('  - '+process)

  # make a ProcessCollection
  PC = ProcessCollection( PIC, args.inputfile )

  # extract the yields
  pyields = PC.get_yields()
  print('Yields:')
  for key in sorted(pyields.keys()):
    print('  - {} : {}'.format(key,pyields[key]))
