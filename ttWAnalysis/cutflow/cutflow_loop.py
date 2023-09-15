import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
CMSSW_VERSION = '~/CMSSW_12_4_6' # newer version needed for BDT evaluation


if __name__=='__main__':

  runmode = 'condor'

  files = [
    #{'inputfile': '/pnfs/iihe/cms/store/user/llambrec/nanoaod/TTWJetsToLNu-RunIISummer20UL18-miniAODnTuple.root',
     #'nevents': 1e5,
     #'tag': 'ttw'},
    #{'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/Merged_crab_Run2018A-12Nov2019_UL2018-v2_singlelepton_data_UL2018V2.root',
     #'nevents': 1e6,
     #'tag': 'data'},
    #{'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/DoubleMuon_crab_Run2018D-12Nov2019_UL2018-v3_singlelepton_data_UL2018V2.root',
     #'nevents': -1,
     #'tag': 'doublemuon'},
    #{'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/SingleMuon_crab_Run2018D-12Nov2019_UL2018-v8_singlelepton_data_UL2018V2.root',
     #'nevents': -1,
     #'tag': 'singlemuon'},
    {'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/SingleMuon_crab_Run2018C-12Nov2019_UL2018-v2_singlelepton_data_UL2018V2.root',
     'nevents': -1,
     'tag': 'singlemuon'},
    #{'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/EGamma_crab_Run2018D-12Nov2019_UL2018-v4_singlelepton_data_UL2018V2.root',
     #'nevents': -1,
     #'tag': 'egamma'},
    #{'inputfile': '/pnfs/iihe/cms/store/user/nivanden/skims_v5/2018/MuonEG_crab_Run2018D-12Nov2019_UL2018_rsb-v1_singlelepton_data_UL2018V2.root',
     #'nevents': -1,
     #'tag': 'muoneg'}
  ]

  #selection_types = ['tight', 'fakerate', 'chargeflips']
  selection_types = ['fakerate']

  for f in files:
    for t in selection_types:
      cmds = []
      outputname = 'output_cutflow_{}_{}'.format(f['tag'], t)
      # fill the cutflow histogram
      cmd = './fillCutFlow'
      cmd += ' {}'.format(f['inputfile'])
      cmd += ' {}'.format('{}.root'.format(outputname))
      cmd += ' {}'.format(t)
      cmd += ' {}'.format(int(f['nevents']))
      cmd += ' 13'
      cmds.append(cmd)
      # make a plot
      cmd = 'python3 plotCutFlow.py'
      cmd += ' -i {}'.format('{}.root'.format(outputname))
      cmd += ' -o {}'.format('{}_plots'.format(outputname))
      #cmds.append(cmd)
      # run or submit the commands
      if runmode=='local':
        for cmd in cmds: os.system(cmd)
      elif runmode=='condor':
        ct.submitCommandsAsCondorJob('cjob_cutflow', cmds, cmssw_version=CMSSW_VERSION)
