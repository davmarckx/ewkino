#######################################
# Make plots of systematic variations #
#######################################
# This script is supposed to be used on an output file of runanalysis.py,
# i.e. a root file containing histograms with the following naming convention:
# <process tag>_<variable name>_<systematic>
# where the systematic is either "nominal" or a systematic name followed by "Up" or "Down".

import sys
import os
import json
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
from variabletools import read_variables
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection
from systplotter import plotsystematics

def get_jec_rms_list( hislist ):
  ### helper function
  # add root-sum-square of the individual JEC variations
  # make sure to exclude the superfluous JEC variations in the selection above
  # or the rss will be too large!
  res = []
  nominalhist = histlist[findbyname( histlist, 'nominal' )]
  jecall = ht.selecthistograms(histlist,mustcontainall=['JECAll','Down'])[1]
  jecgrouped = ht.selecthistograms(histlist,mustcontainall=['JECGrouped','Down'])[1]
  for i,hist in enumerate(jecall):
    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
    jecall[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
    jecall[i].SetName( hist.GetName().replace('Down','Max') )
  for i,hist in enumerate(jecgrouped):
    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
    jecgrouped[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
    jecgrouped[i].SetName( hist.GetName().replace('Down','Max') )
  if( len(jecall)>0 ):
    jecallup = nominalhist.Clone()
    jecallup.Add( ht.rootsumsquare(jecall) )
    jecallup.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
                      + 'JECSqSumAllUp' )
    jecalldown = nominalhist.Clone()
    jecalldown.Add( ht.rootsumsquare(jecall), -1 )
    jecalldown.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
                      + 'JECSqSumAllDown' )
    res.append(jecallup)
    res.append(jecalldown)
  if( len(jecgrouped)>0 ):
    jecgroupedup = nominalhist.Clone()
    jecgroupedup.Add( ht.rootsumsquare(jecgrouped) )
    jecgroupedup.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
                          'JECGrouped')] + 'JECSqSumGroupedUp' )
    jecgroupeddown = nominalhist.Clone()
    jecgroupeddown.Add( ht.rootsumsquare(jecgrouped), -1 )
    jecgroupeddown.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
                            'JECGrouped')] + 'JECSqSumGroupedDown' )
    res.append(jecgroupedup)
    res.append(jecgroupeddown)
  return res


if __name__=="__main__":
    
  # parse arguments
  parser = argparse.ArgumentParser('Plot systematics')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath,
                      help='Input file to start from, supposed to be an output file'
                          +' from runsystematics.cc')
  parser.add_argument('--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('--outputdir', required=True, 
                      help='Directory where to store the output.')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
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

  # parse the variables
  varlist = read_variables(args.variables)
  variablenames = [v.name for v in varlist]

  # parse include and exclude tags
  includetags = []
  if args.includetags is not None: includetags = args.includetags.split(',')
  excludetags = []
  if args.excludetags is not None: excludetags = args.excludetags.split(',')

  # parse tags
  extratags = []
  if args.tags is not None: extratags = args.tags.split(',')
  extratags = [t.replace('_',' ') for t in extratags]

  # make the output directory
  if not os.path.exists(args.outputdir):
    os.makedirs(args.outputdir)

  # get all relevant histograms
  print('Loading histogram names from input file...')
  # requirement: the histogram name must contain at least one includetag (or nominal)
  mustcontainone = []
  if len(includetags)>0: mustcontainone = includetags + ['nominal']
  # shortcut requirements for when only one process or variable is requested
  mustcontainall = []
  if( len(processes)==1 and not doallprocesses ): mustcontainall.append(processes[0])
  if len(variablenames)==1: mustcontainall.append(variablenames[0])
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
  # select variables
  histnames = lt.subselect_strings(histnames, mustcontainone=variablenames)[1]
  print('Further selection (processes and variables):')
  print('Resulting number of histograms: {}'.format(len(histnames)))

  # make a ProcessInfoCollection to extract information
  # (use first variable, assume list of processes, systematics etc.
  #  is the same for all variables)
  PIC = ProcessInfoCollection.fromhistlist( histnames, variablenames[0], datatag='data' )
  print('Constructed following ProcessInfoCollection from histogram list:')
  print(PIC)

  # get valid processes and compare to arguments
  if doallprocesses:
    processes = PIC.plist
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
	
  # loop over variables
  for var in varlist:

    # get name and title
    variablename = var.name
    xaxtitle = var.axtitle
    print('Now running on variable {}...'.format(variablename))

    # make a ProcessCollection for this variable
    PIC = ProcessInfoCollection.fromhistlist( histnames, variablename, datatag='data' )
    PC = ProcessCollection( PIC, args.inputfile )

    # get the histograms
    histlist = []
    # loop over all systematics
    for systematic in sorted(shapesyslist):
      histlist.append( PC.get_systematic_up(systematic) )
      histlist.append( PC.get_systematic_down(systematic) )
    # also add the nominal
    histlist.append( PC.get_nominal() )
	    
    # re-order histograms to put individual pdf, qcd and jec variations in front
    # (so they will be plotted in the background)
    firsthistlist = []
    secondhistlist = []
    for hist in histlist:
      if( 'ShapeVar' in hist.GetName() 
	  or 'JECAll' in hist.GetName() 
          or 'JECGrouped' in hist.GetName() ):
        firsthistlist.append(hist)
      else: secondhistlist.append(hist)
    histlist = firsthistlist + secondhistlist

    # add squared sum of jecs (disable when not needed
    #jecrms = get_jec_rms_list(histlist)
    #for hist in jecrms: histlist.append(hist)

    # make extra infos to display on plot
    extrainfos = []
    pinfohead = 'Processes:'
    pinfostr = ''
    for p in processes: pinfostr += '{}, '.format(str(p))
    pinfostr = pinfostr.strip(' ,')
    extrainfos.append(pinfohead)
    extrainfos.append(pinfostr)
    for tag in extratags: extrainfos.append(tag)

    # set plot properties
    figname = args.inputfile.split('/')[-1].replace('.root','')+'_var_'+variablename 
    figname = os.path.join(args.outputdir,figname)
    yaxtitle = 'Events'
    relyaxtitle = 'Normalized'
    labellist = [hist.GetName().split(variablename)[-1].strip('_') for hist in histlist]
    plotsystematics(histlist, labellist, figname+'_abs', 
                    yaxtitle=yaxtitle, xaxtitle=xaxtitle,
                    relative=False, staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15)
    plotsystematics(histlist, labellist, figname+'_rel',
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    relative=True, staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15)
