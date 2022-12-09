##########################################
# Make impact plot for a given workspace #
##########################################

import os
import sys
import json
import argparse
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_patch1' # temporary


def makeimpactplot(workspace, usedata=False, expectsignal=True, 
                   poi='r', runmode='condor'):
    ### run the commands to produce the impact plots for a given combine workspace (.root)
    
    # define a name for this workspace
    workspace = os.path.abspath(workspace) # absolute path to workspace
    workspace_name = workspace.split('/')[-1].replace('.root','') # simple name of workspace
    # define a name appendix depending on the options
    appendix = ''
    if poi!='r': appendix += '_'+str(poi)
    if usedata: appendix += '_obs'
    if not expectsignal: appendix += '_bkg'
    # create a working directory
    subdir = workspace.replace('.root',appendix) # absolute path to subdirectory for this job
    if os.path.exists(subdir): os.system('rm -r '+subdir)
    os.makedirs(subdir)
    json = workspace_name+appendix+'_impacts.json'
    fig = workspace_name+appendix+'_impacts'
    # make the Impacts command
    command = 'combineTool.py -M Impacts -d '+workspace
    command += ' -m 100' # seems to be required argument, put any value?
    command += ' --rMin 0 --rMax 5'
    #command += ' --cminDefaultMinimizerStrategy 0'
    command += ' --robustFit=1'
    if poi!='r': command += ' --redefineSignalPOIs {}'.format(poi)
    if( not usedata and expectsignal ): command += ' -t -1 --expectSignal 1'
    elif( not usedata and not expectsignal ): command += ' -t -1 --expectSignal 0'
    # make the list of commands to execute
    setdir = 'cd {}'.format(subdir)
    initfit = command + ' --doInitialFit'
    impacts = command + ' --doFits --parallel 10'
    output = command + ' --output {}'.format(json)
    plot = 'plotImpacts.py -i {} -o {}'.format(json,fig)
    commands = [setdir,initfit,impacts,output,plot]
    # submit the job
    ct.submitCommandsAsCondorJob('cjob_impactplot', commands,
                                 cmssw_version=CMSSW_VERSION)


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Make impact plots')
  parser.add_argument('--datacarddir', required=True, type=os.path.abspath)
  parser.add_argument('--workspace', default=None)
  parser.add_argument('--runelementary', default=False, action='store_true')
  parser.add_argument('--runcombinations', default=False, action='store_true')
  parser.add_argument('--usedata', default=False, action='store_true')
  parser.add_argument('--expectsignal', default=False, action='store_true')
  parser.add_argument('--runmode', default='condor', choices=['condor','local'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.datacarddir))
  if args.workspace is not None:
    wspaceabs = os.path.join(args.datacarddir,args.workspace)
    if not os.path.exists(wspaceabs):
      raise Exception('ERROR: workspace {} not found in input directory {}.'.format(
		      args.workspace, args.datacarddir))

  # hard-coded arguments (for now)    
  poi = 'r'
  force = False

  # find workspaces to run on
  workspaces = []
  if args.workspace is not None:
    workspaces = [args.workspace]
  else:
    elwspaces = ([ f for f in os.listdir(args.datacarddir)
                   if (f.endswith('.root') 
                       and (f.startswith('datacard_') ) ) ])
    combwspaces = ([ f for f in os.listdir(args.datacarddir)
                     if (f.endswith('.root')
                         and (f.startswith('dc_combined_') ) ) ])
    if args.runelementary:
      for w in elwspaces: workspaces.append(w)
    if args.runcombinations:
      for w in combwspaces: workspaces.append(w)
      
  # run
  for workspace in workspaces:
    wspaceabs = os.path.join(args.datacarddir,workspace)
    makeimpactplot(wspaceabs, usedata=args.usedata, 
                   expectsignal=args.expectsignal,
                   poi=poi, runmode=args.runmode)
