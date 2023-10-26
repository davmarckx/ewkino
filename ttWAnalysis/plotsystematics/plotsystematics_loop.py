import sys
import os
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
CMSSW_VERSION = '~/CMSSW_10_6_29'


if __name__=='__main__':


  topfolder = '../analysis/output_inclusive_main/'
  outputdir = 'output_test'
  years = ['run2']
  regions = []
  regions.append('signalregion_dilepton_inclusive')
  #regions.append('signalregion_dilepton_mm')
  #regions.append('signalregion_dilepton_me')
  #regions.append('signalregion_dilepton_em')
  #regions.append('signalregion_dilepton_ee')
  #regions.append('signalregion_dilepton_plus')
  #regions.append('signalregion_dilepton_minus')
  #regions.append('cfcontrolregion')
  #regions.append('fourleptoncontrolregion')
  #regions.append('trileptoncontrolregion')
  #regions.append('npcontrolregion_dilepton_inclusive')
  #regions.append('npcontrolregion_dilepton_mm')
  #regions.append('npcontrolregion_dilepton_me')
  #regions.append('npcontrolregion_dilepton_em')
  #regions.append('npcontrolregion_dilepton_ee')
  #regions.append('wzcontrolregion')
  #regions.append('zzcontrolregion')
  #regions.append('zgcontrolregion')
  processes = ({ 
                 'allprocesses': 'all',
		 #'ttwprocess': 'TTW',
                 #'ttbar': 'TT',
                 #'ttz': 'TTZ',
                 #'ttx': 'TTX',
                 #'ttg': 'TTG',
                 #'np_m':"Nonprompt(m)",
                 #'np_e':"Nonprompt(e)"
              })

  # split variables process names
  '''splitvarlist = [ i.name for i in read_variables( '../variables/variables_particlelevel_single.json' )]
  splitbinslist = [ int(i.nbins) for i in read_variables( '../variables/variables_particlelevel_single.json' )]  
  processes = ({})
  for i in range(len(splitvarlist)):
    names = ""
    for binnr in range(splitbinslist[i]): 
      names += "TTW_"+splitvarlist[i]+str(binnr)+","
    processes["split_on_"+splitvarlist[i]] = names[:-1]
  print(processes)'''
  
  
  #variables = '../variables/variables_main.json'
  variables = '../variables/variables_eventbdt.json'
  
  datatag = 'Data'
  includetags = ({ 
                   'allsys': 'none',
                   'jets': 'JEC,JER,Uncl',
                   'grouped':'JECGrouped', 
                   'leptons': 'electron,muon',
                   'scales': 'rScale,fScale',
                   'qcdscales': 'qcdScales,rfScales',
                   'ps': 'isr,fsr',
                   'pdf': 'pdf',
                   'other': 'pileup,prefire,trigger',
                   'bTag': 'bTag_shape'
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
          cmds.append(cmd)
          cmd += ' --outputdir ' + thisoutputdir
          if includeraw:
            rawcmd = cmd + ' --rawsystematics'
            rawcmd += ' --outputdir ' + thisoutputdir+'_rawsystematics'
            rawcmds.append(rawcmd)

  if runLocal:
    for cmd in cmds+rawcmds:
      print(cmd)
      #os.system(cmd)
  else:
    ct.submitCommandsAsCondorCluster( 'cjob_plotsyst', cmds,
                                      cmssw_version=CMSSW_VERSION )
    ct.submitCommandsAsCondorCluster( 'cjob_plotsyst', rawcmds,
                                      cmssw_version=CMSSW_VERSION )
