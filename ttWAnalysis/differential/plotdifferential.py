############################################################
# plot results of a differential cross-section measurement #
############################################################

import sys
import os
import json
import argparse
import ROOT
from ROOT import TFile
sys.path.append('tools')
from uncertaintyprop import normalizexsec
from uncertaintyprop import sstoxsec
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
  parser.add_argument('-i', '--inputfile', required=True, type=os.path.abspath,
                      help='Path to input root file with theoretical differential distributions.')
  parser.add_argument('-y', '--year', required=True,
                      help='Data-taking year (only used for plot aesthetics)')
  parser.add_argument('-r', '--region', required=True)
  parser.add_argument('-p', '--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('-x', '--xsecs', required=True, type=os.path.abspath,
                      help='Path to json file holding (predicted) total cross-sections for each process.')
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('-s', '--signalstrength', default=None, type=apt.path_or_none,
                      help='Path to json file holding signal strengths.')
  parser.add_argument('-o', '--outputdir', required=True,
                      help='Directory where to store the output.')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  parser.add_argument('--absolute', default=False, action='store_true',
                      help='If specified, do not divide by bin width,'
                          +' so y-axis unit is in fb instead of fb/GeV.'
                          +' Mostly for testing purposes.')
  parser.add_argument('--writeuncs', default=False, action='store_true',
                      help='Write measurement uncertainties in ratio plot')
  parser.add_argument('--write_rootfiles', default=False, action='store_true',
                      help='If specified, rootfiles are written for data'
                          +' by applying the signal strengths.'
                          +' (Only needed for e.g. hepdata submissions.)')
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
  outputdir = args.outputdir
  if not os.path.exists(outputdir):
    os.makedirs(outputdir)

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
  # select regions
  mustcontainone = ['_{}_'.format(args.region)]
  histnames = lt.subselect_strings(histnames, mustcontainone=mustcontainone)[1]
  # select variables
  histnames = lt.subselect_strings(histnames, mustcontainone=variablenames)[1]
  print('Further selection (processes, regions and variables):')
  print('Resulting number of histograms: {}'.format(len(histnames)))
  #for histname in histnames: print('  {}'.format(histname))

  # get all hCounter histograms (separate from above)
  mustcontainall = ['hCounter']
  # do loading and initial selection
  hcnames = ht.loadhistnames(args.inputfile, mustcontainall=mustcontainall)

  # make a ProcessInfoCollection to extract information
  # (use first variable, assume list of processes, systematics etc.
  #  is the same for all variables)
  splittag = args.region+'_particlelevel_'+variablenames[0]
  print('Constructing ProcessInfoCollection using split tag "{}"'.format(splittag))
  PIC = ProcessInfoCollection.fromhistlist( histnames, splittag )
  #print('Constructed following ProcessInfoCollection from histogram list:')
  #print(PIC)

  # get valid processes and compare to arguments
  if doallprocesses:
    processes = sorted(PIC.plist)
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
      if 'EFT' in process:
        hcname = str(process.split('EFT')[0]+'EFT_'+splittag+'_hCounter')
      if hcname not in hcnames:
        msg = 'ERROR: wrong hCounter name:'
        msg += ' histogram {} not found'.format(hcname)
        msg += ' in following list:\n'
        for hname in hcnames: msg += '    {}\n'.format(hname)
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

  # if we want a collection of data histograms we innit the root file here
  if args.write_rootfiles:
    if not os.path.exists(os.path.join(outputdir,"rootfiles")):
        os.makedirs(os.path.join(outputdir,"rootfiles"))
    rootfile = TFile( os.path.join(outputdir,"rootfiles/differentialplots_{}.root".format(args.year)), 'RECREATE' )

  # loop over variables
  for var in varlist:

    # get name and title
    variablename = var.name
    xaxtitle = var.axtitle
    if var.unit is not None: xaxtitle += ' ({})'.format(var.unit)
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

    # make some copies of reference histograms before dividing by bin width
    # (note: do not forget to scale by xsec and lumi below!)
    nominalrefs = []
    for process in processes:
      nominalrefs.append( PC.processes[process].get_nominal().Clone() )

    # divide bin contents by bin widths in all histograms
    if not args.absolute:
      for hist in PC.get_allhists():
        for i in range(1,hist.GetNbinsX()+1):
          binwidth = hist.GetBinWidth(i)
          hist.SetBinContent(i, hist.GetBinContent(i)/binwidth)
          hist.SetBinError(i, hist.GetBinError(i)/binwidth)

    # make a second ProcessCollection and normalize all its histograms
    # to unit surface area
    PC_norm = ProcessCollection( PIC, args.inputfile )
    for hist in PC_norm.get_allhists():
      if not args.absolute:
        for i in range(1,hist.GetNbinsX()+1):
          binwidth = hist.GetBinWidth(i)
          hist.SetBinContent(i, hist.GetBinContent(i)/binwidth)
          hist.SetBinError(i, hist.GetBinError(i)/binwidth)
        hist.Scale(1./hist.Integral('width'))
      else:
        hist.Scale(1./hist.Integral())

    # get one nominal and one total systematic histogram for each process
    nominalhists = []
    systhists = []
    #systematics = ['pdfTotalRMS','rfScalesTotal','isrTotal','fsrTotal']
    systematics = [] 
    print('WARNING: list of systematics set to empty for testing.')
    for process in processes:
      nominalhist = PC.processes[process].get_nominal()
      nominalhist.SetTitle( process.split('_')[0] )
      rsshist = PC.processes[process].get_systematics_rss(systematics=systematics)
      systhist = nominalhist.Clone()
      for i in range(0, nominalhist.GetNbinsX()+2):
        systhist.SetBinError(i, rsshist.GetBinContent(i))
      nominalhists.append(nominalhist)
      systhists.append(systhist)

    # do the same for normalized histograms
    nominalhists_norm = []
    systhists_norm = []
    for process in processes:
      nominalhist = PC_norm.processes[process].get_nominal()
      nominalhist.SetTitle( process.split('_')[0] )
      rsshist = PC_norm.processes[process].get_systematics_rss(systematics=systematics)
      systhist = nominalhist.Clone()
      for i in range(0, nominalhist.GetNbinsX()+2):
        systhist.SetBinError(i, rsshist.GetBinContent(i))
      nominalhists_norm.append(nominalhist)
      systhists_norm.append(systhist)

    # do scaling with hCounter (for non-normalized histograms)
    for i, process in enumerate(processes):
      hcounter = hcounters[i]
      xsection = xsections[i]
      sumweights = nominalhists[i].GetSumOfWeights()
      scale = xsection/hcounter
      # printouts for testing
      doprint = False
      if doprint:
        print('Rescaling process {}:'.format(process))
        print('  - sum of weights in histogram: {}'.format(sumweights))
        print('  - hcounter: {}'.format(hcounter))
        print('  - xsection: {}'.format(xsection))
        print('  --> selection efficiency: {}'.format(sumweights/hcounter))
        print('  --> fiducial cross-section: {}'.format(sumweights/hcounter*xsection))
        print('  --> rescaling factor: {}'.format(scale))
      nominalhists[i].Scale(scale)
      systhists[i].Scale(scale)
      nominalrefs[i].Scale(scale)

    # find signal strengths
    datahist = nominalrefs[0].Clone()
    statdatahist = nominalrefs[0].Clone()
    normdatahist = nominalrefs[0].Clone()
    normstatdatahist = nominalrefs[0].Clone()
    dochi2 = True
    if signalstrengths is None:
      dochi2 = False
      datahist.Reset()
      statdatahist.Reset()
      normdatahist.Reset()
      normstatdatahist.Reset()
    else:
      thisss = signalstrengths.get(variablename,None)
      if thisss is None:
        msg = 'ERROR: variable {} not found in signal strengths,'.format(variablename)
        msg += ' setting data to zero.'
        print(msg)
        datahist.Reset()
        statdatahist.Reset()
        normdatahist.Reset()
        normstatdatahist.Reset()
      elif len(thisss['pois'])!=datahist.GetNbinsX():
        msg = 'ERROR: number of signal strengths and number of bins do not agree'
        msg += ' for variable {},'.format(variablename)
        msg += ' setting data to zero.'
        print(msg)
        datahist.Reset()
        statdatahist.Reset()
        normdatahist.Reset()
        normstatdatahist.Reset()
      else:
        # convert signal strengths to absolute cross sections
        # (note: not divided by bin width, so values in fb)
        pred = {}
        for i in range(1, nominalrefs[0].GetNbinsX()+1):
          pred['r_TTW{}{}'.format(i,variablename.strip('_'))] = datahist.GetBinContent(i)
        thisxsec = sstoxsec(thisss, pred)
        # fill the histograms for absolute cross sections
        for i in range(1, datahist.GetNbinsX()+1):
          poi = 'r_TTW{}{}'.format(i,variablename.strip('_'))
          if len(thisxsec['pois'][poi])==3:
            error = max(thisxsec['pois'][poi][1], thisxsec['pois'][poi][2])
            staterror = 0
          elif len(thisxsec['pois'][poi])==5:
            error = max(thisxsec['pois'][poi][3], thisxsec['pois'][poi][4])
            staterror = max(thisxsec['pois'][poi][1], thisxsec['pois'][poi][2])
          datahist.SetBinContent(i, thisxsec['pois'][poi][0])
          datahist.SetBinError(i, error)
          statdatahist.SetBinContent(i, thisxsec['pois'][poi][0])
          statdatahist.SetBinError(i, staterror)
        # calculate normalized cross-sections
        thisnormxsec = normalizexsec(thisxsec)
        # fill normalized histograms
        for i in range(1, normdatahist.GetNbinsX()+1):
          poi = 'r_TTW{}{}'.format(i,variablename.strip('_'))
          if len(thisnormxsec[poi])==3:
            error = max(thisnormxsec[poi][1], thisnormxsec[poi][2])
            staterror = 0
          elif len(thisnormxsec[poi])==5:
            errordown = (thisnormxsec[poi][1]**2 + thisnormxsec[poi][3]**2)**(0.5)
            errorup = (thisnormxsec[poi][2]**2 + thisnormxsec[poi][4]**2)**(0.5)
            error = max(errordown, errorup)
            staterror = max(thisnormxsec[poi][1], thisnormxsec[poi][2])
          normdatahist.SetBinContent(i, thisnormxsec[poi][0])
          normdatahist.SetBinError(i, error)
          normstatdatahist.SetBinContent(i, thisnormxsec[poi][0])
          normstatdatahist.SetBinError(i, staterror)
        # divide by bin width
        if not args.absolute:
          for hist in [datahist, statdatahist, normdatahist, normstatdatahist]:
            for i in range(1,hist.GetNbinsX()+1):
              binwidth = hist.GetBinWidth(i)
              hist.SetBinContent(i, hist.GetBinContent(i)/binwidth)
              hist.SetBinError(i, hist.GetBinError(i)/binwidth)

    # make extra infos to display on plot
    extrainfos = []
    for tag in extratags: extrainfos.append(tag)
    if dochi2:
      for nominalhist in nominalhists:
        (chi2,ndof) = calcchi2(datahist,nominalhist)
        chi2info = '#chi2 / ndof = {:.2f} / {}'.format(chi2, ndof)
        if len(nominalhists)>1:
          chi2info = nominalhist.GetTitle() + ' ' + chi2info
        extrainfos.append(chi2info)

    # set plot properties
    figname = variablename
    figname = os.path.join(outputdir,figname)
    yaxdenom = var.axtitle
    if var.shorttitle is not None: yaxdenom = var.shorttitle
    yaxunit = 'fb'
    if var.unit is not None: yaxunit += '/{}'.format(var.unit)
    yaxunit_norm = ''
    if var.unit is not None: yaxunit_norm = '(1/{})'.format(var.unit)
    yaxtitle = 'd#sigma / d{} ({})'.format(yaxdenom,yaxunit)
    yaxtitle_norm = '1/#sigma d#sigma / d{} {}'.format(yaxdenom,yaxunit_norm)
    if args.absolute:
      yaxunit = 'fb'
      yaxtitle = '#sigma ({})'.format(yaxunit)
      yaxtitle_norm = '#sigma (normalized)'
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

    # temporary for plots: change label for legend
    # (to do more cleanly later)
    if len(nominalhists)==1:
        nominalhists[0].SetTitle('Prediction')
    if len(nominalhists_norm)==1:
        nominalhists_norm[0].SetTitle('Prediction')

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
        extrainfos=extrainfos, infosize=15,
        writeuncs=args.writeuncs )

    # make the plot with normalized distributions
    figname_norm = figname+'_norm'
    plotdifferential(
        nominalhists_norm, normdatahist,
        systhists=systhists_norm,
        statdatahist=normstatdatahist,
        figname=figname_norm,
        yaxtitle=yaxtitle_norm, xaxtitle=xaxtitle,
        drawoptions='hist e',
        extracmstext=extracmstext,
        lumitext=lumitext,
        extrainfos=extrainfos, infosize=15,
        writeuncs=args.writeuncs  )
    
    # we write the histograms if requested
    if args.write_rootfiles:
        rootfile.cd()
        print("...and write the data histograms to " + os.path.join(outputdir,"rootfiles/differentialplots_{}_{}.root".format(args.region,args.year)) )
        datahist.SetName( datahist.GetName().replace("nominal","data"))
        datahist.Write()
        statdatahist.SetName( statdatahist.GetName().replace("nominal","datastat"))
        statdatahist.Write()
        normdatahist.SetName( normdatahist.GetName().replace("nominal","normdata"))
        normdatahist.Write()
        normstatdatahist.SetName( normstatdatahist.GetName().replace("nominal","normstatdata"))
        normstatdatahist.Write()
        print("...and try the theory histograms as well")
        if len(nominalhists) == 1:
            nominalhists[0].SetName( nominalhists[0].GetName().replace("nominal","MC"))
            nominalhists[0].Write()
            nominalhists_norm[0].SetName( nominalhists_norm[0].GetName().replace("nominal","normMC"))
            nominalhists_norm[0].Write()
        if len(systhists) == 1:    
            systhists[0].SetName( systhists[0].GetName().replace("nominal","MC_syst"))
            systhists[0].Write()
            systhists_norm[0].SetName( systhists_norm[0].GetName().replace("nominal","normMC_syst"))
            systhists_norm[0].Write()
        elif len(nominalhists) != 1 or len(systhists) != 1:
            print("WARNING: one theory histogram was expected so theory histograms aren't written!")


  # close rootfile if it was requested
  if args.write_rootfiles:
     rootfile.Close()
