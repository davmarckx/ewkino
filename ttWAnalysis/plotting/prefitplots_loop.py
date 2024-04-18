########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
import json

# input directory
inputdir = sys.argv[1]
runmode = 'condor'

# choose what to run on
regions = ['auto']

years = []
#years = ['2016PreVFP','2016PostVFP','2017','2018']
years.append('run2')

npmodes = ['npfromdatasplit']
cfmodes = ['cffromdata']

unblindmodes = []
unblindmodes.append('blind')
unblindmodes.append('unblind')

# choose systematics
dummysystematics = False

rawsystematics = False

# choose variables

dolog = True

#variables = '../variables/variables_main.json' # single variables
#variables = '../variables/variables_main_reduced.json' # single variables, slightly reduced set
#variables = '../variables/variables_eventbdt.json' # single variable (bdt only)
#variables = '../variables/variables_inputfeatures.json'
variables = '../variables/variables_particlelevel_double.json' # double variables

# other settings

colormap = 'ttw'
regroup_processes = True
filemode = 'split'
datatag = 'Data'
excludetags = None
signals = ['TTW']
for i in range(1,10): signals.append('TTW{}'.format(i))

# define how to handle the splitting of particle level processes
# for plots without particle level splitting, use
# - splitprocess = None
# - splitvariables = None
# for plots with particle level splitting using the same variable as the plotting variable, use
# - splitprocess = <process> (i.e. usually 'TTW')
# - splitvariables = None
# for plots with particle level splitting using another variable than the plotting variable,
# yet to test and document later.
#splitprocess = None # no splitting
splitprocess = 'TTW' # show this splitted process
splitvariables = None # no splitting or automatic splitting
#splitvariables = '../variables/variables_particlelevel_single.json' # to test

# read split variable file if needed
if( splitvariables is not None and isinstance(splitvariables,str) ):
  with open(splitvariables) as json_file:
    splitvariables = [d['variable'] for d in json.load(json_file)]


cmds = []
if 'auto' in years: years = os.listdir(inputdir)
for year in years:
  for npmode in npmodes:
    for cfmode in cfmodes:
      if 'auto' in regions: regions = os.listdir(os.path.join(inputdir,year))
      for region in regions:
        for unblindmode in unblindmodes:
          unblind = False
          if unblindmode=='unblind': unblind = True
          if splitvariables is None: splitvariables = [None]
          for splitvariable in splitvariables:
            regiondir = ''
            if filemode=='split': regiondir = region
            subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
            inputfile = os.path.join(inputdir, subdir, 'merged.root')
            if not os.path.exists(inputfile):
              print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
              continue
            thisoutputdir = '{}_{}_{}_{}'.format(year,region,npmode,cfmode)
            if not unblind: thisoutputdir += '_blind'
            if rawsystematics: thisoutputdir += '_rawsystematics'
            if dummysystematics: thisoutputdir += '_dummysystematics'
            thisoutputdir = os.path.join(inputdir, subdir, 'plots', thisoutputdir)
            cmd = 'python prefitplots.py'
            cmd += ' --inputfile '+inputfile
            cmd += ' --year '+year
            cmd += ' --region '+region
            cmd += ' --processes all'
            cmd += ' --variables '+variables
            cmd += ' --outputdir '+thisoutputdir
            cmd += ' --datatag '+datatag
            cmd += ' --colormap '+colormap
            if regroup_processes: cmd += ' --regroup_processes'
            if excludetags is not None: cmd += ' --excludetags ' +excludetags
            if splitprocess is not None: cmd += ' --splitprocess '+splitprocess
            if splitvariable is not None: cmd += ' --splitvariable '+splitvariable
            if unblind: cmd += ' --unblind'
            else: cmd += ' --blind'
            if rawsystematics: cmd += ' --rawsystematics'
            if dummysystematics: cmd += ' --dummysystematics'
            if dolog: cmd += ' --dolog'
            if signals is not None:
              cmd += ' --signals '+' '.join(signals)
            if runmode=='local':
              print('executing '+cmd)
              os.system(cmd)
            elif runmode=='condor':
              print('submitting '+cmd)
              cmds.append(cmd)
            else: raise Exception('ERROR: runmode "{}" not recognized'.format(runmode))

if runmode=='condor':
  ct.submitCommandsAsCondorCluster('cjob_prefitplots', cmds,
                                    cmssw_version=CMSSW_VERSION)
