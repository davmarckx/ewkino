###############################################################################
# Merge the output of eventbinner over several years to make full run 2 plots #
###############################################################################
# Note: naive implementation of simply adding nominal histograms together;
#       taking into account systematics and correlations to be implemented later
#       using combine tools.
# Note: should be run after mergehists_submit.py.

import sys
import os

if __name__=='__main__':

  topdir = sys.argv[1]
  years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
  npmodes = ['npfromsim', 'npfromdata']
  cfmodes = ['cffromsim', 'cffromdata']

  # loop over combinations
  for npmode in npmodes:
    for cfmode in cfmodes:
      # check all required files
      allfiles = True
      hfiles = []
      for year in years:
        f = os.path.join(topdir,year,'merged_{}_{}'.format(npmode,cfmode),'merged.root')
        if not os.path.exists(f):
          msg = 'ERROR: file {} not does not exist (for year {});'.format(f,year)
          msg += ' skipping merging for npmode {} cfmode {}.'.format(npmode,cfmode)
          print(msg)
          allfiles = False
        else: hfiles.append(f)
      if not allfiles: continue
      # define output file
      outdir = os.path.join(topdir,'run2','merged_{}_{}'.format(npmode,cfmode))
      if not os.path.exists(outdir): os.makedirs(outdir)
      outf = os.path.join(outdir,'merged.root')
      # do hadd
      cmd = 'hadd -f {}'.format(outf)
      for f in hfiles: cmd += ' {}'.format(f)
      os.system(cmd)
