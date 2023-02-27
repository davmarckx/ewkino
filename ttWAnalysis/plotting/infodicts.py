################################################################
# Collection of mappings between code strings and plot strings #
################################################################

def get_region_dict():
  ### get a printable name for each selection region
  res = {}
  res['signalregion_dilepton_inclusive'] = 'Signal region 2L'
  res['signalregion_dilepton_ee'] = 'Signal region 2L (ee)'
  res['signalregion_dilepton_em'] = 'Signal region 2L (em)'
  res['signalregion_dilepton_me'] = 'Signal region 2L (me)'
  res['signalregion_dilepton_mm'] = 'Signal region 2L (mm)'
  res['signalregion_trilepton'] = 'Signal region 3L'
  res['wzcontrolregion'] = 'WZ control region'
  res['zzcontrolregion'] = 'ZZ control region'
  res['zgcontrolregion'] = 'ZG control region'
  res['trileptoncontrolregion'] = '3L control region'
  res['fourleptoncontrolregion'] = '4L control region'
  res['npcontrolregion_dilepton_inclusive'] = 'NP control region 2L'
  res['npcontrolregion_dilepton_ee'] = 'NP control region 2L (ee)'
  res['npcontrolregion_dilepton_em'] = 'NP control region 2L (em)'
  res['npcontrolregion_dilepton_me'] = 'NP control region 2L (me)'
  res['npcontrolregion_dilepton_mm'] = 'NP control region 2L (mm)'
  res['cfcontrolregion'] = 'CF control region'
  return res
