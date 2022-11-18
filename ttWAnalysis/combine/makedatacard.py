###################
# make a datacard #
###################

# The input histograms are supposed to be contained in a single root file.
# The naming of the histograms should be <process name>_<variable name>_<systematic>
# where the systematic is either "nominal" or a systematic name followed by "Up" or "Down".

import sys
import os
import ROOT
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection
from datacardtools import writedatacard 


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make datacard')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True)
  parser.add_argument('--region', required=True)
  parser.add_argument('--variable', required=True)
  parser.add_argument('--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('--signals', default=None,
                      help='Comma-separated list of process tags to consider as signal.')
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--datatag', default='data')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # parse input file
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: requested to run on '+args.inputfile
                    +' but it does not seem to exist...')

  # parse the string with process tags
  processes = args.processes.split(',')
  doallprocesses = (len(processes)==1 and processes[0]=='all')

  # parse the string with signal tags
  nosignal = False
  if args.signals is None: nosignal = True
  else: signals = args.signals.split(',')

  # parse include and exclude tags
  includetags = []
  if args.includetags is not None: includetags = args.includetags.split(',')
  excludetags = []
  if args.excludetags is not None: excludetags = args.excludetags.split(',')

  # make the output directory
  outputdir, outputfilename = os.path.split(args.outputfile)
  outputfilename = os.path.splitext(outputfilename)[0]
  if not os.path.exists(outputdir):
    os.makedirs(outputdir)

  # get all relevant histograms
  print('Loading histogram names from input file...')
  # requirement: the histogram name must contain at least one includetag (or nominal)
  mustcontainone = []
  if len(includetags)>0: mustcontainone = includetags + ['nominal']
  # requirement: consider only the requested variable
  mustcontainall = []
  mustcontainall.append('_{}_'.format(args.variable))
  # requirement: consider only the requested region
  mustcontainall.append('_{}_'.format(args.region))
  # shortcut requirement for when only one process is requested
  if( len(processes)==1 and not doallprocesses ): mustcontainall.append(processes[0])
  # do loading and initial selection
  histnames = ht.loadhistnames(args.inputfile, mustcontainone=mustcontainone,
		           maynotcontainone=excludetags,
		           mustcontainall=mustcontainall)
  print('Initial selection:')
  print(' - mustcontainone: {}'.format(mustcontainone))
  print(' - mustontainall: {}'.format(mustcontainall))
  print(' - maynotcontainone: {}'.format(excludetags))
  print('Resulting number of histograms: {}'.format(len(histnames)))
  # select processes
  if not doallprocesses: 
    mustcontainone = ['{}_'.format(p) for p in processes]
    histnames = lt.subselect_strings(histnames, mustcontainone=mustcontainone)[1]
    print('Further selection (processes):')
    print('Resulting number of histograms: {}'.format(len(histnames)))

  # make a ProcessInfoCollection to extract information
  splittag = args.region+'_'+args.variable
  PIC = ProcessInfoCollection.fromhistlist( histnames, splittag, datatag=args.datatag )
  print('Constructed following ProcessInfoCollection from histogram list:')
  print(PIC)

  # get valid processes and compare to arguments
  if doallprocesses: processes = PIC.plist
  else:
    for p in processes:
      if p not in PIC.plist:
        raise Exception('ERROR: requested process {}'.format(p)
                        +' not found in the ProcessInfoCollection.')
  print('Extracted following valid process tags from input file:')
  for process in processes: print('  - '+process)
        
  # get valid systematics and compare to arguments
  shapesyslist = PIC.slist
  print('Extracted following relevant systematics from histogram file:')
  for systematic in shapesyslist: print('  - '+systematic)

  # write the datacard
  writedatacard( outputdir, outputfilename, PIC,
                 args.inputfile, args.variable, 
                 datatag=args.datatag,
                 shapesyslist=shapesyslist, lnNsyslist=[],
                 rateparamlist=[], ratio=[],
                 automcstats=10,
                 writeobs=False,
                 autof=False )
