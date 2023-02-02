#######################################
# Simple looper for makeyieldtable.py #
#######################################

import sys
import os

inputdir = sys.argv[1]

regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['signalregion_trilepton']: regions.append(r)

years = []
years = ['2016PreVFP','2016PostVFP','2017','2018']
years.append('run2')

signal = 'TTW'

unblind = True

cmds = []
for year in years:
  for region in regions:
    subdir = os.path.join(year, region, 'merged_npfromdata_cffromdata')
    inputfile = os.path.join(inputdir, subdir, 'merged.root')
    if not os.path.exists(inputfile):
      print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
      continue
    thisoutputfile = 'yields_{}_{}.txt'.format(year,region)
    cmd = 'python makeyieldtable.py'
    cmd += ' --inputfile '+inputfile
    cmd += ' --region '+region
    cmd += ' --outputfile '+thisoutputfile
    cmd += ' --datatag Data'
    cmd += ' --signals '+signal 
    #if unblind: cmd += ' --unblind'
    print('executing '+cmd)
    os.system(cmd)
