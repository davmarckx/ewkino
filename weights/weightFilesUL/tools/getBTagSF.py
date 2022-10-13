######################################################################
# small utility for getting the b-tagging scale factor uncertainties #
######################################################################

# Always refer to this page for the latest recommendations:
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun2LegacyAnalysis
# The implementation here will not work anymore if the location
# of the scale factor files is changed in the future.

# Note: it seems the BTV Twiki pages have been superseded by this GitLab page:
# https://gitlab.cern.ch/groups/cms-btv/-/wikis/Subgroups/PerformanceCalibration/ScaleFactorRecommendations
# Get new scale factors from there! (but this script was not yet updated.)

# Note: does not yet work!
# Wget seems to download a webpage with a redirect link
# instead of the actual file...
# For now, download and copy the files here manually,
# then run this script to perform the proper moving and name change.

import os
import sys

if __name__=='__main__':

    # settings
    years = ({
		'UL16preVFP': ('2016PreVFP','v2'),
		'UL16postVFP': ('2016PostVFP','v3'),
		'UL17': ('2017','v3'),
		'UL18': ('2018','v2')
	    })
    targetdir = '../bTagSF'

    # loop over years and ptbins
    for yearin, (yearout,version) in years.items():
	    furl = 'https://twiki.cern.ch/twiki/pub/CMS/BtagRecommendation106X{}'.format(yearin)
	    fname = 'reshaping_deepJet_106X{}_{}.csv'.format(yearin,version)
	    fpath = os.path.join(furl,fname)
	    tname = 'bTagReshaping_unc_{}.csv'.format(yearout)
	    tpath = os.path.join(targetdir,tname)
	    getcmd = 'wget {}'.format(fpath)
	    mvcmd = 'mv {} {}'.format(fname,tpath)
	    #print(getcmd)
	    #os.system(getcmd)
	    print(mvcmd)
	    os.system(mvcmd)
