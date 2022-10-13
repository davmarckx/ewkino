#############################################
# get the muon reco scale factor json files #
#############################################

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
  years = ['2016PreVFP','2016PostVFP','2017','2018']
  baseurl = 'https://gitlab.cern.ch/cms-muonPOG/muonefficiencies/-/raw/master/Run2/UL'
  fileurl = 'NUM_TrackerMuons_DEN_genTracks_Z_abseta_pt.json'
  url = os.path.join(baseurl,'{}',fileurl)
  targetdir = '../MuonRecoSF'
  fname = 'muonRECO_SF_{}.json'

  for year in years:
    thisurl = url.format(year)
    thisfname = fname.format(year)
    thisfname = os.path.join(targetdir,thisfname)
    cmd = 'wget {}'.format(thisurl)
    print(cmd)
    os.system(cmd)
    cmd = 'mv {} {}'.format(fileurl, thisfname)
    print(cmd)
    os.system(cmd)
