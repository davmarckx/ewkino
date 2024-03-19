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
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt
from variabletools import read_variables, write_variables_json
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
  
def make_workspaces( channels, variables, outputdir, workspacename, 
                     signals=[], adddata=True, datatag='Data',
                     rawsystematics=False, dummysystematics=False,
                     includestatonly=False ):
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

    # make ProcessInfoCollection
    (PIC, shapesyslist, normsyslist, _) = makeProcessInfoCollection(
      path, year, region, variable, ['all'],
      signals=signals, adddata=adddata, datatag=datatag,
      rawsystematics=rawsystematics, dummysystematics=dummysystematics,
      verbose=verbose )

    # write the datacard
    print('Writing full datacard...')
    writedatacard( outputdir, name, PIC,
                 path, variable,
                 shapesyslist=shapesyslist, lnnsyslist=normsyslist,
                 rateparamlist=[], ratio=[],
                 automcstats=10,
                 writeobs=False,
                 writeallhists=True )
    
    # rename the datacard taking into account the specific variable
    oldname = os.path.join(outputdir,'datacard_'+name+'.txt')
    newname = oldname.replace('.txt','_var_'+variable+'.txt')
    os.system('mv {} {}'.format(oldname,newname))

    # write the stat-only datacard
    if includestatonly:
      print('Writing statistics-only datacard...')
      writedatacard( outputdir, name, PIC,
                 path, variable,
                 shapesyslist=[], lnnsyslist=[],
                 rateparamlist=[], ratio=[],
                 automcstats=10,
                 writeobs=False,
                 writeallhists=True )
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


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make postfit plots')
  parser.add_argument('-i', '--inputfile', required=True, type=os.path.abspath,
                      help='Input configuration file in json format.')
  parser.add_argument('-v', '--variables', required=True, type=os.path.abspath,
                      help='Path to json file holding variable definitions.')
  parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath,
                      help='Output directory for the workspace and datacard.')
  parser.add_argument('--datatag', default='data',
                      help='Name of the data process')
  parser.add_argument('--rawsystematics', default=False, action='store_true',
                      help='Take the systematics from the input file without modifications'
                          +' (i.e. no disablings and no adding of norm uncertainties).')
  parser.add_argument('--dummysystematics', default=False, action='store_true',
                      help='Use dummy systematics.')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))
    
  # check the input file
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))

  # parse the variables
  varlist = read_variables(args.variables)

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
  variablenames = [var.name for var in varlist]
  wspacepaths = make_workspaces( channels, variablenames,
		    thisoutputdir, comboname,
                    signals=['TTW'], 
                    datatag=args.datatag, 
                    rawsystematics=args.rawsystematics,
                    dummysystematics=args.dummysystematics,
                    includestatonly=True )

  # loop over variables to write individual variable json files
  for variable, wspacepath in zip(varlist, wspacepaths):
      wspacedir = os.path.dirname(wspacepath)
      variablefile = os.path.join(wspacedir,'variable_{}.json'.format(variable.name))
      write_variables_json( [variable], variablefile, builtin=True )
