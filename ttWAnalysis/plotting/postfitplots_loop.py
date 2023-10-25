####################################################################################
# Submitter that runs postfitplots.py for a number of predefined regions and years #
####################################################################################

import sys
import os
import json
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_10_2_16_UL3' # temporary

inputdir = os.path.abspath(sys.argv[1])
runmode = 'condor'

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)

years = []
#years = ['2016PreVFP','2016PostVFP','2017','2018']
years.append('run2')

npmode = 'npfromdatasplit'
cfmode = 'cffromdata'

dolog = True

variables = '../variables/variables_eventbdt.json'

colormap = 'ttw'

filemode = 'split'

datatag = 'Data'

outputdirname = 'plots_prefit'

confdir = 'postfitplots_confs'

# make configuration directory
if not os.path.exists(confdir):
  os.makedirs(confdir)

# make elementary configurations
channels = []
allyears = ['2016PreVFP','2016PostVFP','2017','2018']
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
    channeldict = ({
      "name": 'channel_{}_{}'.format(year,region),
      "path": inputfile,
      "year": year,
      "region": region
    })
    channels.append(channeldict)
    # write elementary configuration
    if year in years:
      confdict = {'name': channeldict['name'], 'channels': [channeldict]}
      conffile = confdict['name']+'.json'
      conffile = os.path.join(confdir, conffile)
      with open(conffile, 'w') as f: json.dump(confdict, f)

# make run 2 configurations
if 'run2' in years:
  for region in regions:
    thischannels = [c for c in channels if c['name'].split('_',2)[-1]==region]
    confdict = {'name': 'channel_run2_{}'.format(region), 'channels': thischannels}
    conffile = confdict['name']+'.json'
    conffile = os.path.join(confdir, conffile)
    with open(conffile, 'w') as f: json.dump(confdict, f)

# make commands
cmds = []
for year in years:
  print(year)
  for region in regions:
        # find configuration file
        conffile = os.path.join(confdir, 'channel_{}_{}.json'.format(year,region))
        # set output directory
        regiondir = ''
        if filemode=='split': regiondir = region
        subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
        thisoutputdir = '{}_{}_{}_{}'.format(year,region,npmode,cfmode)
        thisoutputdir = os.path.join(inputdir, subdir, outputdirname, thisoutputdir)
        print(thisoutputdir)
        # make the command
        unblind = True
        cmd = 'python postfitplots.py'
        cmd += ' --inputfile '+conffile
        cmd += ' --year '+year
        cmd += ' --region '+region
        cmd += ' --processes all'
        cmd += ' --variables '+variables
        cmd += ' --outputdir '+thisoutputdir
        cmd += ' --datatag '+datatag
        cmd += ' --colormap '+colormap
        if unblind: cmd += ' --unblind'
        if dolog: cmd += ' --dolog'
        cmd += ' --doclean'
        if runmode=='local':
          print('executing '+cmd)
          os.system(cmd)
        elif runmode=='condor':
          print('submitting '+cmd)
          cmds.append(cmd)
        else: raise Exception('ERROR: runmode "{}" not recognized'.format(runmode))

if runmode=='condor':
  ct.submitCommandsAsCondorCluster('cjob_postfitplots', cmds,
                                    cmssw_version=CMSSW_VERSION)
