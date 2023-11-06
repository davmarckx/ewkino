############################################
# Find which output file gives a Nan value #
############################################
# See notes 27/10/2023 and 03/11/2023
# Run with CMSSW_12_X and python3!

import sys
import os
import numpy as np
import uproot


def checkfile(rootfile, histnames):
    values = []
    with uproot.open(rootfile) as f:
        for histname in histnames:
            hist = f[histname].values()
            values.append(hist)
    values = np.array(values)
    if np.any(np.isinf(values)): return False
    if np.any(np.isnan(values)): return False
    return True


if __name__=='__main__':

    topdir = '../analysis/output_20231025_single'
    years = ([
      #'2016PreVFP',
      #'2016PostVFP',
      #'2017',
      '2018'
    ])
    regions = ([
      'fourleptoncontrolregion',
      #'npcontrolregion_dilepton_inclusive',
      #'signalregion_dilepton_inclusive',
      #'trileptoncontrolregion'
    ])
    selectiontype = 'mfakerate'
    variable = '_nJetsNZCat'
    systematic = 'efakerateetaUp'
    histnames = ([
      'nonpromptm_{}_{}_{}_{}'.format('REGION', selectiontype, variable, systematic)
    ])

    # loop over configurations
    for year in years:
      for region in regions:

        # find all files
        inputdir = os.path.join(topdir, year, region, selectiontype)
        rootfiles = os.listdir(inputdir)

        # format histnames
        thishistnames = [h.replace('REGION', region) for h in histnames]
    
        # loop over files and check histograms
        nfail = 0
        for rootfile in rootfiles:
          rootfile = os.path.join(inputdir, rootfile)
          fpass = checkfile(rootfile, thishistnames)
          if not fpass:
            print('  Found issue in file: {}'.format(rootfile))
            nfail += 1

        # print report
        print('Report for {} / {}:'.format(year, region))
        print('  Found {} problematic files out of {} checked.'.format(nfail, len(rootfiles)))
