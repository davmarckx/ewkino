#############################################################
# small utility for getting the electron reco scale factors #
#############################################################

# Always refer to this page for the latest recommendations:
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun2LegacyAnalysis
# The implementation here will not work anymore if the location
# of the scale factor files is changed in the future.

# Note: does not yet work!
# Wget seems to download a webpage with a redirect link
# instead of the actual file...
# Probably due to login required.
# For now, download and copy the files here manually,
# then run this script to perform the proper moving and name change.

import os
import sys

if __name__=='__main__':

    # settings
    baseurl = 'https://twiki.cern.ch/twiki/pub/CMS/EgammaUL2016To2018/'
    years = ({
		'UL2016preVFP': '2016PreVFP',
		'UL2016postVFP': '2016PostVFP',
		'UL2017': '2017',
		'UL2018': '2018'
	    })
    ptbins = ({
		'ptBelow20': 'ptBelow20',
		'ptAbove20': 'ptAbove20'
	    })
    targetdir = '../electronRecoSF'

    # loop over years and ptbins
    for yearin, yearout in years.items():
	for ptbinin, ptbinout in ptbins.items():
	    fname = 'egammaEffi_{}.txt_EGM2D_{}.root'.format(
			ptbinin, yearin)
	    fpath = os.path.join(baseurl,fname)
	    tname = 'electronRECO_SF_{}_{}.root'.format(yearout,ptbinout)
	    tpath = os.path.join(targetdir,tname)
	    getcmd = 'wget {}'.format(fpath)
	    mvcmd = 'mv {} {}'.format(fname,tpath)
	    #print(getcmd)
	    #os.system(getcmd)
	    print(mvcmd)
	    os.system(mvcmd)
