#######################################
# Make plots of systematic variations #
#######################################
# This script is supposed to be used on an output file of runanalysis.py,
# i.e. a root file containing histograms with the following naming convention:
# <process tag>_<selection region>_<variable name>_<systematic>
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
sys.path.append(os.path.abspath('../combine/'))
from uncertaintytools import remove_systematics_default
sys.path.append(os.path.abspath('../plotting'))
from infodicts import get_region_dict
from colors import getcolormap
from systtools import category
from systplotter import plotsystematics


if __name__=="__main__":
    
  # parse arguments
  parser = argparse.ArgumentParser('Plot systematics')
  parser.add_argument('-i', '--inputfile', required=True, type=os.path.abspath,
                      help='Input file to start from, supposed to be an output file'
                          +' from runsystematics.cc or equivalent')
  parser.add_argument('-y', '--year', required=True)
  parser.add_argument('-r', '--region', required=True)
  parser.add_argument('-p', '--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('-o', '--outputdir', required=True, 
                      help='Directory where to store the output.')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  parser.add_argument('--rawsystematics', action='store_true',
                      help='Take the systematics from the input file without modifications'
                          +' (i.e. no disablings).') 
  parser.add_argument('--datatag', default='data',
                      help='Process name of data histograms in input file.')
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  parser.add_argument('--noclip', default=False, action='store_true',
                      help='Disable automatic clipping of all histograms before plotting.')
  parser.add_argument('--group', default=False, action='store_true',
                      help='Group systematics in categories before plotting.')
  parser.add_argument('--includetotal', default=False, action='store_true',
                      help='Include total systematic uncertainty.')
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
  if includetags==['none']: includetags = []
  excludetags = []
  if args.excludetags is not None: excludetags = args.excludetags.split(',')
  if excludetags==['none']: excludetags = []

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

  # do printouts (ony for testing)
  #for histname in histnames: print('  {}'.format(histname))

  # make a ProcessInfoCollection to extract information
  # (use first variable, assume list of processes, systematics etc.
  #  is the same for all variables)
  splittag = args.region+'_'+variablenames[0]
  print('Constructing ProcessInfoCollection using split tag "{}"'.format(splittag))
  PIC = ProcessInfoCollection.fromhistlist( histnames, splittag, datatag=args.datatag )
  if not args.rawsystematics:
      _ = remove_systematics_default( PIC, region=args.region, year=args.year )
  # (note: removing systematics here has no impact except on printing)
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

  # group systematics
  if args.group:
    categories = [category(systematic) for systematic in shapesyslist]
    catdict = {}
    for cat in set(categories):
        catdict[cat] = [shapesyslist[i] for i in range(len(shapesyslist)) if categories[i]==cat]
    print('Will group systematics as follows:')
    for cat in catdict.keys():
        print('  {}'.format(cat))
        for sys in catdict[cat]: print('    {}'.format(sys))

  # loop over variables
  for var in varlist:

    # get name and title
    variablename = var.name
    xaxtitle = var.axtitle
    print('Now running on variable {}...'.format(variablename))

    # extra histogram selection for overlapping variable names
    othervarnames = [v.name for v in varlist if v.name!=variablename]
    thishistnames = lt.subselect_strings(histnames,
                      mustcontainall=[variablename],
                      maynotcontainone=['_{}_'.format(el) for el in othervarnames])[1]

    # make a ProcessCollection for this variable
    splittag = args.region+'_'+variablename
    PIC = ProcessInfoCollection.fromhistlist( thishistnames, splittag, datatag=args.datatag )
    if not args.rawsystematics:
      _ = remove_systematics_default( PIC, region=args.region, year=args.year )
    PC = ProcessCollection( PIC, args.inputfile )

    # get the nominal histogram
    nominalhist = PC.get_nominal()
    nominalhist.SetTitle('nominal')

    # get the systematics histograms
    syshistlist = []
    # make a list of all systematics
    for systematic in sorted(shapesyslist):
      uphist = PC.get_systematic_up(systematic)
      uphist.SetTitle(systematic+'Up')
      if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
      downhist = PC.get_systematic_down(systematic)
      downhist.SetTitle(systematic+'Down')
      if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
      syshistlist.append(uphist)
      syshistlist.append(downhist)
    # re-group systematics if requested
    if args.group:
      newsyshistlist = []
      for cat in catdict.keys():
        rss = PC.get_systematics_rss(systematics=catdict[cat], correlate_processes=True)
        uphist = nominalhist.Clone()
        uphist.Add(rss)
        uphist.SetTitle(cat+'Up')
        if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
        downhist = nominalhist.Clone()
        downhist.Add(rss, -1)
        downhist.SetTitle(cat+'Down')
        if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
        newsyshistlist.append(uphist)
        newsyshistlist.append(downhist)
      syshistlist = newsyshistlist
    # add total if requested
    if args.includetotal:
        rss = PC.get_systematics_rss(correlate_processes=True)
        uphist = nominalhist.Clone()
        uphist.Add(rss)
        uphist.SetTitle('totalUp')
        if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
        downhist = nominalhist.Clone()
        downhist.Add(rss, -1)
        downhist.SetTitle('totalDown')
        if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
        syshistlist.append(uphist)
        syshistlist.append(downhist)

    # printouts for testing
    #for hist in syshistlist:
    #  print(hist)

    # re-order histograms to put individual pdf, qcd and jec variations in front
    # (so they will be plotted in the background)
    firsthistlist = []
    secondhistlist = []
    for hist in syshistlist:
      if( 'ShapeVar' in hist.GetName() 
	  or 'JECAll' in hist.GetName() 
          or 'JECGrouped' in hist.GetName() ):
        firsthistlist.append(hist)
      else: secondhistlist.append(hist)
    syshistlist = firsthistlist + secondhistlist

    # add squared sum of jecs (disable when not needed
    #jecrms = get_jec_rms_list(syshistlist)
    #for hist in jecrms: syshistlist.append(hist)

    # format the labels
    # (remove year and process tags for more readable legends)
    for hist in syshistlist:
      label = str(hist.GetTitle())
      baselabel = label
      if label.endswith('Up'): baselabel = label[:-2]
      if label.endswith('Down'): baselabel = label[:-4]
      for p in processes:
        p = str(p)
        if baselabel.endswith(p):
          label = label.replace(p,'')
      for y in ['2016PreVFP','2016PostVFP','2017','2018']:
        if baselabel.endswith(y): label = label.replace(y,'')
      hist.SetTitle(label)

    # make extra infos to display on plot
    extrainfos = []
    # processes
    pinfohead = 'Processes:'
    if doallprocesses:
      pinfohead += ' all'
      extrainfos.append(pinfohead)
    else:
      pinfostr = ','.join([str(p) for p in processes])
      extrainfos.append(pinfohead)
      extrainfos.append(pinfostr)
    # year
    yeartag = args.year.replace('run2', 'Run 2')
    extrainfos.append(yeartag)
    # region
    regiontag = get_region_dict().get(args.region, args.region)
    extrainfos.append(regiontag)
    # others
    for tag in extratags: extrainfos.append(tag)

    # choose color map
    colormap = None
    if args.group: colormap = getcolormap('systematics_grouped')

    # set plot properties
    figname = args.inputfile.split('/')[-1].replace('.root','')+'_var_'+variablename 
    figname = os.path.join(args.outputdir,figname)
    yaxtitle = 'Events'
    relyaxtitle = 'Normalized'
    # make absolute plot
    plotsystematics(nominalhist, syshistlist, figname+'_abs', 
                    colormap=colormap,
                    yaxtitle=yaxtitle, xaxtitle=xaxtitle,
                    style='absolute', staterrors=True,
                    doclip=(not args.noclip),
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
    # make normalized plot
    plotsystematics(nominalhist, syshistlist, figname+'_nrm',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='normalized', staterrors=True,
                    doclip=(not args.noclip),
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
    # make relative plot
    plotsystematics(nominalhist, syshistlist, figname+'_rel',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='relative', staterrors=True,
                    doclip=(not args.noclip),
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
