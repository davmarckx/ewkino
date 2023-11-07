########################
# tools for color maps #
########################

import sys
import os
import argparse
import ROOT
sys.path.append('../../plotting/python')
import histplotter as hp

def getcolormap( style='default' ):
  ### get a color map (see definitions below)
  style=style.lower()
  if(style=='default'): return getcolormap_default()
  if(style=='tttt'): return getcolormap_tttt()
  if(style=='systematics'): return getcolormap_systematics()
  if(style=='systematics_grouped'): return getcolormap_systematics_grouped()
  if(style=='eft'): return getcolormap_eft()
  if(style=='ttw'): return getcolormap_ttw()
  if(style=='oviedo'): return getcolormap_oviedo()
  else: print('WARNING in getcolormap: style not recognized, returning None')
  return None

def define_color_hex( hexstring ):
  ### define a color based on a hexadecimal encoding.
  # note: see e.g. here for choosing colors:
  #       https://htmlcolorcodes.com/  
  # note: the color object needs to stay in scope, else the index will refer to a nullptr
  #       and the color will turn out white...
  #       this is achieved by setting the color as a global ROOT property.
  r, g, b = tuple(int(hexstring[i:i+2], 16) for i in (1, 3, 5))
  cindex = ROOT.TColor.GetFreeColorIndex()
  color = ROOT.TColor(cindex, r/255., g/255., b/255.)
  setattr(ROOT,'temp_color_'+str(cindex),color)
  return cindex,color

def getcolormap_default():
    # map of histogram titles to colors
    cmap = {}
    cmap['nonprompt'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt'] = cmap['nonprompt']
    cmap['DY'] = define_color_hex('#ffd22e')[0]
    cmap['TT'] = define_color_hex('#ffbd80')[0]
    cmap['TTX'] = define_color_hex('#4e09be')[0]
    cmap['TTZ'] = define_color_hex('#336fce')[0]
    cmap['TTW'] = define_color_hex('#2f8ceb')[0]
    cmap['TTTT'] = define_color_hex('#ff0000')[0]
    cmap['WZ'] = define_color_hex('#81efd7')[0]
    cmap['ZZ'] = define_color_hex('#2fbc6c')[0]
    cmap['ZZH'] = cmap['ZZ']
    cmap['ZG'] = define_color_hex('#9c88ff')[0]
    cmap['XG'] = cmap['ZG']
    cmap['triboson'] = define_color_hex('#c6ff00')[0]
    cmap['Triboson'] = cmap['triboson']
    cmap['other'] = define_color_hex('#ccccaa')[0]
    cmap['Other'] = cmap['other']
    return cmap

def getcolormap_tttt():
    # color map synchronizing with Niels's sample lists
    cmap = {}
    cmap['nonprompt'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt'] = cmap['nonprompt']
    cmap['TT+X'] = define_color_hex('#4e09be')[0]
    cmap['TTZ'] = define_color_hex('#336fce')[0]
    cmap['TTW'] = define_color_hex('#2f8ceb')[0]
    cmap['TTH'] = define_color_hex('#ccccaa')[0] 
    cmap['TTTT'] = define_color_hex('#ff0000')[0]
    cmap['WZ'] = define_color_hex('#81efd7')[0]
    cmap['ZZ-H'] = define_color_hex('#2fbc6c')[0]
    cmap['XG'] = define_color_hex('#9c88ff')[0]
    cmap['VVV'] = define_color_hex('#c6ff00')[0]
    return cmap

def getcolormap_systematics():
    # color map for plotting systematics
    cmap = {}
    # nominal in black
    cmap['nominal'] = ROOT.kBlack

    # acceptance uncertainties in shades of red
    cmap['JEC'] = ROOT.kRed
    cmap['JER'] = ROOT.kRed+2
    cmap['JER_2016'] = ROOT.kRed+2
    cmap['JER_2017'] = ROOT.kRed+2
    cmap['JER_2018'] = ROOT.kRed+2
    cmap['Uncl'] = ROOT.kRed-7
    cmap['Uncl_2016'] = ROOT.kRed-7
    cmap['Uncl_2017'] = ROOT.kRed-7
    cmap['Uncl_2018'] = ROOT.kRed-7
    cmap['JECSqSumAll'] = ROOT.kRed
    # (should correspond to JEC)
    cmap['JECSqSumGrouped'] = ROOT.kRed
    # (should correspond to JEC)
    cmap['JECAll'] = ROOT.kRed-10
    cmap['JECGrouped'] = ROOT.kRed-10

    # muon uncertainties in shades of blue
    cmap['muonID'] = ROOT.kBlue
    cmap['muonIDSyst'] = ROOT.kBlue
    cmap['muonIDStat'] = ROOT.kBlue+2
    cmap['muonIDStat_2016'] = ROOT.kBlue+2
    cmap['muonIDStat_2017'] = ROOT.kBlue+2
    cmap['muonIDStat_2018'] = ROOT.kBlue+2
    cmap['muonReco'] = ROOT.kBlue-7

    # electron uncertainties in shades of cyan
    cmap['electronID'] = ROOT.kCyan
    cmap['electronIDSyst'] = ROOT.kCyan
    cmap['electronIDStat'] = ROOT.kCyan+2
    cmap['electronIDStat_2016'] = ROOT.kCyan+2
    cmap['electronIDStat_2017'] = ROOT.kCyan+2
    cmap['electronIDStat_2018'] = ROOT.kCyan+2
    cmap['electronReco'] = ROOT.kCyan-9
    cmap['electronScale'] = ROOT.kCyan-5
    cmap['electronRes'] = ROOT.kCyan-5

    # other weights in shades of orange
    cmap['pileup'] = ROOT.kOrange
    cmap['bTag_heavy'] = ROOT.kOrange # not used anymore
    cmap['bTag_light'] = ROOT.kOrange # not used anymore
    cmap['prefire'] = ROOT.kOrange+9
    cmap['trigger'] = ROOT.kOrange+7

    # fakerate uncertainties in shades of cyan and purple
    cmap['efakeratenorm'] = ROOT.kCyan
    cmap['efakeratept'] = ROOT.kCyan+2
    cmap['efakerateeta'] = ROOT.kCyan+4
    cmap['mfakeratenorm'] = ROOT.kViolet
    cmap['mfakeratept'] = ROOT.kViolet+1
    cmap['mfakerateeta'] = ROOT.kViolet-1

    # btag weights in green
    cmap['bTag_shape_lf'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats1'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats2'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats1_2016'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats2_2016'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats1_2017'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats2_2017'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats1_2018'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats2_2018'] = ROOT.kGreen+1
    cmap['bTag_shape_hf'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats1'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats2'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats1_2016'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats2_2016'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats1_2017'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats2_2017'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats1_2018'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats2_2018'] = ROOT.kGreen+3
    cmap['bTag_shape_cferr1'] = ROOT.kSpring+6
    cmap['bTag_shape_cferr2'] = ROOT.kSpring+6

    # scales in shaded of purple
    cmap['fScale'] = ROOT.kMagenta
    cmap['fScaleTotal'] = ROOT.kMagenta
    cmap['fScaleShape'] = ROOT.kMagenta-1
    cmap['fScaleNorm'] = ROOT.kMagenta+1
    cmap['rScale'] = ROOT.kMagenta+3
    cmap['rScaleTotal'] = ROOT.kMagenta+3
    cmap['rScaleShape'] = ROOT.kMagenta+2
    cmap['rScaleNorm'] = ROOT.kMagenta+4
    cmap['rfScales'] = ROOT.kMagenta-7
    cmap['rfScalesTotal'] = ROOT.kMagenta-7
    cmap['rfScalesShape'] = ROOT.kMagenta-9
    cmap['rfScalesNorm'] = ROOT.kMagenta-8

    # isr/fsr in shades of violet
    cmap['isrTotal'] = ROOT.kViolet
    cmap['isrShape'] = ROOT.kViolet+1
    cmap['isrNorm'] = ROOT.kViolet-1
    cmap['fsrTotal'] = ROOT.kViolet+6
    cmap['fsrShape'] = ROOT.kViolet+5
    cmap['fsrNorm'] = ROOT.kViolet+7

    # qcd scale variations in magenta
    cmap['qcdScalesShapeVar'] = ROOT.kGray
    cmap['qcdScalesTotalEnv'] = ROOT.kMagenta-8
    cmap['qcdScalesShapeEnv'] = ROOT.kMagenta-9
    cmap['qcdScalesNorm'] = ROOT.kViolet-7

    # pdf variations in yellow
    cmap['pdfShapeVar'] = ROOT.kOrange-4
    cmap['pdfTotalRMS'] = ROOT.kOrange-5
    cmap['pdfShapeRMS'] = ROOT.kOrange-5
    cmap['pdfTotalEnv'] = ROOT.kYellow-7
    cmap['pdfShapeEnv'] = ROOT.kYellow-7
    cmap['pdfNorm'] = ROOT.kOrange-2

    # underlying event and color reconnection in blue
    cmap['CR_QCD'] = ROOT.kCyan+1
    cmap['CR_GluonMove'] = ROOT.kCyan+3
    cmap['UE'] = ROOT.kAzure+7

    # ad hoc added uncertainties in brown
    cmap['njets'] = ROOT.kRed+3
    cmap['nbjets'] = ROOT.kRed-1

    return cmap

def getcolormap_systematics_grouped():
    ### systematics but grouped per category
    cmap = {}
    cmap['nominal'] = ROOT.kBlack
    cmap['jetmet'] = ROOT.kRed
    cmap['muon'] = ROOT.kBlue
    cmap['electron'] = ROOT.kCyan
    cmap['pileup'] = ROOT.kOrange
    cmap['prefire'] = ROOT.kOrange+9
    cmap['trigger'] = ROOT.kOrange+7
    cmap['b-tagging'] = ROOT.kGreen+1
    cmap['scales'] = ROOT.kMagenta
    cmap['parton shower'] = ROOT.kViolet
    cmap['pdf'] = ROOT.kOrange-4
    cmap['lumi'] = ROOT.kMagenta+2
    cmap['njets'] = ROOT.kGreen+3
    cmap['norm'] = ROOT.kRed+2
    return cmap

def getcolormap_eft():
    cmap = {}
    cmap['nominal'] = ROOT.kBlack
    cmap['sm'] = cmap['nominal']
    cmap['cQei'] = ROOT.kRed
    cmap['cQl3i'] = ROOT.kRed
    cmap['cQlMi'] = ROOT.kRed
    cmap['cQq11'] = ROOT.kBlue
    cmap['cQq13'] = ROOT.kBlue
    cmap['cQq81'] = ROOT.kBlue
    cmap['cQq83'] = ROOT.kBlue
    cmap['cbW'] = ROOT.kCyan
    cmap['cpQ3'] = ROOT.kOrange
    cmap['cpQM'] = ROOT.kOrange
    cmap['cpt'] = ROOT.kOrange
    cmap['cptb'] = ROOT.kOrange
    cmap['ctG'] = ROOT.kOrange+9
    cmap['ctW'] = ROOT.kOrange+9
    cmap['ctZ'] = ROOT.kOrange+9
    cmap['ctei'] = ROOT.kOrange+7
    cmap['ctlSi'] = ROOT.kOrange+7
    cmap['ctlTi'] = ROOT.kOrange+7
    cmap['ctli'] = ROOT.kOrange+7
    cmap['ctp'] = ROOT.kGreen+1
    cmap['ctq'] = ROOT.kGreen+1
    cmap['ctq8'] = ROOT.kGreen+1
    return cmap

def getcolormap_ttw():
    # colormap for ttw analysis (in development)
    cmap = {}
    cmap['nonprompt'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt'] = cmap['nonprompt']
    cmap['Nonprompt(e)'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt (e)'] = cmap['Nonprompt(e)']
    cmap['NonpromptE'] = cmap['Nonprompt(e)']
    cmap['Nonprompt(m)'] = define_color_hex('#fcda60')[0]
    cmap['Nonprompt (mu)'] = cmap['Nonprompt(m)']
    cmap['NonpromptE'] = cmap['Nonprompt(e)']
    cmap['NonpromptMu'] = cmap['Nonprompt(m)']
    cmap['chargeflips'] = define_color_hex('#fcee1e')[0]
    cmap['Chargeflips'] = cmap['chargeflips']
    cmap['DY'] = define_color_hex('#ffd22e')[0]
    cmap['ZG'] = define_color_hex('#900ead')[0]
    cmap['TT'] = define_color_hex('#ffbd80')[0]
    cmap['TTG'] = define_color_hex('#b443fa')[0]
    cmap['Conversions'] = define_color_hex('#b443fa')[0]
    cmap['TTX'] = define_color_hex('#2f8ceb')[0]
    cmap['TTH'] = define_color_hex('#531bfa')[0]
    cmap['TTZ'] = define_color_hex('#336fce')[0]
    cmap['TTW'] = define_color_hex('#ff0000')[0]
    cmap['TTW0'] = define_color_hex('#990000')[0]
    cmap['TTW1'] = define_color_hex('#cc0000')[0]
    cmap['TTW2'] = define_color_hex('#ff0000')[0]
    cmap['TTW3'] = define_color_hex('#ff4400')[0]
    cmap['TTW4'] = define_color_hex('#ff6f00')[0]
    cmap['TTWplus'] = define_color_hex('#ff0000')[0]
    cmap['TTW+'] = cmap['TTWplus']
    cmap['TTWminus'] = define_color_hex('#ff4400')[0]
    cmap['TTW-'] = cmap['TTWminus']
    cmap['TX'] = define_color_hex('#49c7f5')[0]
    cmap['WZ'] = define_color_hex('#81efd7')[0]
    cmap['ZZ'] = define_color_hex('#2fbc6c')[0]
    cmap['multiboson'] = define_color_hex('#54f035')[0]
    cmap['Multiboson'] = cmap['multiboson']
    cmap['other'] = define_color_hex('#ccccaa')[0]
    cmap['Other'] = cmap['other']
    return cmap

def getcolormap_oviedo():
    # colormap for ttw analysis (sync background categories with Oviedo)
    cmap = {}
    cmap['Nonprompt'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt (e)'] = define_color_hex('#ffe380')[0]
    cmap['Nonprompt (mu)'] = define_color_hex('#fcda60')[0]
    cmap['Chargeflips'] = define_color_hex('#fcee1e')[0]
    cmap['Conversions'] = define_color_hex('#b443fa')[0]
    cmap['TTVV'] = define_color_hex('#2f8ceb')[0]
    cmap['TTH'] = define_color_hex('#531bfa')[0]
    cmap['TTZ'] = define_color_hex('#336fce')[0]
    cmap['TTW'] = define_color_hex('#ff0000')[0]
    cmap['TTW0'] = define_color_hex('#990000')[0]
    cmap['TTW1'] = define_color_hex('#cc0000')[0]
    cmap['TTW2'] = define_color_hex('#ff0000')[0]
    cmap['TTW3'] = define_color_hex('#ff4400')[0]
    cmap['TTW4'] = define_color_hex('#ff6f00')[0]
    cmap['TTWplus'] = define_color_hex('#ff0000')[0]
    cmap['TTW+'] = cmap['TTWplus']
    cmap['TTWminus'] = define_color_hex('#ff4400')[0]
    cmap['TTW-'] = cmap['TTWminus']
    cmap['THQ'] = define_color_hex('#ffd22e')[0]
    cmap['THW'] = define_color_hex('#900ead')[0]
    cmap['TZQ'] = define_color_hex('#ffbd80')[0]
    cmap['WZ'] = define_color_hex('#81efd7')[0]
    cmap['ZZ'] = define_color_hex('#2fbc6c')[0]
    cmap['VVV'] = define_color_hex('#54f035')[0]
    cmap['Rares'] = define_color_hex('#ccccaa')[0]
    return cmap


if __name__=='__main__':
  # make a dummy plot of a colormap for quick testing

  # parse arguments
  parser = argparse.ArgumentParser(description='Plot color map')
  parser.add_argument('--colormap', required=True)
  parser.add_argument('--signal', default=None)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # get colormap
  cmap = getcolormap( style=args.colormap )

  # define signals
  signals = [args.signal] if args.signal is not None else None
  
  # make a dummy histogram for each unique label
  keys = sorted(cmap.keys())
  histlist = []
  unique_keys = []
  for key in keys:
    # skip labels that only differ in case
    upkey = key.upper()
    if upkey in unique_keys: continue
    unique_keys.append(upkey)
    # make a histogram
    hist = ROOT.TH1D( key, key, 1, 0., 1.)
    hist.SetDirectory(0)
    hist.Fill(0.5)
    histlist.append(hist)

  # make fake data histogram
  datahist = histlist[0].Clone()
  for hist in histlist[1:]:
    datahist.Add(hist)

  # set plot properties
  xaxtitle = 'Yield'
  yaxtitle = 'Number of events'
  outfile = 'testcolors'
  extrainfos = []
  extrainfos.append( 'Fake yields' )
  extrainfos.append( '(for testing colors)' )

  # make the plot
  hp.plotdatavsmc(outfile, datahist, histlist,
            signals=signals,
            xaxtitle=xaxtitle,
            yaxtitle=yaxtitle,
            colormap=cmap,
            extrainfos=extrainfos, infosize=15 )
