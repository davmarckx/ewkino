######################
# Make postfit plots #
######################
# Using the CombineHarverster functionality,
# see here: http://cms-analysis.github.io/CombineHarvester/post-fit-shapes-ws.html

# The input histograms are supposed to be contained in a single root file.
# The naming of the histograms should be <process name>_<variable name>_<systematic>
# where the systematic is either "nominal" or a systematic name followed by "Up" or "Down".

import sys
import os
import json
import ROOT
import math
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
import argparsetools as apt
from variabletools import read_variables, write_variables_json
from variabletools import DoubleHistogramVariable
sys.path.append(os.path.abspath('../../plotting/python'))
import histplotter as hp
import colors
import infodicts
import regroupdicts


def readPostFitShapesFromWorkspace( histfile ):
  ### read an output file from the PostFitShapesFromWorkspace command
  # input arguments:
  # - histfile: path to the root file to be read,
  #   typically the output of a PostFitShapesFromWorkspace command.
  # output:
  # - a 2-level dict with the same structure as the input file
  #   (i.e. folder name to histogram name to histogram)
  res = {}
  f = ROOT.TFile.Open(histfile)
  gdir = ROOT.gDirectory
  channelkeys = gdir.GetListOfKeys()
  for chkey in channelkeys:
    f.cd()
    f.cd(chkey.GetName())
    thisdir = ROOT.gDirectory
    thisdict = {}
    histkeys = thisdir.GetListOfKeys()
    for key in histkeys:
      keypath = chkey.GetName()+'/'+key.GetName()
      hist = f.Get(keypath)
      try: hist.SetDirectory(0)
      except: 
        print('WARNING in readPostFitShapesFromWorkspace:'
                    +' object with name {} not recognized,'.format(keypath)
                    +' skipping it.')
        continue
      # add hist to dict
      hist.SetTitle(hist.GetName())
      thisdict[hist.GetName()] = hist
    # add dict to total dict
    res[chkey.GetName()] = thisdict
  return res

def runPostFitShapesFromWorkspace( workspace, datacard, fitresultfile=None ):
  ### make and run the PostFitShapesFromWorkspace command
  # input arguments:
  # - workspace: a root workspace, typically obtained from a datacard
  #   with the text2workspace command.
  # - datacard: the datacard corresponding to the workspace.
  # - fitresultfile: a FitDiagnostics result file.
  #   Note: if it is specified, PostFitShapesFromWorkspace will make 
  #   both prefit and postfit histograms; if not, only prefit histograms.
  #   Note: can also be a MultiDimFit result file, but the naming convention
  #   in both types of files is different. Maybe add as an argument later,
  #   for now just assume the distinction can be made based on the file name.
  # output:
  # - an output file is created containing pre- and/or postfit histograms.
  #   name: same as provided workspace but with suffix "_postfitshapes".
  # returns:
  # - a dictionary corresponding to the output file,
  #   containing the following entries:
  #   - 'postfit' (if --total-shapes was added)
  #     with all processes for all channels summed + correctly evaluated uncertainty;
  #     use this only for total uncertainty as it is not split per process.
  #   - '<channelname>_postfit' with the different processes for that channel; 
  #     use only for nominal, not total uncertainty.
  #   - similar entries for prefit
  
  # switch between naming convention for FitDiagnostics and MultiDimFit files
  fitobj = 'fit_s' # in FitDiagnostics file
  if( fitresultfile is not None and 'multidimfit' in fitresultfile.lower() ):
      fitobj = 'fit_mdf' # in MultiDimFit file

  # make the command
  shapesfile = workspace.replace('.root','')+'_postfitshapes.root'
  pfcmd = 'PostFitShapesFromWorkspace'
  pfcmd += ' -d '+datacard
  pfcmd += ' -w '+workspace
  pfcmd += ' -o '+shapesfile
  pfcmd += ' -m 125'
  if fitresultfile is not None:
    pfcmd += ' -f {}:{}'.format(fitresultfile, fitobj)
    pfcmd += ' --postfit'
    pfcmd += ' --sampling'
  pfcmd += ' --print'
  pfcmd += ' --total-shapes'

  # run the command
  print('Info from runPostFitShapesFromWorkspace:'
       +' running following command: {}'.format(pfcmd))
  os.system(pfcmd)

  # read the output
  histdict = readPostFitShapesFromWorkspace( shapesfile )
  return histdict

def cleanplotdir( plotdir, rmroot=True, rmtxt=True, rmjson=True ):
    ### remove all txt and root files from a directory
    if rmroot: os.system('rm {}/*.root'.format(plotdir))
    if rmtxt: os.system('rm {}/*.txt'.format(plotdir))
    if rmjson: os.system('rm {}/*.json'.format(plotdir))


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make postfit plots')
  parser.add_argument('-w', '--workspace', required=True, type=os.path.abspath,
                      help='A ROOT workspace containing all required histograms.')
  parser.add_argument('-d', '--datacard', required=True, type=os.path.abspath,
                      help='The datacard corresponding to the provided workspace.')
  parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath,
                      help='Output directory for the plots (and intermediate files).')
  parser.add_argument('-v', '--variables', default=None,
                      help='Path to json file holding variable definition.')
  parser.add_argument('-y', '--year', default=None,
                      help='Data-taking year (only used for lumi label in plot).')
  parser.add_argument('-r', '--region', default=None,
                      help='Name of signal or control region (only used for label in plot).')
  parser.add_argument('--fitresultfile', default=None, type=apt.path_or_none,
                      help='Combine fit result file (if None, make prefit plots).')
  parser.add_argument('--statworkspace', default=None, type=apt.path_or_none,
                      help='A ROOT workspace with stat-only uncertainties.')
  parser.add_argument('--statdatacard', default=None, type=apt.path_or_none,
                      help='The datacard corresponding to the provided workspace.')
  parser.add_argument('--extrainfos', default=None,
                      help='Comma-separated list of additional info to display on plot.')
  parser.add_argument('--colormap', default=None,
                      help='Name of the color map to use.')
  parser.add_argument('--signals', default=None,
                      help='Comma-separated list of signal process names (will be put on top).')
  parser.add_argument('--regroup_processes', default=False, action='store_true',
                      help='Regroup some processes for less crowded plots.')
  parser.add_argument('--extracmstext', default=None,
                      help='Extra label next to "CMS" text.')
  parser.add_argument('--unblind', default=False, action='store_true',
                      help='Show data on plots.')
  parser.add_argument('--dolog', default=False, action='store_true',
                      help='Make log plots in addition to linear ones.')
  parser.add_argument('--doclean', default=False, action='store_true',
                      help='Remove temporary .root and .txt files after plotting.')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # check the input file
  if not os.path.exists(args.workspace):
    raise Exception('ERROR: input file {} does not exist.'.format(args.workspace))

  # parse the variables
  variable = None
  if args.variables is not None:
    varlist = read_variables(args.variables)
    if len(varlist)!=1:
      raise Exception('ERROR: found {} variables while 1 was expected.'.format(len(varlist)))
    variable = varlist[0]

  # parse the provided list of signal processes
  signals = None
  if args.signals is not None: signals = args.signals.split(',')

  # optionally re-group some nominal histograms
  regroup_processes_dict = None
  if args.regroup_processes:
    regroup_processes_dict = regroupdicts.get_regroup_process_dict(groupid=args.region)

  # parse extra labels to display on plot
  extrainfos = []
  if args.extrainfos is not None: extrainfos = args.extrainfos.split(',')

  # make the output directory
  if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)

  # get a printable version of the region name
  regiondict = infodicts.get_region_dict()
  if args.region is None: regionname = None
  else:
    if args.region not in regiondict.keys():
      msg = 'WARNING: region {} not found in region dict,'.format(args.region)
      msg += ' will write raw region name on plot.'
      print(msg)
    regionname = regiondict.get(args.region, args.region)

  # get a dictionary to match histogram titles to legend entries
  processdict = infodicts.get_process_dict()
 
  # check the fit result file and datacard
  if( args.fitresultfile is not None ):
    if not os.path.exists(args.fitresultfile):
      raise Exception('ERROR: fitresultfile {} does not exist.'.format(args.fitresultfile))
  if not os.path.exists(args.datacard):
    raise Exception('ERROR: datacard {} does not exist'.format(args.datacard))
  
  # set prefit/postfit mode
  mode = 'postfit'
  if args.fitresultfile is None: mode = 'prefit'

  # get the post-fit shapes from the workspace 
  histdict = runPostFitShapesFromWorkspace( 
    args.workspace, args.datacard, fitresultfile=args.fitresultfile )
  # histdict is a dictionary containing the following entries:
  # - 'postfit': histogram with all processes for all channels summed + correct uncertainty;
  #   use this only to get the total uncertainty for plotting.
  #   (note: only present if --total-shapes was added.)
  # - '<channelname>_postfit': histograms with the different processes for that channel; 
  #   use this only for the nominal contributions, not for the total uncertainty.
  # - similar entries as the above for prefit instead of postfit.
   
  # get total data
  datahist = histdict[mode]['data_obs'].Clone()
  
  # get total simulated uncertainty (bin content = sum, bin error = stat+syst error)
  simhisterror = histdict[mode]['TotalProcs'].Clone()
  
  # get nominal sums for each process over all channels
  simhistdict = {}
  for key in histdict.keys():
    if not mode in key: continue
    if key==mode: continue
    thisdict = histdict[key]
    for pkey in thisdict.keys():
      if('Total' in pkey or 'data' in pkey): continue 
      if not pkey in simhistdict: simhistdict[pkey] = thisdict[pkey].Clone()
      else: simhistdict[pkey].Add( thisdict[pkey].Clone() )
  
  # clip all histograms and ignore artificial small values
  ht.cliphistogram(simhisterror,clipboundary=1e-4)
  for hist in simhistdict.values(): ht.cliphistogram(hist,clipboundary=1e-4)
    
  # optional: check that the sum of all individual histograms
  # matches the total histogram as obtained directly
  docheck = True
  if docheck:
    simhistsum = simhistdict[simhistdict.keys()[0]].Clone()
    simhistsum.Reset()
    for pkey in simhistdict: simhistsum.Add(simhistdict[pkey].Clone())
    for i in range(0,simhistsum.GetNbinsX()+2):
      if simhisterror.GetBinContent(i) < 1e-12: reldev = abs(simhistsum.GetBinContent(i))
      else:
        reldev = ( abs(simhistsum.GetBinContent(i)-simhisterror.GetBinContent(i))
                   / simhisterror.GetBinContent(i) )
      if( reldev > 1e-3 ):
        msg = 'WARNING: total prediction and sum-of-processes does not agree'
        msg += ' for workspace {}.'.format(args.workspace)
        msg += ' found following histograms: {}\n{}'.format(
                ht.printhistogram(simhistsum,returnstr=True),
                ht.printhistogram(simhisterror,returnstr=True))
        print(msg)

  # optional: printouts for testing
  doprint = False
  if doprint:
    print('Found following histograms:')
    print('  Simulation:')
    for key,hist in simhistdict.items():
      print('    {}: {} ({})'.format(key, hist.GetName(), hist.GetTitle()))
    print('  Simulation total:')
    ht.printhistogram(simhisterror)
    print('  Data:')
    ht.printhistogram(datahist)

  # find workspace with stat-only uncertainties to get the total statistical uncertainty
  simhiststaterror = None
  dostat = False
  if( args.statworkspace is not None and os.path.exists(args.statworkspace)
      and args.statdatacard is not None and os.path.exists(args.statdatacard) ):
    dostat = True
    # note: it appears best to always take prefit statistical uncertainty,
    #       as the final uncertainty on the signal seems to be incorporated in the 
    #       uncertainty on the signal histogram, even in case of only statistics
    stathistdict = runPostFitShapesFromWorkspace(
                     args.statworkspace, args.statdatacard, fitresultfile=args.fitresultfile)
    simhiststaterror = stathistdict['prefit']['TotalProcs'].Clone()
    ht.cliphistogram(simhiststaterror,clipboundary=1e-4)
  else:
    simhiststaterror = simhisterror.Clone()
    simhiststaterror.Reset()

  # make the errors relative
  for i in range(0, simhisterror.GetNbinsX()+2):
    simhisterror.SetBinContent(i, simhisterror.GetBinError(i))
  if dostat:
    for i in range(0, simhiststaterror.GetNbinsX()+2):
      simhiststaterror.SetBinContent(i, simhiststaterror.GetBinError(i))

  # subtract the statistical errors from the total one
  if dostat:
    for i in range(0, simhisterror.GetNbinsX()+2):
      toterror = simhisterror.GetBinContent(i)
      staterror = simhiststaterror.GetBinContent(i)
      if toterror > staterror:
        systerror = math.sqrt(toterror**2-staterror**2)
        simhisterror.SetBinContent(i, systerror)

  # define output file
  figname = os.path.basename(args.workspace).replace('.root','')
  figname = figname.replace('dc_combined_','')
  figname = figname.replace('datacard_','')
  if mode=='prefit': figname += '_prefit'
  if mode=='postfit': figname += '_postfit'
  figname = os.path.join(args.outputdir,figname)

  # modify histogram titles
  for hist in simhistdict.values():
    title = hist.GetTitle()
    if title in processdict.keys():
      hist.SetTitle(processdict[title])

  # blind data histogram
  if not args.unblind:
    for i in range(0,datahist.GetNbinsX()+2):
      datahist.SetBinContent(i, 0)
      datahist.SetBinError(i, 0)

  # get variable properties
  variablemode = 'single'
  binlabels = None
  labelsize = None
  canvaswidth = None
  canvasheight = None
  p1legendbox = None
  if variable is None:
    xaxtitle = 'Fit variable'
    unit = None
  else:
    if isinstance(variable,DoubleHistogramVariable): variablemode = 'double'
    if variablemode=='single':
      xaxtitle = variable.axtitle
      unit = variable.unit
      if( variable.iscategorical and variable.xlabels is not None ):
        binlabels = variable.xlabels
        labelsize = 15
    elif variablemode=='double':
      xaxtitle = variable.primary.axtitle
      unit = variable.primary.unit
      primarybinlabels = variable.primary.getbinlabels()
      secondarybinlabels = variable.secondary.getbinlabels(extended=True)
      binlabels = (primarybinlabels, secondarybinlabels)
      labelsize = 15
      canvaswidth = 900
      p1legendbox = [0.5, 0.8, 0.9, 0.9]

  # set plot properties
  if( xaxtitle is not None and unit is not None ):
    xaxtitle += ' ({})'.format(unit)
  yaxtitle = 'Number of events'
  lumimap = {'run2':137600, '2016':36300, '2017':41500, '2018':59700,
             '2016PreVFP':19520, '2016PostVFP':16810 }
  if args.year is None: lumi = None
  else:
    if not args.year in lumimap.keys():
      msg = 'WARNING: year {} not recognized,'.format(args.year)
      msg += ' will not write lumi header.'
      print(msg)
    lumi = lumimap.get(args.year,None)
  cmapname = 'default'
  if args.colormap is not None: cmapname = args.colormap
  colormap = colors.getcolormap(style=cmapname)
  if args.year is not None: extrainfos.append( args.year )
  if args.region is not None: extrainfos.append( regionname )

  # make the plot
  hp.plotdatavsmc(figname, datahist, simhistdict.values(),
            groupdict=regroup_processes_dict,
            mcsysthist=simhisterror,
            mcstathist=simhiststaterror,
            dostat=dostat,
            xaxtitle=xaxtitle,
            yaxtitle=yaxtitle,
            colormap=colormap,
            signals=signals,
            extrainfos=extrainfos,
            lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox )

  if args.dolog:
    # make plot in log scale
    figname = figname+'_log'
    hp.plotdatavsmc(figname, datahist, simhistdict.values(),
            groupdict=regroup_processes_dict,
            mcsysthist=simhisterror,
            mcstathist=simhiststaterror,
            dostat=dostat,
            xaxtitle=xaxtitle,
            yaxtitle=yaxtitle,
            colormap=colormap,
            signals=signals,
            extrainfos=extrainfos,
            lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox,
            yaxlog=True )

  # do cleaning 
  if args.doclean: cleanplotdir(args.outputdir)
