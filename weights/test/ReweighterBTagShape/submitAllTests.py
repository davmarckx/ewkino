########################################
# useful looper to run all usual tests #
########################################

import sys
import os

if __name__=='__main__':

  donorm = True
  donormjec = True
  nevents = 100000
  basedir = '/pnfs/iihe/cms/store/user/nivanden/skims_v4/'
  files = ([
            {'input': basedir+'2016PreVFP/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_crab_RunIISummer20UL16MiniAODAPV-106X_mcRun2_asymptotic_preVFP_v8-v1_singlelepton_MC_2016_ULpreVFPv6.root',
             'output': 'output_ttw_2016PreVFP.root'},
            {'input': basedir+'2016PostVFP/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_crab_RunIISummer20UL16MiniAOD-106X_mcRun2_asymptotic_v13-v2_singlelepton_MC_2016_ULpostVFPv6.root',
             'output': 'output_ttw_2016PostVFP.root'},
            {'input': basedir+'2017/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_crab_RunIISummer20UL17MiniAOD-106X_mc2017_realistic_v6-v2_singlelepton_MC_2017_ULv6.root',
             'output': 'output_ttw_2017.root'},
            {'input': basedir+'2018/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_crab_RunIISummer20UL18MiniAOD-106X_upgrade2018_realistic_v11_L1v1-v2_singlelepton_MC_2018_ULv6.root',
             'output': 'output_ttw_2018.root'}, 
          ])

  cmds = []
  for el in files:
    if donorm:
      infile = el['input']
      outfile = el['output'].replace('.root','_norm.root')
      cmd = 'python fillTestNormalization.py'
      cmd += ' {} {} {}'.format(infile, outfile, nevents)
      cmds.append(cmd)
    if donormjec:
      infile = el['input']
      outfile = el['output'].replace('.root','_jec.root')
      cmd = 'python fillTestNormalizationJEC.py'
      cmd += ' {} {} {}'.format(infile, outfile, nevents)
      cmds.append(cmd)

  for cmd in cmds:
    os.system(cmd)
