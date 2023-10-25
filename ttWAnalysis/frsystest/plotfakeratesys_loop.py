import sys
import os
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
CMSSW_VERSION = '~/CMSSW_10_6_29'


if __name__=='__main__':

  inputdir = sys.argv[1]

  years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
  #years = ['2018']

  variables = '../variables/variables_main_reduced.json'
  #variables = '../variables/variables_yield.json'

  runmode = 'condor' 
 
  cmds = []
  for year in years:
    for eventselection in sorted(os.listdir(os.path.join(inputdir,year))):
      for selectiontype in sorted(os.listdir(os.path.join(inputdir,year,eventselection))):
        for process in ['TTW', 'Obs']:   
            thisdir = os.path.join(inputdir,year,eventselection,selectiontype)
            inputfile = os.path.join(thisdir, 'merged.root')
            thisoutputdir = os.path.join(thisdir, 'plots_{}_{}_{}_{}'.format(
              year, eventselection, selectiontype, process))
            cmd = 'python ../plotsystematics/plotsystematics.py'
            cmd += ' --inputfile ' + inputfile
            cmd += ' --year ' + year
            cmd += ' --region ' + eventselection+'_'+selectiontype
            cmd += ' --processes ' + process
            cmd += ' --variables ' + variables
            cmd += ' --outputdir ' + thisoutputdir 
            cmd += ' --datatag dummy'
            cmd += ' --noclip'
            cmd += ' --rawsystematics'
            cmds.append(cmd)
  
  if runmode=='local':
    for cmd in cmds: os.system(cmd)
  else:
    ct.submitCommandsAsCondorCluster( 'cjob_plotsyst', cmds,
                                      cmssw_version=CMSSW_VERSION )
