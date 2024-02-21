########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
import json

inputdir = sys.argv[1]
runmode = 'condor'


regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
#for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
#for r in ['signalregion_trilepton']: regions.append(r)
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
#for r in ['npcontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['ee','em','me','mm']: regions.append('npcontrolregion_dilepton_{}'.format(r))
#for r in ['nplownjetscontrolregion_dilepton_inclusive']: regions.append(r)
#for r in ['cfcontrolregion']: regions.append(r)
#for r in ['cfjetscontrolregion']: regions.append(r)

years = []
#years = ['2016PreVFP','2016PostVFP','2017','2018']
years.append('run2')
EFT_WCs = ["EFTsm","EFTcQq81","EFTcQq83","EFTcQei","EFTcQl3i","EFTcQlMi","EFTcQq11","EFTcQq13","EFTcbW","EFTcpQ3","EFTcpQM","EFTcpt","EFTcptb","EFTctG","EFTctW","EFTctZ","EFTctei","EFTctlSi","EFTctlTi","EFTctli","EFTctp","EFTctq1","EFTctq8"]
#EFT_WCs = ["_nJets","_nBJets","_HT","_jetPtSubLeading"]
#EFT_WCs = [""]
npmodes = ['npfromdatasplit']
cfmodes = ['cffromdata']

unblind = True

dummysystematics = False

rawsystematics = False

dolog = False

#variables = '../variables/variables_main.json' # single variables
#variables = '../variables/variables_main_reduced.json' # single variables, slightly reduced set
#variables = '../variables/variables_fitcr.json'
#variables = '../variables/variables_particlelevel_single_BINSTUDY_reduced.json'
#variables = '../variables/variables_eventbdt.json' # single variable (bdt only)
#variables = '../variables/variables_inputfeatures.json'
#variables = '../variables/variables_particlelevel_double.json' # double variables
variables = '../variables/variables_particlelevel_double_BINSTUDY.json' # double variables for bin study
#variables = '../variables/variables_particlelevel_single_BINSTUDY.json'

colormap = 'ttw'

filemode = 'split'

datatag = 'Data'

excludetags = None

signals = ['TTW']
for i in range(1,10): signals.append('TTW{}'.format(i))

# split variables can either be a list or a json file
#splitvariables = [''] # no splitting
splitvariables = '../variables/variables_particlelevel_single.json'
#splitvariables = ['differential']

splitprocess = 'TTW'


if isinstance(splitvariables,str):
    with open(splitvariables) as json_file:
        splitvariables = [d['variable'] for d in json.load(json_file)]

cmds = []
if 'auto' in years: years = os.listdir(inputdir)
for year in years:
  for npmode in npmodes:
    for cfmode in cfmodes:
      if 'auto' in regions: regions = os.listdir(os.path.join(inputdir,year))
      for region in regions:
        for splitvar in splitvariables:
         for eft in EFT_WCs:
          regiondir = ''
          if filemode=='split': regiondir = region
          subdir = os.path.join(year, regiondir, 'merged_{}_{}'.format(npmode,cfmode))
          inputfile = os.path.join(inputdir, subdir, 'merged_scaled{}.root'.format(eft))
          if not os.path.exists(inputfile):
            print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
            continue
          thisoutputdir = '{}_{}_{}_{}'.format(year,region,npmode,cfmode)
          if not unblind: thisoutputdir += '_blind'
          if rawsystematics: thisoutputdir += '_rawsystematics'
          if dummysystematics: thisoutputdir += '_dummysystematics'
          thisoutputdir = os.path.join(inputdir, subdir, 'plots{}'.format(eft), thisoutputdir)
          cmd = 'python prefitplots.py'
          cmd += ' --inputfile '+inputfile
          cmd += ' --year '+year
          cmd += ' --region '+region
          cmd += ' --processes all'
          cmd += ' --variables '+variables
          cmd += ' --outputdir '+thisoutputdir
          cmd += ' --datatag '+datatag
          cmd += ' --colormap '+colormap
          if excludetags is not None:
            cmd += ' --excludetags ' +excludetags
          if splitvar != '':
            cmd += ' --splitvariable '+splitvar
            cmd += ' --splitprocess '+splitprocess
          if unblind: cmd += ' --unblind'
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
                                    cmssw_version="/user/dmarckx/CMSSW_10_6_28")
