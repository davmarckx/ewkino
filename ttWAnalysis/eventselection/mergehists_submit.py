#########################################################################################
# Simple submitter that runs mergehists.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to new convention with one file per sample
#       (inclusive in event selection and selection type)

import sys
import os

topdir = sys.argv[1]

years = ['2016PreVFP','2016PostVFP','2017','2018']
#years = ['2016PreVFP']

npmodes = []
#npmodes.append( 'npfromsim' )
npmodes.append( 'npfromdata' )
npmodes.append( 'npfromdatasplit' )

cfmodes = []
#cfmodes.append( 'cffromsim' )
cfmodes.append( 'cffromdata' )

rename = 'processes/rename_processes.json'
renamemode = 'fast'

selectmode = 'custom'

doclip = True

runmode = 'condor'

for year in years:
  for npmode in npmodes:
    for cfmode in cfmodes:
      inputdir = os.path.join(topdir, year)
      outputfile = os.path.join(topdir, year, 'merged_{}_{}'.format(npmode,cfmode), 'merged.root')
      cmd = 'python mergehists.py'
      cmd += ' --directory '+inputdir
      cmd += ' --outputfile '+outputfile
      cmd += ' --npmode '+npmode
      cmd += ' --cfmode '+cfmode
      if rename is not None:
        cmd += ' --rename '+rename
        cmd += ' --renamemode '+renamemode
      cmd += ' --selectmode '+selectmode
      if doclip: cmd += ' --doclip'
      cmd += ' --runmode '+runmode
      print('executing '+cmd)
      os.system(cmd)
