#############################################################
# Tools for working with uncertainties in combine datacards #
#############################################################

import sys
import os
import fnmatch
import math


def get_lumi_unc_dict():
  # note: values to update with latest recommendations!
  res = {}
  res['2016PreVFP'] = {'lumi':1.006,
		       'lumi_2016':1.01}
  res['2016PostVFP'] = res['2016PreVFP']
  res['2017'] = {'lumi':1.009,
		 'lumi_2017':1.02,
		 'lumi_20172018':1.006}
  res['2018'] = {'lumi':1.02,
		 'lumi_2018':1.015,
		 'lumi_20172018':1.002}
  # add root sum square 
  # (only approximate, for naive run2 plotting)
  rss = 0
  for sources in res.values():
    for val in sources.values():
      rss += math.pow(val-1,2)
  rss = 1+math.sqrt(rss)
  res['run2'] = {'lumi': rss}
  return res


def get_lumi_unc_sources( year ):
  return get_lumi_unc_dict()[year]


def get_systematics_to_disable( processes, pnonorm=None, year=None, allyears=None ):
  ### get systematics to disable in a ProcessInfoCollection
  
  rmforall = []
  rmspecific = {}
  for p in processes: rmspecific[p] = []

  # remove overlap between renormalization and factorization scales
  rmforall.append('rScale*')
  rmforall.append('fScale*')
  rmforall.append('rfScales*')
  #rmforall.append('qcdScales*')

  # remove overlap between pdf envelope types
  rmforall.append('pdfShapeEnv*')
  #rmforall.append('pdfShapeRMS*')

  # remove norm uncertainties for specified processes
  if pnonorm is not None:
    for p in pnonorm: 
      rmspecific[p].append('rScaleNorm*')
      rmspecific[p].append('fScaleNorm*')
      rmspecific[p].append('rfScalesNorm*')
      rmspecific[p].append('qcdScalesNorm*')
      rmspecific[p].append('pdfNorm*')
      rmspecific[p].append('isrNorm*')
      rmspecific[p].append('fsrNorm*')

  # remove individual qcd and pdf variations
  # (if not done so before)
  rmforall.append('qcdScalesShapeVar*')
  rmforall.append('pdfShapeVar*')

  # remove overlap between JEC sources
  #rmforall.append('JEC')
  rmforall.append('JECGrouped*')
  rmforall.append('JECGrouped_Total*')

  # remove uncertainties for other years
  # (cleaner, but not strictly needed since they are set to nominal anyway)
  if( year is not None and allyears is not None and year in allyears ):
    for y in allyears:
      if y==year: continue
      rmforall.append('*{}'.format(y))

  return (rmforall, rmspecific)

def remove_systematics_default( processinfo, year=None ):
  ### default sequence removing some shape systematics
  pnonorm = ['WZ','ZZ','TTZ','ZG']
  if 'Nonprompt' in processinfo.plist: pnonorm.append('Nonprompt')
  if 'Chargeflips' in processinfo.plist: pnonorm.append('Chargeflips')
  for p in ['TTW','TTW0','TTW1','TTW2','TTW3','TTW4']:
    if p in processinfo.plist: pnonorm.append(p)
  allyears = ['2016PreVFP', '2016PostVFP', '2017', '2018']
  (rmforall, rmspecific) = get_systematics_to_disable( processinfo.plist, 
    pnonorm=pnonorm, year=year, allyears=allyears )
  removedforall = []
  removedspecific = {}
  # remove some systematics for all processes
  for pattern in rmforall:
    for s in processinfo.slist:
      if fnmatch.fnmatch(s, pattern):
        processinfo.disablesys( s, processinfo.plist )
        removedforall.append(s)
  # remove some systematics for selected processes
  for p,patterns in rmspecific.items():
    removedspecific[p] = []
    for pattern in patterns:
      for s in processinfo.slist:
        if fnmatch.fnmatch(s, pattern):
          processinfo.disablesys( s, [p] )
          removedspecific[p].append(s)
  return (removedforall, removedspecific)


def add_systematics_default( processinfo, year=None ):
  ### default sequence of adding some norm systematics

  # initialize list of all norm systematics
  normsyslist = []

  # add luminosity uncertainty
  if year is not None:
    for source,impact in get_lumi_unc_sources(year).items():
      impacts = {}
      for p in processinfo.plist: impacts[p] = impact
      processinfo.addnormsys( source, impacts )
      if 'Nonprompt' in processinfo.plist:
        processinfo.disablesys( source, ['Nonprompt'] )
      if 'Chargeflips' in processinfo.plist:
        processinfo.disablesys( source, ['Chargeflips'] )
      normsyslist.append(source)

  # add trigger uncertainty
  # to be updated later in case of non-flat uncertainty!
  if year is not None:
    impacts = {}
    for p in processinfo.plist: impacts[p] = 1.02
    source = 'Trigger_{}'.format(year)
    processinfo.addnormsys( source, impacts )
    if 'Nonprompt' in processinfo.plist:
      processinfo.disablesys( source, ['Nonprompt'] )
    if 'Chargeflips' in processinfo.plist:
      processinfo.disablesys( source, ['Chargeflips'] )
    normsyslist.append(source)

  # add individual norm uncertainties
  norms = ({
    'WZ': 1.1,
    'ZZ': 1.1,
    'TTZ': 1.15,
    'ZG': 1.1
  })
  if 'Nonprompt' in processinfo.plist:
    norms['Nonprompt'] = 1.2
  if 'Chargeflips' in processinfo.plist:
    norms['Chargeflips'] = 1.2
  for process,mag in norms.items():
    source = 'Norm_{}'.format(process)
    impacts = {}
    for p in processinfo.plist: impacts[p] = '-'
    impacts[process] = mag
    processinfo.addnormsys( source, impacts )
    normsyslist.append(source)
 
  # return list of all norm systematics that were added
  return normsyslist
