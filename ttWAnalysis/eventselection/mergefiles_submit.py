#########################################################################################
# Simple submitter that runs mergefiles.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to old convention with one file per sample
#       and per event selection and per selection type!
#       will probably not be used anymore in new convention,
#       use mergehists instead!
# note: might need updates in naming before being usable again!

import sys
import os

topdir = sys.argv[1]

regions = []
for r in ['wzcontrolregion']: regions.append(r)
for r in ['zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromsim','npfromdata']

for year in years:
  for region in regions:
    for npmode in npmodes:
      inputdir = os.path.join(topdir, year+'_'+region)
      outputfile = os.path.join(topdir, year+'_'+region, 
                    'merged_{}'.format(npmode), 'merged.root')
      cmd = 'python mergefiles.py'
      cmd += ' --directory {}'.format(inputdir)
      cmd += ' --outputfile {}'.format(outputfile)
      cmd += ' --npmode {}'.format(npmode)
      print('executing '+cmd)
      os.system(cmd)
