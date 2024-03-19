####################################################################################
# Submitter that runs postfitplots.py for a number of predefined regions and years #
####################################################################################

import sys
import os
import json
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_13_combine/CMSSW_10_2_13'

inputdir = os.path.abspath(sys.argv[1])
runmode = 'local'

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)

years = []
#years = ['2016PreVFP','2016PostVFP','2017','2018']
years = ['2016PreVFP']
#years.append('run2')

npmode = 'npfromdatasplit'
cfmode = 'cffromdata'

dolog = True

variables = '../variables/variables_eventbdt.json'

colormap = 'ttw'

signals = ['TTW']

regroup_processes = True

unblind = True

filemode = 'split'

datatag = 'Data'

outputdirname = 'output_test'

confdir = 'postfitplots_confs'

# make configuration directory
if not os.path.exists(confdir): os.makedirs(confdir)

# make all elementary configurations
configurations = {}
if 'run2' in years: allyears = ['2016PreVFP','2016PostVFP','2017','2018']
else: allyears = years
for year in allyears:
  for region in regions:
    # find the input file
    regiondir = ''
    if filemode=='split': regiondir = region
    subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
    inputfile = os.path.join(inputdir, subdir, 'merged.root')
    if not os.path.exists(inputfile):
      print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
      continue
    # make a dict for this channel
    name = 'channel_{}_{}'.format(year,region)
    channeldict = ({
      "name": name,
      "path": inputfile,
      "year": year,
      "region": region
    })
    configurations[name] = channeldict

# write elementary configurations to json files
for year in years:
    for region in regions:
      name = 'channel_{}_{}'.format(year,region)
      confdict = {'name': name, 'channels': [configurations[name]]}
      conffile = name+'.json'
      conffile = os.path.join(confdir, conffile)
      with open(conffile, 'w') as f: json.dump(confdict, f)

# write run2 configurations to json files
if 'run2' in years:
  for region in regions:
    channelnames = ['channel_{}_{}'.format(year,region) for year in allyears]
    channels = [configurations[name] for name in channelnames]
    name = 'channel_run2_{}'.format(region)
    confdict = {'name': name, 'channels': channels}
    conffile = name+'.json'
    conffile = os.path.join(confdir, conffile)
    with open(conffile, 'w') as f: json.dump(confdict, f)

# get a list of variables
with open(variables, 'r') as f:
  varlist = json.load(f)
varnames = [var['name'] for var in varlist]

# make commands
cmds = []
for year in years:
  for region in regions:
        # find configuration file
        name = 'channel_{}_{}'.format(year,region)
        conffile = os.path.join(confdir, name+'.json')
        # set output directory
        regiondir = ''
        if filemode=='split': regiondir = region
        subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
        thisoutputdir = '{}_{}_{}_{}'.format(year,region,npmode,cfmode)
        thisoutputdir = os.path.join(inputdir, subdir, outputdirname, thisoutputdir)
        # make the workspace command
        wcmd = 'python postfitplots_makeworkspaces.py'
        wcmd += ' -i {}'.format(conffile)
        wcmd += ' -v {}'.format(variables)
        wcmd += ' -o {}'.format(thisoutputdir)
        wcmd += ' --datatag {}'.format(datatag)
        wcmd += ' --dummysystematics' # only for testing!
        # make the plot commands
        pcmds = []
        for varname in varnames:
          workspacename = 'dc_combined_{}_var_{}.root'.format(name, varname)
          workspace = os.path.join(thisoutputdir, name, workspacename)
          varfile = os.path.join(thisoutputdir, name, 'variable_{}.json'.format(varname))
          pcmd = 'python postfitplots.py'
          pcmd += ' -w {}'.format(workspace)
          pcmd += ' -d {}'.format(workspace.replace('.root', '.txt'))
          pcmd += ' -v {}'.format(varfile)
          pcmd += ' -o {}'.format(os.path.join(thisoutputdir, name))
          pcmd += ' --statworkspace {}'.format(workspace.replace('.root', '_stat.root'))
          pcmd += ' --statdatacard {}'.format(workspace.replace('.root', '_stat.txt'))
          pcmd += ' --year '+year
          pcmd += ' --region '+region
          pcmd += ' --colormap '+colormap
          pcmd += ' --signals {}'.format(','.join(signals))
          if regroup_processes: pcmd += ' --regroup_processes'
          if unblind: pcmd += ' --unblind'
          if dolog: pcmd += ' --dolog'
          pcmd += ' --extracmstext Preliminary'
          pcmd += ' --doclean'
          pcmds.append(pcmd)
        # make a temporary executable grouping the workspace and plot commands
        exename = 'temp_postfitplots_{}_{}.sh'.format(year, region)
        with open(exename, 'w') as f:
          f.write(wcmd+'\n')
          for pcmd in pcmds: f.write(pcmd+'\n')
        # add temporary executable to list of commands
        cmds.append('bash {}'.format(exename))

# run or submit the commands
if runmode=='local':
  for cmd in cmds:
    print('executing '+cmd)
    os.system(cmd)
elif runmode=='condor':
  ct.submitCommandsAsCondorCluster('cjob_postfitplots', cmds,
                                    cmssw_version=CMSSW_VERSION)
