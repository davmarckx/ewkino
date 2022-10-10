############################################################
# convert scale factors json files to root files with TH2D #
############################################################

import sys
import os
import json
import array
import math
import ROOT

if __name__=='__main__':

  years = ['2016PreVFP','2016PostVFP','2017','2018']
  jsonfname = 'muonRECO_SF_{}.json'
  rootfname = jsonfname.replace('.json','.root')

  for year in years:
    thisjsonfname = jsonfname.format(year)
    thisrootfname = rootfname.format(year)
    print('Now reading file {}...'.format(thisjsonfname))
    # open the json file and read it as a dictionary
    with open(thisjsonfname,'r') as f:
      jsondict = json.load(f)
    # extract the relevant entries
    jsondict = jsondict['NUM_TrackerMuons_DEN_genTracks']['abseta_pt']
    # get the binning
    absetabins = jsondict['binning'][0]['binning']
    etabins = [-v for v in absetabins if v>0.1] + absetabins
    etabins = sorted(etabins)
    ptbins = jsondict['binning'][1]['binning']
    # (pt bins are not relevant here, they refer to the measurement region only;
    #  the same values should be applied in the full muon pt range of interest)
    # make the output histograms
    nomhist = ROOT.TH2D( 'nominal', 'Muon reco scale factors',
                         len(etabins)-1, array.array('f',etabins),
                         1, array.array('f',[10., 200.]) )
    nomhist.SetDirectory(0)
    systhist = nomhist.Clone()
    systhist.SetDirectory(0)
    systhist.SetName('syst')
    systhist.SetTitle('Muon reco systematic uncertainty')
    stathist = nomhist.Clone()
    stathist.SetDirectory(0)
    stathist.SetName('stat')
    stathist.SetTitle('Muon reco statistical uncertainty')
    # fill the output histograms
    for i in range(len(etabins)-1):
      absetavalues = sorted([abs(etabins[i]),abs(etabins[i+1])])
      absetakey = 'abseta:[{},{}]'.format(absetavalues[0], absetavalues[1])
      ptkey = 'pt:[40,60]'
      sf = jsondict[absetakey][ptkey]['value']
      stat = jsondict[absetakey][ptkey]['stat']
      syst = jsondict[absetakey][ptkey]['syst']
      unc = math.sqrt(math.pow(stat,2)+math.pow(syst,2))
      # printouts for testing
      print('Bin {} / {}: value: {}, uncertainty: {}'.format(absetakey,ptkey,sf,unc))
      # write the scale factor as bin content and total uncertainty as bin error
      nomhist.SetBinContent(i+1,1,sf)
      nomhist.SetBinError(i+1,1,unc)
      # write absolute uncertainties as bin content
      systhist.SetBinContent(i+1,1,syst)
      systhist.SetBinError(i+1,1,0)
      stathist.SetBinContent(i+1,1,stat)
      stathist.SetBinError(i+1,1,0)
    # make output file
    f = ROOT.TFile.Open(thisrootfname,'recreate')
    # write output histograms
    nomhist.Write()
    systhist.Write()
    stathist.Write()
    f.Close()
