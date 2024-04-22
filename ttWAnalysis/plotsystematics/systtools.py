import sys
import os
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
from systplotter import findbyname


def category( systematic ):
    if( systematic.startswith('JEC')
        or systematic.startswith('JER')
        or systematic.startswith('Uncl') ): return 'jetmet'
    if( systematic.startswith('muon') ): return 'muon'
    if( systematic.startswith('electron') ): return 'electron'
    if( systematic=='pileup'
        or systematic=='prefire'
        or systematic=='trigger' ): return systematic
    if( systematic.startswith('bTag') ): return 'b-tagging'
    if( systematic.startswith('fScale')
        or systematic.startswith('rScale')
        or systematic.startswith('rfScale')
        or systematic.startswith('qcdScales') ): return 'scales'
    if( systematic.startswith('isr')
        or systematic.startswith('fsr') ): return 'parton shower'
    if( systematic.startswith('pdf') ): return 'pdf'
    if( systematic.startswith('lumi') ): return 'lumi'
    if( systematic.startswith('njets')
        or systematic.startswith('nbjets') ): return 'njets'
    if( systematic.startswith('Norm_') ): return 'norm'
    if( systematic.startswith('efakerate') ): return 'fakerate (e)'
    if( systematic.startswith('mfakerate') ): return 'fakerate (m)'
    return systematic


def get_jec_rms_list( hislist ):
  ### helper function
  # add root-sum-square of the individual JEC variations
  # make sure to exclude the superfluous JEC variations in the selection above
  # or the rss will be too large!
  res = []
  nominalhist = histlist[findbyname( histlist, 'nominal' )]
  jecall = ht.selecthistograms(histlist,mustcontainall=['JECAll','Down'])[1]
  jecgrouped = ht.selecthistograms(histlist,mustcontainall=['JECGrouped','Down'])[1]
  for i,hist in enumerate(jecall):
    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
    jecall[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
    jecall[i].SetName( hist.GetName().replace('Down','Max') )
  for i,hist in enumerate(jecgrouped):
    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
    jecgrouped[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
    jecgrouped[i].SetName( hist.GetName().replace('Down','Max') )
  if( len(jecall)>0 ):
    jecallup = nominalhist.Clone()
    jecallup.Add( ht.rootsumsquare(jecall) )
    jecallup.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
                      + 'JECSqSumAllUp' )
    jecalldown = nominalhist.Clone()
    jecalldown.Add( ht.rootsumsquare(jecall), -1 )
    jecalldown.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
                      + 'JECSqSumAllDown' )
    res.append(jecallup)
    res.append(jecalldown)
  if( len(jecgrouped)>0 ):
    jecgroupedup = nominalhist.Clone()
    jecgroupedup.Add( ht.rootsumsquare(jecgrouped) )
    jecgroupedup.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
                          'JECGrouped')] + 'JECSqSumGroupedUp' )
    jecgroupeddown = nominalhist.Clone()
    jecgroupeddown.Add( ht.rootsumsquare(jecgrouped), -1 )
    jecgroupeddown.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
                            'JECGrouped')] + 'JECSqSumGroupedDown' )
    res.append(jecgroupedup)
    res.append(jecgroupeddown)
  return res
