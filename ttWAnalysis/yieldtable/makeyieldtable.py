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
  parser.add_argument('--outputfile', default=None)
  parser.add_argument('--datatag', default='Data')
  parser.add_argument('--signals', default=None, nargs='+')
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
  for histname in histnames: print('  {}'.format(histname))

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

  # extract and sort the yields
  pyields = PC.get_yields()
  pyields = sorted(pyields.items(), key=lambda el: el[1])
  pyields = pyields[::-1]
  pyieldsbkg = []
  pyieldssig = []
  pyieldtot = []
  for pyield in pyields:
    if( args.signals is not None and pyield[0] in args.signals ):
      pyieldssig.append(pyield)
    elif pyield[0]=='total':
      pyieldtot.append(pyield)
    else: pyieldsbkg.append(pyield)
  pyields = pyieldssig + pyieldsbkg + pyieldtot

  # prepare the lines for writing
  lines = []
  for pyield in pyields:
    lines.append('{} : {}'.format(pyield[0],pyield[1]))

  # write or print
  print('Yields for file {}:'.format(args.inputfile))
  for line in lines:
    print('  - {}'.format(line))
  if args.outputfile is not None:
    with open(args.outputfile,'w') as f:
      for line in lines:
        f.write(line+'\n')
