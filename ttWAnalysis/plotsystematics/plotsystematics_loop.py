import sys
import os

if __name__=='__main__':

  topfolder = '../analysis/output_20230418_single'
  outputdir = 'output_test'
  years = ['run2']
  regions = ['signalregion_dilepton_inclusive']
  processes = ({ 
                 'allprocesses': 'all',
		 'ttwprocess': 'TTW'
              })
  variables = '../variables/variables_eventbdt.json'
  datatag = 'Data'
  includetags = ({ 
                   'allsys': None,
                   #'jets': 'JEC,JER,Uncl',
                   #'leptons': 'electron,muon',
                   #'scales': 'Scale',
                   #'pdf': 'pdf',
                   #'other': 'pileup,prefire'
                })
  includeraw = True

  cmds = []
  for year in years:
    for region in regions:
      for pkey,pval in processes.items():
        for includekey,includeval in includetags.items():
          inputfile = os.path.join(topfolder,year,region,'merged_npfromdata_cffromdata/merged.root')
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
            cmd = 'python plotsystematics.py'
            cmd += ' --inputfile ' + inputfile 
            cmd += ' --year ' + year 
            cmd += ' --region ' + region
            cmd += ' --processes ' + pval
            cmd += ' --variables ' + variables
            cmd += ' --outputdir ' + thisoutputdir
            cmd += ' --datatag ' + datatag
            if includeval is not None: cmd += ' --includetags ' + includeval
            cmd += ' --tags {},{},Raw'.format(year,region)
            cmd += ' --rawsystematics'
            cmds.append(cmd)

  for cmd in cmds:
    os.system(cmd)
