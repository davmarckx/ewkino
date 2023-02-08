############################################################
# plot results of a differential cross-section measurement #
############################################################

import sys
import os
import json
import argparse
import ROOT
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
import argparsetools as apt
from variabletools import read_variables
sys.path.append(os.path.abspath('../../plotting/python'))
from differentialplotter import plotdifferential
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection

def calcchi2(observed, expected):
  ### calculate the chi2 divergence between two histograms
  # note: how to take into account uncertainties?
  chi2 = 0
  ndof = 0
  for i in range(1, expected.GetNbinsX()+1):
    exp = expected.GetBinContent(i)
    obs = observed.GetBinContent(i)
    if exp>0:
      chi2 += (obs-exp)**2/exp
      ndof += 1
  return (chi2,ndof)


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot differential cross-section')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True)
  parser.add_argument('--region', required=True)
  parser.add_argument('--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('--xsecs', required=True, type=os.path.abspath,
                      help='Path to json file holding (predicted) total cross-sections for each process.')
  parser.add_argument('--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('--signalstrength', default=None, type=apt.path_or_none,
                      help='Path to json file holding signal strengths.')
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

  # parse the cross-sections
  if not os.path.exists(args.xsecs):
    raise Exception('ERROR: cross-section file '+args.xsecs
                    +' but it does not seem to exist...')
  with open(args.xsecs, 'r') as f:
    xsecs = json.load(f)

  # parse the variables
  varlist = read_variables(args.variables)
  variablenames = [v.name for v in varlist]

  # read signal strenght file
  signalstrengths = None
  if args.signalstrength is not None:
    with open(args.signalstrength,'r') as f:
      signalstrengths = json.load(f)

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
  if len(includetags)>0: mustcontainone = includetags + ['nominal'] + ['hCounter']
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
  # select regions
  mustcontainone = ['_{}_'.format(args.region)]
  histnames = lt.subselect_strings(histnames, mustcontainone=mustcontainone)[1]
  # select variables
  histnames = lt.subselect_strings(histnames, mustcontainone=variablenames)[1]
  print('Further selection (processes, regions and variables):')
  print('Resulting number of histograms: {}'.format(len(histnames)))
  for histname in histnames: print('  {}'.format(histname))

  # make a ProcessInfoCollection to extract information
  # (use first variable, assume list of processes, systematics etc.
  #  is the same for all variables)
  splittag = args.region+'_particlelevel_'+variablenames[0]
  print('Constructing ProcessInfoCollection using split tag "{}"'.format(splittag))
  PIC = ProcessInfoCollection.fromhistlist( histnames, splittag )
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

  # get the hCounter and cross-sections for each process
  hcounters = []
  xsections = []
  f = ROOT.TFile.Open(args.inputfile,'read')
  for process in processes:
      # get the hCounter
      hcname = str(process+'_'+splittag+'_hCounter')
      if hcname not in histnames:
        msg = 'ERROR: wrong hCounter name:'
        msg += ' histogram {} not found'.format(hcname)
        msg += ' in following list:\n'
        for hname in histnames: msg += '    {}\n'.format(hname)
        raise Exception(msg)
      hcounter = f.Get(hcname)
      hcounter = hcounter.GetBinContent(1)
      hcounters.append(hcounter)
      # get the cross-section
      if process not in xsecs.keys():
        msg = 'ERROR: wrong cross-section:'
        msg += ' process {} not found'.format(process)
        msg += ' in provided dict: {}'.format(xsecs)
        raise Exception(msg)
      xsections.append(xsecs[process]*1000)
      # (factor 1000 is to convert from pb to fb)
  f.Close()

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
    splittag = args.region+'_particlelevel_'+variablename
    PIC = ProcessInfoCollection.fromhistlist( thishistnames, splittag )
    PC = ProcessCollection( PIC, args.inputfile )

    # get one nominal and one total systematic histogram for each process
    nominalhists = []
    systhists = []
    systematics = ['pdfTotalRMS','qcdScalesTotalEnv']
    for process in processes:
      nominalhist = PC.processes[process].get_nominal()
      nominalhist.SetTitle( process.split('_')[0] )
      rsshist = PC.processes[process].get_systematics_rss(systematics=systematics)
      systhist = nominalhist.Clone()
      for i in range(0, nominalhist.GetNbinsX()+2):
        systhist.SetBinError(i, rsshist.GetBinContent(i))
      nominalhists.append(nominalhist)
      systhists.append(systhist)

    # do scaling with hCounter
    for i, process in enumerate(processes):
      hcounter = hcounters[i]
      xsection = xsections[i]
      sumweights = nominalhists[i].GetSumOfWeights()
      scale = xsection/hcounter
      print('Rescaling process {}:'.format(process))
      print('  - sum of weights in histogram: {}'.format(sumweights))
      print('  - hcounter: {}'.format(hcounter))
      print('  - xsection: {}'.format(xsection))
      print('  --> selection efficiency: {}'.format(sumweights/hcounter))
      print('  --> fiducial cross-section: {}'.format(sumweights/hcounter*xsection))
      print('  --> rescaling factor: {}'.format(scale))
      nominalhists[i].Scale(scale)
      systhists[i].Scale(scale)

    # divide bin contents by bin widths
    for nominalhist,systhist in zip(nominalhists,systhists):
      for i in range(1,nominalhist.GetNbinsX()+1):
        binwidth = nominalhist.GetBinWidth(i)
        nominalhist.SetBinContent(i, nominalhist.GetBinContent(i)/binwidth)
	nominalhist.SetBinError(i, nominalhist.GetBinError(i)/binwidth)
	systhist.SetBinContent(i, systhist.GetBinContent(i)/binwidth)
        systhist.SetBinError(i, systhist.GetBinError(i)/binwidth)

    # find signal strengths
    datahist = nominalhists[0].Clone()
    statdatahist = nominalhists[0].Clone()
    if signalstrengths is None:
      datahist.Reset()
      statdatahist.Reset()
    else:
      thisss = signalstrengths.get(variablename,None)
      if thisss is None:
        msg = 'ERROR: variable {} not found in signal strengths,'.format(variablename)
        msg += ' setting data to zero.'
        print(msg)
        datahist.Reset()
        statdatahist.Reset()
      elif len(thisss)!=datahist.GetNbinsX():
        msg = 'ERROR: number of signal strengths and number of bins do not agree'
        msg += ' for variable {},'.format(variablename)
        msg += ' setting data to zero.'
        print(msg)
        datahist.Reset()
        statdatahist.Reset()
      else:
        for i in range(1, datahist.GetNbinsX()+1):
          ss = thisss[i-1][0]
          if len(thisss[i-1])==3:
            error = max(thisss[i-1][1],thisss[i-1][2])
            staterror = 0
          elif len(thisss[i-1])==5:
            error = max(thisss[i-1][3],thisss[i-1][4])
            staterror = max(thisss[i-1][1],thisss[i-1][2])
          datahist.SetBinContent(i, datahist.GetBinContent(i)*ss)
          datahist.SetBinError(i, datahist.GetBinContent(i)*error/ss)
          statdatahist.SetBinContent(i, datahist.GetBinContent(i))
          statdatahist.SetBinError(i, datahist.GetBinContent(i)*staterror/ss)

    # make extra infos to display on plot
    extrainfos = []
    for tag in extratags: extrainfos.append(tag)
    for nominalhist in nominalhists:
      (chi2,ndof) = calcchi2(datahist,nominalhist)
      chi2info = '{} #chi2 / ndof = {:.2f} / {}'.format(
                 nominalhist.GetTitle(), chi2, ndof)
      extrainfos.append(chi2info)

    # set plot properties
    figname = variablename
    figname = os.path.join(args.outputdir,figname)
    yaxdenom = var.axtitle
    if var.shorttitle is not None: yaxdenom = var.shorttitle
    yaxunit = 'fb'
    if var.unit is not None: yaxunit += '/{}'.format(var.unit)
    yaxtitle = 'd#sigma / d{} ({})'.format(yaxdenom,yaxunit)
    extracmstext = 'Preliminary'

    # set lumi value to display
    lumimap = {'run2':137600, '2016':36300, '2017':41500, '2018':59700,
                    '2016PreVFP':19520, '2016PostVFP':16810 }
    lumitext = ''
    if args.year is not None:
      if not args.year in lumimap.keys():
        print('WARNING: year {} not recognized,'.format(args.year)
              +' will not write lumi header.')
      lumi = lumimap.get(args.year,None)
      if lumi is not None: lumitext = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'

    # make the plot
    plotdifferential(
        nominalhists, datahist,
	systhists=systhists,
        statdatahist=statdatahist,
	figname=figname,
        yaxtitle=yaxtitle, xaxtitle=xaxtitle,
        drawoptions='hist e',
        extracmstext=extracmstext,
        lumitext=lumitext,
        extrainfos=extrainfos, infosize=15 )
