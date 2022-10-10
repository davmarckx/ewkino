#############################################
# get the muon reco scale factor json files #
#############################################
# does not work, probably due to login required...
# instead, download them manually...

import sys
import os

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  baseurl = 'https://gitlab.cern.ch/cms-muonPOG/muonefficiencies/-/raw/master/Run2/UL'
  fileurl = 'NUM_TrackerMuons_DEN_genTracks_Z_abseta_pt.json'
  url = os.path.join(baseurl,'{}',fileurl)
  fname = 'muonRECO_SF_{}.json'

  for year in years:
    thisurl = url.format(year)
    thisfname = fname.format(year)
    cmd = 'wget {}'.format(thisurl)
    print(cmd)
    os.system(cmd)
    cmd = 'mv {} {}'.format(fileurl, thisfname)
    print(cmd)
    os.system(cmd)
