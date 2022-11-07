#########################################################################################
# Simple submitter that runs mergehists.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to new convention with one file per sample
#       (inclusive in event selection and selection type)

import sys
import os

topdir = sys.argv[1]

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = []
npmodes.append( 'npfromsim' )
#npmodes.append( 'npfromdata' )

rename = 'processes/rename_processes_tttt.json'
renamemode = 'fast'

runmode = 'condor'

for year in years:
  for npmode in npmodes:
    inputdir = os.path.join(topdir, year)
    outputfile = os.path.join(topdir, year, 'merged_{}'.format(npmode), 'merged.root')
    cmd = 'python mergehists.py'
    cmd += ' --directory '+inputdir
    cmd += ' --outputfile '+outputfile
    cmd += ' --npmode '+npmode
    if rename is not None:
      cmd += ' --rename '+rename
      cmd += ' --renamemode '+renamemode
    cmd += ' --runmode '+runmode
    print('executing '+cmd)
    os.system(cmd)
