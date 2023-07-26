######################################################
# Check cross-section consistency over several lists #
######################################################

import sys
import os
sys.path.append('../../../Tools/python')
from samplelisttools import readsamplelist

if __name__=='__main__':

  samplelists = sys.argv[1:]
  print('Checking sample lists: {}'.format(samplelists))
  samplexsec = {}
  nerrors = 0
  mask = []
  # loop over sample lists
  for samplelist in samplelists:
    # read and loop over samples
    samples = readsamplelist(samplelist)
    for sample in samples.samples:
      # isolate sample name
      sname = sample.name
      if '_crab_' in sname: sname = sname.split('_crab_')[0]
      else:
        # for some private samples, need to use different split
        sname = sname.split('_')[0]
      if sname in mask: continue
      if sname not in samplexsec.keys():
        samplexsec[sname] = sample.xsec
      else:
        xs = samplexsec[sname]
        if xs!=sample.xsec:
          msg = 'WARNING: found incompatible cross-section'
          msg += ' for sample {}: {} vs {}'.format(sname, xs, sample.xsec)
          print(msg)
          mask.append(sname)
          nerrors += 1
  if nerrors == 0:
    print('No inconsistencies found.')
