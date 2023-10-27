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
from processinfo import ProcessInfoCollection
from datacardtools import writedatacard
from uncertaintytools import remove_systematics_default
from uncertaintytools import add_systematics_default
from uncertaintytools import get_systematics_to_smooth
from uncertaintytools import remove_systematics_all
from uncertaintytools import add_systematics_dummy
from copy import deepcopy

def makeProcessInfoCollection( inputfile, year, region, variable, processes, 
  signals=[], strict_signals=True,
  includetags=[], excludetags=[], 
  adddata=True, datatag='data',
  rawsystematics=False, dummysystematics=False,
  verbose=False ):
  ### make a ProcessInfoCollection with specified parameters

  # get all relevant histograms
  if verbose: print('Loading histogram names from input file...')
  doallprocesses = (len(processes)==1 and processes[0]=='all')
  # requirement: the histogram name must contain at least one includetag (or nominal)
  mustcontainone = []
  if len(includetags)>0: mustcontainone = includetags + ['nominal']
  # requirement: consider only the requested variable
  mustcontainall = []
  mustcontainall.append('_{}_'.format(variable))
  # requirement: consider only the requested region
  mustcontainall.append('_{}_'.format(region))
  # shortcut requirement for when only one process is requested
  if( len(processes)==1 and not doallprocesses ): mustcontainall.append(processes[0])
  # do loading and initial selection
  histnames = ht.loadhistnames(inputfile, mustcontainone=mustcontainone,
                           maynotcontainone=excludetags,
                           mustcontainall=mustcontainall)
  if verbose:
    print('Initial selection:')
    print(' - mustcontainone: {}'.format(mustcontainone))
    print(' - mustontainall: {}'.format(mustcontainall))
    print(' - maynotcontainone: {}'.format(excludetags))
    print('Resulting number of histograms: {}'.format(len(histnames)))
  # select processes
  if not doallprocesses:
    mustcontainone = ['{}_'.format(p) for p in processes]
    histnames = lt.subselect_strings(histnames, mustcontainone=mustcontainone)[1]
    if verbose:
      print('Further selection (processes):')
      print('Resulting number of histograms: {}'.format(len(histnames)))

  # make a ProcessInfoCollection to extract information
  splittag = region+'_'+variable
  PIC = ProcessInfoCollection.fromhistlist( histnames, splittag,
          datatag=datatag, adddata=adddata )
  if verbose:
    print('Constructed following ProcessInfoCollection from histogram list:')
    print(PIC)

  # get valid processes and compare to arguments
  if doallprocesses: processes = PIC.plist
  else:
    for p in processes:
      if p==datatag: continue
      if p not in PIC.plist:
        raise Exception('ERROR: requested process {}'.format(p)
                        +' not found in the ProcessInfoCollection.')
  if verbose:
    print('Extracted following valid process tags from input file:')
    for process in processes: print('  - '+process)

  # get valid systematics and compare to arguments, [deep(just to be sure)]copy is really necessary, otherwise unexpected updates of shapesyslist can happen later on when PIC is modified
  shapesyslist = deepcopy(PIC.slist)

  if verbose:
    print('Extracted following relevant systematics from histogram file:')
    for systematic in shapesyslist: print('  - '+systematic)

  # set some processes to signal
  for p in signals: 
    if not strict_signals:
      if p not in PIC.plist:
        print('WARNING: requested signal {} not found in ProcessInfoCollection. Will continue anyway without this signal.'.format(p))
        continue
    PIC.makesig( p )

  # manage systematics
  if( not rawsystematics and not dummysystematics ):
    print("correct filter")
    (removedforall, removedspecific) = remove_systematics_default( PIC, year=year )
    for el in removedforall:
      if el in shapesyslist: shapesyslist.remove(el)
    for p in PIC.plist:
      if p not in removedspecific.keys(): continue
      for el in removedspecific[p]:
        if( p in el and el in shapesyslist ):
          shapesyslist.remove(el)
    print(shapesyslist)
    normsyslist = add_systematics_default( PIC, year=year )
    (shapesyslist)
    smoothsyslist = get_systematics_to_smooth( shapesyslist )
    print(shapesyslist)
  elif( dummysystematics ):
    (removedforall, _) = remove_systematics_all( PIC )
    for el in removedforall: shapesyslist.remove(el)
    normsyslist = add_systematics_dummy( PIC )
    smoothsyslist = []
  else:
    normsyslist = []
    smoothsyslist = []

  return (PIC, shapesyslist, normsyslist, smoothsyslist)

  
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
  parser.add_argument('--rateparams', default=None,
                      help='Comma-separated list of process tags to consider as free floating.')
  parser.add_argument('--renamesignals', default=None,
                      help='Comma-separated list (same length and order as --signals)'
                          +'with new names for signals in data card.')
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--datatag', default='data')
  parser.add_argument('--dummydata', default=False, action='store_true')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  parser.add_argument('--rawsystematics', default=False, action='store_true',
                      help='Take the systematics from the input file without modifications'
                          +' (i.e. no disablings and no adding of norm uncertainties).')
  parser.add_argument('--dummysystematics', default=False, action='store_true',
                      help='Use dummy systematics (see uncertaintytools for details).')
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
  signals = []
  if args.signals is not None: signals = args.signals.split(',')

  # parse the string with rate parameter tags
  rateparams = []
  if args.rateparams is not None: rateparams = args.rateparams.split(',')

  # parse the string with rename signal tags
  renamesignals = []
  if args.renamesignals is not None: renamesignals = args.renamesignals.split(',')

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

  # make the ProcessInfoCollection
  adddata = not args.dummydata
  (PIC, shapesyslist, normsyslist, smoothsyslist) = makeProcessInfoCollection( 
    args.inputfile, args.year, args.region, args.variable, processes,
    signals=signals, strict_signals=False,
    includetags=includetags, excludetags=excludetags, 
    adddata=adddata, datatag=args.datatag,
    rawsystematics=args.rawsystematics,
    dummysystematics=args.dummysystematics,
    verbose=True)

  # rename signals if requested
  for oldname,newname in zip(signals,renamesignals):
    PIC.changename(oldname, newname)



  # write the datacard
  writedatacard( outputdir, outputfilename, PIC,
                 args.inputfile, args.variable, 
                 dummydata=args.dummydata,
                 shapesyslist=shapesyslist, smoothsyslist=smoothsyslist,
                 lnnsyslist=normsyslist,
                 rateparamlist=rateparams, ratio=[],
                 automcstats=10,
                 writeobs=False,
                 autof=False )
