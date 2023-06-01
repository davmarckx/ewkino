import sys
import os
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
CMSSW_VERSION = '~/CMSSW_10_6_28'


if __name__=='__main__':

  topfolder = '../analysis/output_runanalysis_robustness_bdt'
  outputdir = 'output_systs_robustness_bdt'
  years = ['run2']
  regions = ['signalregion_dilepton_inclusive',#'signalregion_dilepton_mm','signalregion_dilepton_me','signalregion_dilepton_em','signalregion_dilepton_ee','signalregion_dilepton_plus','signalregion_dilepton_minus',
             'cfcontrolregion','fourleptoncontrolregion','trileptoncontrolregion','npcontrolregion_dilepton_inclusive','npcontrolregion_dilepton_mm',
             'npcontrolregion_dilepton_me','npcontrolregion_dilepton_em','npcontrolregion_dilepton_ee','wzcontrolregion','zzcontrolregion','zgcontrolregion']
  # normal processes
  processes = ({ 
                 'allprocesses': 'all',
		 'ttwprocess': 'TTW',
                 'ttbar': 'TT',
                 'ttz': 'TTZ',
                 'ttx': 'TTX',
                 'ttg': 'TTG',
                 'np_m':"Nonprompt(m)",
                 'np_e':"Nonprompt(e)"
              })

  # split variables process names
  #splitvarlist = [ i.name for i in read_variables( '../variables/variables_particlelevel_single.json' )]
  #splitbinslist = [ int(i.nbins) for i in read_variables( '../variables/variables_particlelevel_single.json' )]  
  #processes = ({})
  #for i in range(len(splitvarlist)):
  #  names = ""
  #  for binnr in range(splitbinslist[i]): 
  #    names += "TTW_"+splitvarlist[i]+str(binnr)+","
  #  processes["split_on_"+splitvarlist[i]] = names[:-1]
  #print(processes)
  
  
  #variables = '../variables/variables_main.json'
  variables = '../variables/variables_eventbdt.json'
  datatag = 'Data'
  includetags = ({ 
                   #'allsys': None,                           #never run all en combo at the same time as jobs
                   'jets': 'JEC,JER,Uncl',
                   'leptons': 'electron,muon',
                   'scales': 'Scale',
                   'pdf': 'pdf',
                   'other': 'pileup,prefire'
                })
  includeraw = True
  runLocal = False

  cmds = []
  rawcmds = []
  for year in years:
    for region in regions:
      for pkey,pval in processes.items():
        for includekey,includeval in includetags.items():
          inputfile = os.path.join(topfolder,year,region,'merged_npfromdatasplit_cffromdata/merged.root')
          thisoutputdir = os.path.join(outputdir,year,region,pkey,includekey)
          cmd = 'python plotsystematics.py'
          cmd += ' --inputfile ' + inputfile
          cmd += ' --year ' + year
          cmd += ' --region ' + region
          cmd += ' --processes ' + pval
          cmd += ' --variables ' + variables
          cmd += ' --outputdir ' + thisoutputdir 
          cmd += ' --datatag ' + datatag
          if includeval is not None: cmd += ' --includetags ' + includeval
          cmd += ' --tags {},{}'.format(year,region)
          cmds.append(cmd)
          if includeraw:
            thisoutputdir = os.path.join(outputdir,year,region,pkey,includekey+'_raw')
            rawcmd = 'python plotsystematics.py'
            rawcmd += ' --inputfile ' + inputfile 
            rawcmd += ' --year ' + year 
            rawcmd += ' --region ' + region
            rawcmd += ' --processes ' + pval
            rawcmd += ' --variables ' + variables
            rawcmd += ' --outputdir ' + thisoutputdir
            rawcmd += ' --datatag ' + datatag
            if includeval is not None: rawcmd += ' --includetags ' + includeval
            rawcmd += ' --tags {},{},Raw'.format(year,region)
            rawcmd += ' --rawsystematics'
            rawcmds.append(rawcmd)

  if runLocal:
    for cmd in cmds:
      os.system(cmd)
    if includeraw:
      for cmd in rawcmds:
        os.system(cmd)
  else:
    #print(cmds)
    ct.submitCommandsAsCondorCluster( 'cjob_plotsyst', cmds,
                                      cmssw_version=CMSSW_VERSION )
    if includeraw:
      ct.submitCommandsAsCondorCluster( 'cjob_plotsyst', rawcmds,
                                      cmssw_version=CMSSW_VERSION )
