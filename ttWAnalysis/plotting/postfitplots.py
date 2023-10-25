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
import combinetools as cbt
from variabletools import read_variables, write_variables_json
from variabletools import DoubleHistogramVariable
sys.path.append(os.path.abspath('../../plotting/python'))
import histplotter as hp
import colors
import infodicts
sys.path.append(os.path.abspath('../combine/'))
from makedatacard import makeProcessInfoCollection 
from datacardtools import writedatacard


def parse_json_input( inputfile ):
  ### get the contents of a json input configuration file
  # the json file is supposed to contain the configuration to run on;
  # it should contain a dictionary with the following fields:
  # - name: a name that will be used throughout the plotting,
  #         typically something like 'channel_<year>_<selection region>'.
  # - channels: a list of dicts with following fields:
  #   - name: name of this channel
  #   - year: year of this channel
  #   - region: selection region of this channel
  #   - path: path to file containing all required histograms
  # for most cases, the dict will have just one channel 
  # (one year + selection region),
  # but multiple channels are possible e.g. for combining years
  with open(args.inputfile,'r') as f:
    channelcombo = json.load(f)
  if not isinstance(channelcombo, dict):
    raise Exception('ERROR in parse_json_input:'
      +' object loaded from json file is a {},'.format(type(channelcombo))
      +' while a dict was expected.')
  if( 'name' not in channelcombo.keys() 
    or 'channels' not in channelcombo.keys() ):
    raise Exception('ERROR in parse_json_input:'
      +' dict loaded from json file does not contain required keys.')
  comboname = str(channelcombo['name'])
  channels = channelcombo['channels']
  if not isinstance(channels, list):
    raise Exception('ERROR in parse_json_input:'
      +' "channels" value must be a list, but found {}'.format(type(channels)))
  if len(channels)==0:
    raise Exception('ERROR in parse_json_input:'
      +' "channels" list has length 0.')
  for channel in channels:
    if( 'name' not in channel.keys() 
      or 'year' not in channel.keys()
      or 'region' not in channel.keys()
      or 'path' not in channel.keys() ):
      raise Exception('ERROR in parse_json_input:'
        +' channel does not contain required keys.')
    if not os.path.exists(channel['path']):
      raise Exception('ERROR in parse_json_input:'
        +' file {} does not exist'.format(channel['path'])
        +' (in channel {})'.format(channel['name']))
  return channelcombo
  
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
  # - datacard: the datacard corresponding to the workspace
  # - fitresultfile: a FitDiagnostics result file;
  #   if it is specified, PostFitShapesFromWorkspace will make 
  #   both prefit and postfit histograms,
  #   if not, only prefit histograms.
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
  shapesfile = workspace.replace('.root','')+'_postfitshapes.root'
  pfcmd = 'PostFitShapesFromWorkspace'
  pfcmd += ' -d '+datacard
  pfcmd += ' -w '+workspace
  pfcmd += ' -o '+shapesfile
  pfcmd += ' -m 125'
  if fitresultfile is not None:
    pfcmd += ' -f '+fitresultfile+':fit_s'
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

def make_workspace( channels, variables, outputdir, workspacename, 
                    signals=[], adddata=True, datatag='Data', 
                    rawsystematics=False, includestatonly=False ):
  ### make combined workspaces as input for PostFitShapesFromWorkspace
  # a single combined workspace for all the channels is made (e.g. to combine years)
  # but split in variables (i.e. one separate workspace for each variable)
  # input arguments:
  # - channels = a list of dicts encoding the channels to combine in the workspace
  #   note: each dict must have the keys 'path' (to a root file), 'name', 'year', 'region';
  #   see also parse_json_input.
  # - variables = a collection of HistogramVariables for which to make plots
  # - outputdir = directory where to put the output
  # - workspacename = name for the workspace to be created (should not contain .root suffix)
  # - signals: process names to consider as signal
  # output:
  # - elementary datacards for each channel for each variable.
  #   name: "datacard_<channel name>_var_<variable name>.txt" (see writedatacard)
  # - same datacards without systematics (with suffix "_stat")
  # - copies of the required root files (as specified in the "path" field of each channel dict)
  #   name: "histograms_<channel name>.root" (see writedatacard)
  # - datacards for each variable for the combined channels.
  #   name: "dc_combined_<channel name>_var_<variable name>.txt"
  # - same datacards without systematics (with suffix "_stat")
  # - workspaces of all combined datacards.
  # returns:
  # - a list of the names of the created workspaces (excluding the "_stat" ones).

  # define the variable for which to run the full procedure,
  # with the creation of a ProcessInfoCollection and corresponding datacard
  # for each channel.
  # (for all other variables, can apply a trick (for speed)
  #  where the datacards are simply copied and the variable name changed.)
  variable = variables[0]

  # write the elementary datacards for all channels for this variable
  for cnum,channel in enumerate(channels):
    # get region, year and file to use
    name = channel['name']
    region = channel['region']
    year = channel['year']
    path = channel['path']
    print('Making datacard for channel {}'.format(name)
      +' (with histogram input file {})...'.format(path))
    # set verbosity level
    verbose = False
    if cnum==0:
      verbose = True
      print('Printing ProcessInfoCollection as an example:')

    # make ProcessInfoCollection
    (PIC, shapesyslist, normsyslist) = makeProcessInfoCollection(
      path, year, region, variable, ['all'],
      signals=signals, adddata=adddata, datatag=datatag,
      rawsystematics=rawsystematics, verbose=verbose )

    # write the datacard
    print('Writing full datacard...')
    writedatacard( outputdir, name, PIC,
                 path, variable,
                 #datatag=datatag,
                 shapesyslist=shapesyslist, lnnsyslist=normsyslist,
                 rateparamlist=[], ratio=[],
                 automcstats=10,
                 writeobs=True,
                 writeallhists=True,
                 autof=False )
    
    # rename the datacard taking into account the specific variable
    oldname = os.path.join(outputdir,'datacard_'+name+'.txt')
    newname = oldname.replace('.txt','_var_'+variable+'.txt')
    os.system('mv {} {}'.format(oldname,newname))

    # write the stat-only datacard
    if includestatonly:
      print('Writing statistics-only datacard...')
      writedatacard( outputdir, name, PIC,
                 path, variable,
                 #datatag=datatag,
                 shapesyslist=[], lnnsyslist=[],
                 rateparamlist=[], ratio=[],
                 automcstats=10,
                 writeobs=True,
                 writeallhists=True,
                 autof=False )
      # rename the datacard taking into account the specific variable
      oldname = os.path.join(outputdir,'datacard_'+name+'.txt')
      newname = oldname.replace('.txt','_var_'+variable+'_stat.txt')
      os.system('mv {} {}'.format(oldname,newname))

  # create identical datacards with only the name of the variable changed
  print('Copying datacards and changing variable name...')
  for var in variables:
    for channel in channels:
      statonlysuffixes = ['']
      if includestatonly: statonlysuffixes = ['','_stat']
      for statonlysuffix in statonlysuffixes:
        name = channel['name']
        oldname = os.path.join(outputdir,'datacard_'+name
                  +'_var_'+variable+statonlysuffix+'.txt')
        newname = oldname.replace(variable,var)
        f = open(oldname,'r')
        fdata = f.read()
        f.close()
        fdata = fdata.replace(variable,var)
        f = open(newname,'w')
        f.write(fdata)
        f.close()

  # for each variable, create a combined workspace
  combinationdict = {}
  combinationnames = [] # holding the keys to combinationdict in an ordered way
  # loop over variables
  for var in variables:
    statonlysuffixes = ['']
    if includestatonly: statonlysuffixes = ['','_stat']
    for statonlysuffix in statonlysuffixes:
      combinationname = ('dc_combined_'+workspacename
                         +'_var_'+var+statonlysuffix+'.txt')
      combinationdict[combinationname] = {}
      combinationnames.append(combinationname)
      for c in channels:
        name = c['name']
        combinationdict[combinationname]['datacard_'+name+'_var_'+var+statonlysuffix+'.txt'] = name
  # make the combinations
  print('Making combined datacards...')
  cbt.makecombinedcards( outputdir, combinationdict )
  # convert datacards to workspaces
  res = []
  for wsname in combinationnames:
    print('Making workspace {}...'.format(wsname))
    onelinecmd = ''
    for c in cbt.get_workspace_commands( outputdir, wsname ):
      onelinecmd += c+'; '
    os.system(onelinecmd)
    if not '_stat' in wsname:
      res.append( os.path.join(outputdir,wsname.replace('.txt','.root')) )
  return res

def cleanplotdir( plotdir, rmroot=True, rmtxt=True, rmjson=True ):
    ### remove all txt and root files from a directory
    if rmroot: os.system('rm {}/*.root'.format(plotdir))
    if rmtxt: os.system('rm {}/*.txt'.format(plotdir))
    if rmjson: os.system('rm {}/*.json'.format(plotdir))


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make postfit plots')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--year', required=True)
  parser.add_argument('--region', required=True)
  parser.add_argument('--processes', required=True,
                      help='Comma-separated list of process tags to take into account;'
                          +' use "all" to use all processes in the input file.')
  parser.add_argument('--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--fitresultfile', default=None, type=apt.path_or_none,
                      help='Combine fit result file; if None, make prefit plots.')
  parser.add_argument('--datatag', default='data')
  parser.add_argument('--includetags', default=None,
                      help='Comma-separated list of systematic tags to include')
  parser.add_argument('--excludetags', default=None,
                      help='Comma-separated list of systematic tags to exclude')
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  parser.add_argument('--colormap', default='default')
  parser.add_argument('--extracmstext', default='Preliminary')
  parser.add_argument('--unblind', action='store_true')
  parser.add_argument('--dolog', action='store_true',
                      help='Make log plots in addition to linear ones.')
  parser.add_argument('--rawsystematics', action='store_true',
                      help='Take the systematics from the input file without modifications'
                          +' (i.e. no disablings and no adding of norm uncertainties).')
  parser.add_argument('--doclean', action='store_true',
                      help='Remove .root and .txt files after plotting.')
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

  # get a printable version of the region name
  regiondict = infodicts.get_region_dict()
  if args.region in regiondict.keys():
    regionname = regiondict[args.region]
  else:
    print('WARNING: region {} not found in region dict,'.format(args.region)
          +' will write raw region name on plot.')
    regionname = args.region
 
  # if the input file is a ROOT workspace, run on it directly
  if args.inputfile.endswith('.root'):

    # check arguments
    datacard = args.inputfile.replace('.root','.txt')
    if not os.path.exists(args.inputfile):
      raise Exception('ERROR: workspace {} does not exist'.format(args.inputfile))
    if( args.fitresultfile is not None and not os.path.exists(args.fitresultfile) ):
      raise Exception('ERROR: fitresultfile {} does not exist'.format(args.fitresultfile))
    if not os.path.exists(datacard):
      raise Exception('ERROR: datacard {} does not exist'.format(datacard))
    mode = 'postfit'
    if args.fitresultfile is None: mode = 'prefit'
    if len(varlist)!=1:
      raise Exception('ERROR: found {} variables while 1 was expected.'.format(len(varlist)))
    variable = varlist[0]
      
    histdict = runPostFitShapesFromWorkspace( 
      args.inputfile, datacard, fitresultfile=args.fitresultfile )
    # - the histdict contains an entry 'postfit' (if --total-shapes was added)
    # with all processes for all channels summed + correctly evaluated uncertainty;
    # use this only for total uncertainty (not nominal) as it is not split per process.
    # - the histdict contains entries <channelname>_postfit
    # with the different processes for that channel; 
    # use only for nominal, not total uncertainty.
    # also similar entries for prefit
   
    # printouts for testing
    #for key,value in histdict.items():
    #  print(key)
    #  print(value)

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
        if not pkey in simhistdict:
	  simhistdict[pkey] = thisdict[pkey].Clone()
	else:
	  simhistdict[pkey].Add( thisdict[pkey].Clone() )
    # clip all histograms and ignore artificial small values
    ht.cliphistogram(simhisterror,clipboundary=1e-4)
    for hist in simhistdict.values(): ht.cliphistogram(hist,clipboundary=1e-4)
    # make the sum of all histograms in the dict
    simhistsum = simhistdict[simhistdict.keys()[0]].Clone()
    simhistsum.Reset()
    for pkey in simhistdict: simhistsum.Add(simhistdict[pkey].Clone())
    for i in range(0,simhistsum.GetNbinsX()+2):
      if( abs(simhistsum.GetBinContent(i)-simhisterror.GetBinContent(i))
	  > 1e-3*simhisterror.GetBinContent(i) ):
	print('WARNING: total simulation and sum-of-processes does not agree'
		+' for workspace {}.'.format(args.inputfile)
		+' found following histograms: {}\n{}'.format(
		ht.printhistogram(simhistsum,returnstr=True),
		ht.printhistogram(simhisterror,returnstr=True)))

    # printouts for testing
    print('Found following histograms:')
    print('  Simulation:')
    for key,hist in simhistdict.items():
      print('    {}: {} ({})'.format(key, hist.GetName(), hist.GetTitle()))
    print('  Simulation total:')
    ht.printhistogram(simhisterror)
    print('  Data:')
    ht.printhistogram(datahist)

    # check if stat-only workspace exists and run on it
    statworkspace = args.inputfile.replace('.root','_stat.root')
    statdatacard = datacard.replace('.txt','_stat.txt')
    simhiststaterror = None
    dostat = True
    if os.path.exists(statworkspace):
      print('Trying to read stat-only uncertainties...')
      # note: it appears best to always take prefit statistical uncertainty,
      #       as the final uncertainty on the signal seems to be incorporated in the 
      #       uncertainty on the signal histogram, even in case of only statistics
      stathistdict = runPostFitShapesFromWorkspace( statworkspace, statdatacard,
			        fitresultfile=args.fitresultfile )
      # get total simulated uncertainty (bin content = sum, bin error = stat+syst error)
      simhiststaterror = stathistdict['prefit']['TotalProcs'].Clone()
      ht.cliphistogram(simhiststaterror,clipboundary=1e-4)
    else:
        print('WARNING: stat-only workspace not found, considering only total uncertainty!')
        simhiststaterror = simhisterror.Clone()
        simhiststaterror.Reset()
        dostat = False

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
    figname = os.path.basename(args.inputfile).replace('.root','')
    figname = figname.replace('dc_combined_','')
    figname = figname.replace('datacard_','')
    figname = os.path.join(args.outputdir,figname)

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
    if not args.year in lumimap.keys():
      print('WARNING: year {} not recognized,'.format(args.year)
            +' will not write lumi header.')
    lumi = lumimap.get(args.year,None)
    colormap = colors.getcolormap(style=args.colormap)
    extrainfos = []
    extrainfos.append( args.year )
    extrainfos.append( regionname )

    # make the plot
    hp.plotdatavsmc(figname, datahist, simhistdict.values(),
	    mcsysthist=simhisterror,
            mcstathist=simhiststaterror,
            dostat=dostat,
	    xaxtitle=xaxtitle,
	    yaxtitle=yaxtitle,
	    colormap=colormap,
            extrainfos=extrainfos,
	    lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox )

    if args.dolog:
      # make plot in log scale
      figname = figname+'_log'
      hp.plotdatavsmc(figname, datahist, simhistdict.values(),
            mcsysthist=simhisterror,
            mcstathist=simhiststaterror,
            dostat=dostat,
            xaxtitle=xaxtitle,
            yaxtitle=yaxtitle,
            colormap=colormap,
            extrainfos=extrainfos,
            lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox,
            yaxlog=True )

  # if the input file is a json, make the workspaces and then run on them.
  elif args.inputfile.endswith('.json'):
    # read the channel combination
    print('Reading channel info...')
    channelcombo = parse_json_input( args.inputfile )
    comboname = str(channelcombo['name'])
    channels = channelcombo['channels']
    print('Found following channel configuration:')
    print('  Name: {}'.format(comboname))
    print('  Channels: {}'.format(channels))
    # set output directory
    thisoutputdir = os.path.join(args.outputdir, comboname)
    if not os.path.exists(thisoutputdir): os.makedirs(thisoutputdir)
    # make workspaces
    wspacepaths = make_workspace( channels, variablenames,
		    thisoutputdir, comboname,
                    signals=['TTW'], 
                    adddata=args.unblind,
                    datatag=args.datatag, rawsystematics=args.rawsystematics,
                    includestatonly=True )
    # loop over variables
    for variable, wspacepath in zip(varlist, wspacepaths):
      wspacedir = os.path.dirname(wspacepath)
      # write the variable json
      variablefile = os.path.join(wspacedir,'variable_{}.json'.format(variable.name))
      write_variables_json( [variable], variablefile, builtin=True )
      # define the postfit plot command for this variable
      command = 'python postfitplots.py'
      command += ' --inputfile ' + wspacepath
      command += ' --year ' + args.year
      command += ' --region ' + args.region
      command += ' --processes ' + args.processes
      command += ' --variables ' + variablefile
      command += ' --outputdir ' + wspacedir
      if args.fitresultfile is not None: 
        command += ' --fitresultfile ' + args.fitresultfile
      command += ' --datatag ' + args.datatag
      if args.includetags is not None:
        command += ' --includetags ' + args.includetags
      if args.excludetags is not None:
        command += ' --excludetags ' + args.excludetags
      if args.tags is not None:
        command += ' --tags ' + args.tags
      command += ' --colormap ' + args.colormap
      command += ' --extracmstext ' + args.extracmstext
      if args.unblind: command += ' --unblind'
      if args.dolog: command += ' --dolog'
      if args.rawsystematics: command += ' --rawsystematics'
      if args.doclean: command += ' --doclean'
      os.system( command )
    # do cleaning 
    # (keep separate from loop above 
    # since directory is usually the same for all workspaces)
    if args.doclean:
      cleaneddirs = []
      for wspacepath in wspacepaths:
        wspacedir = os.path.dirname(wspacepath)
        if wspacedir not in cleaneddirs: 
          cleanplotdir(wspacedir)
          cleaneddirs.append(wspacedir)

  else:
    raise Exception('ERROR in postfitplotter.py: input file type not recognized.')
  
  sys.stderr.write('###done###\n')
