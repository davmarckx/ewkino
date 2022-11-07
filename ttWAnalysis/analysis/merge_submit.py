####################################################################################
# Simple submitter that runs merge.py for a number of predefined regions and years #
####################################################################################
# Note: can work with both submission conventions:
#       - one job per sample, inclusive in event selections and selection types.
#       - multiple jobs per sample, separate for event selections and selection types.
# In both conventions, the histograms are assumed to be named as follows:
# <process name>_<event selection>_<selection type>_<variable>_<systematic>

import sys
import os

topdir = sys.argv[1]

years = ['2016PreVFP','2016PostVFP','2017','2018']

regions = []
for r in ['wzcontrolregion']: regions.append(r)
for r in ['zzcontrolregion','zgcontrolregion']: regions.append(r)

#npmodes = ['npfromsim','npfromdata']
npmodes = ['npfromsim']

rename = '../eventselection/processes/rename_processes_tttt.json'

runmode = 'condor'

submitcombined = True

for year in years:
  for npmode in npmodes:
    if submitcombined:
      # case of combined submission with one job per sample
      inputdir = os.path.join(topdir, year)
      outputfile = os.path.join(topdir, year, 'merged_{}'.format(npmode), 'merged.root')
      cmd = 'python ../eventselection/mergehists.py'
      cmd += ' --directory '+inputdir
      cmd += ' --outputfile '+outputfile
      cmd += ' --npmode '+npmode
      if rename is not None: cmd += ' --rename '+rename
      cmd += ' --runmode '+runmode
      print('executing '+cmd)
      os.system(cmd)
    else:
      # case of separate submission split in event selections and selection types
      for region in regions:
        outputfile = os.path.join(topdir, year, region, npmode, 'merged.root')
        cmd = 'python merge.py'
        cmd += ' --directory ' + topdir
        cmd += ' --year ' + year
        cmd += ' --region ' + region 
        cmd += ' --outputfile ' + outputfile
        cmd += ' --npmode ' + npmode
        if rename is not None: cmd += ' --rename ' + rename 
        cmd += ' --runmode ' + runmode
        print('executing '+cmd)
        os.system(cmd)
