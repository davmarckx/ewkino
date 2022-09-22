##################################################
# temporary script to parse variable definitions #
##################################################
# in principle only meant to be run once...

# input: 
# - variable binning definition in old hard-coded (C++) convention,
#   e.g. here: https://github.com/LukaLambrecht/ewkino/blob/43a93413e42bb80406028da905017308264f4224/tZqAnalysisCode/systematics/runsystematics.cc#L1333
# - variable axis title definition in old hard-coded (Python) convention,
#   e.g. here: https://github.com/LukaLambrecht/ewkino/blob/43a93413e42bb80406028da905017308264f4224/tZqAnalysisCode/plotting/histplotter_postfit.py#L286

# output:
# - a json file with all variables in new convention

import sys
import os
import json
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import HistogramVariable, read_variables, write_variables_json


def parse_variable_cppline( line ):
  # operations on the line
  line = line.strip(' \t\n')
  line = line.replace('/*','')
  line = line.replace('*/','')
  line = line.replace('//',' ')
  line = line.replace('vars.push_back(std::make_tuple(','')
  line = line.replace('));','')
  lineparts = line.split(',')
  # operations on the parts
  lineparts[0] = lineparts[0].strip('"')
  lineparts[3] = lineparts[3].replace('.','')
  # attribute assignments
  name = lineparts[0]
  xlow = float(lineparts[1])
  xhigh = float(lineparts[2])
  nbins = int(lineparts[3])
  var = HistogramVariable( name, name, nbins, xlow, xhigh )
  return var

def parse_variable_pyline( line ):
  line = line.strip(' \t\n,')
  line = line.replace('#','')
  line = line.replace('\'','"')
  line = r'{}'.format(line)
  line = line.replace('r"','"')
  line = line.replace('\\','\\\\')
  linedict = json.loads(line)
  linedict['name'] = str(linedict['name'])
  linedict['title'] = r'{}'.format(str(linedict['title']))
  linedict['unit'] = str(linedict['unit'])
  return linedict
 
  
if __name__=='__main__':

  cmdline = False

  if cmdline:
    print('Enter/paste text with variable binning definitions (C++).')
    print('Use Ctrl-D to save it.')
    cppcontents = []
    while True:
      try: line = raw_input()
      except EOFError: break
      cppcontents.append(line)

    print('Enter/paste text with variable axis definitions (Python).')
    print('Use Ctrl-D to save it.')
    pycontents = []
    while True:
      try: line = raw_input()
      except EOFError: break
      pycontents.append(line)

  else:
    cpptext = '''/*vars.push_back(std::make_tuple("_abs_eta_recoil",0.,5.,20));
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_deepFlavor_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,200.,20));
    vars.push_back(std::make_tuple("_coarseBinnedMT",0.,200.,4.));
    vars.push_back(std::make_tuple("_smallRangeMT", 0., 150., 15));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dRlb_min",0.,3.15,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,6.5,7));
    vars.push_back(std::make_tuple("_nBJets",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_dRlWrecoil",0.,10.,20));
    vars.push_back(std::make_tuple("_dRlWbtagged",0.,7.,20));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));*/
    vars.push_back(std::make_tuple("_altBinnedM3l", 100., 400., 20)); 
    /*vars.push_back(std::make_tuple("_fineBinnedM3l",75.,105,20));
    vars.push_back(std::make_tuple("_abs_eta_max",0.,5.,20));
    vars.push_back(std::make_tuple("_eventBDT",-1,1,15));
    vars.push_back(std::make_tuple("_fineBinnedeventBDT",-1,1,1000));
    vars.push_back(std::make_tuple("_nMuons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_nElectrons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));
    vars.push_back(std::make_tuple("_leptonPtLeading",0.,300.,12));
    vars.push_back(std::make_tuple("_leptonPtSubLeading",0.,180.,12));
    vars.push_back(std::make_tuple("_leptonPtTrailing",0.,120.,12));
    vars.push_back(std::make_tuple("_fineBinnedleptonPtTrailing",0.,50.,20));
    vars.push_back(std::make_tuple("_leptonEtaLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaSubLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaTrailing",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_jetPtLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_jetPtSubLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_numberOfVertices",-0.5,70.5,71));
    vars.push_back(std::make_tuple("_bestZMass",0.,150.,15));
    vars.push_back(std::make_tuple("_lW_pt",10.,150.,14));
    vars.push_back(std::make_tuple("_coarseBinnedlW_pt",0.,105.,4));
    vars.push_back(std::make_tuple("_Z_pt",0.,300.,15));
    vars.push_back(std::make_tuple("_coarseBinnedZ_pt",0.,275.,4));*/'''
    cppcontents = cpptext.split('\n')

    pytext = r'''{'name':'_abs_eta_recoil','title':r'\left|\eta\right|_{recoil}','unit':''},
        {'name':'_Mjj_max','title':r'm_{jet+jet}^{max}','unit':'GeV'},
        {'name':'_lW_asymmetry','title':r'Asymmetry (lepton from W)','unit':''},
        {'name':'_deepCSV_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_deepFlavor_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_lT','title':'L_{T}','unit':'GeV'},
    {'name':'_MT','title':'Transverse W boson mass','unit':'GeV'},
    {'name':'_smallRangeMT','title':'Transverse W boson mass','unit':'GeV'},
    {'name':'_coarseBinnedMT','title':'Transverse W boson mass','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'\Delta R(l,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'\Delta \Phi (l,l)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'Number of jets','unit':''},
        {'name':'_nBJets','title':r'Number of b-tagged jets','unit':''},
        {'name':'_dRlWrecoil','title':r'\Delta R(l_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'\Delta R(l_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'm_{3l}','unit':'GeV'},
        {'name':'_fineBinnedM3l','title':r'm_{3l}','unit':'GeV'},
        {'name':'_abs_eta_max','title':r'\left|\eta\right|_{max}','unit':''},
        {'name':'_eventBDT','title':r'Event BDT discriminant','unit':''},
        {'name':'_nMuons','title':r'Number of muons in event','unit':''},
        {'name':'_nElectrons','title':r'Number of electrons in event','unit':''},
        {'name':'_yield','title':r'Total yield','unit':''},
        #{'name':'_leptonMVATOP_min','title':r'Minimum TOP MVA value in event','unit':''},
        #{'name':'_leptonMVAttH_min','title':r'Minimum ttH MVA value in event','unit':''},
        #{'name':'_rebinnedeventBDT','title':'Event BDT output score','unit':''},
        {'name':'_leptonPtLeading','title':r'Leading lepton p_{T}','unit':'GeV'},
        {'name':'_leptonPtSubLeading','title':r'Subleading lepton p_{T}','unit':'GeV'},
        {'name':'_leptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
        {'name':'_fineBinnedleptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
        {'name':'_leptonEtaLeading','title':r'Leading lepton \eta','unit':''},
        {'name':'_leptonEtaSubLeading','title':r'Subleading lepton \eta','unit':''},
        {'name':'_leptonEtaTrailing','title':r'Trailing lepton \eta','unit':''},
        {'name':'_jetPtLeading','title':r'Leading jet p_{T}','unit':'GeV'},
        {'name':'_jetPtSubLeading','title':r'Subleading jet p_{T}','unit':'GeV'},
        {'name':'_bestZMass','title':r'Mass of OSSF pair','unit':'GeV'},
    {'name':'_Z_pt','title':'Z boson p_{T}','unit':'GeV'},
        {'name':'_coarseBinnedZ_pt','title':'Z boson p_{T}','unit':'GeV'},
        {'name':'_lW_pt','title':'Lepton from W boson p_{T}','unit':'GeV'},
        {'name':'_coarseBinnedlW_pt','title':'Lepton from W boson p_{T}','unit':'GeV'}'''
    pycontents = pytext.split('\n')

  # parse cpp lines and make a variable collection
  variables = []
  for line in cppcontents:
    var = parse_variable_cppline( line )
    variables.append(var)

  # parse python lines
  vardicts = []
  for line in pycontents:
    vardict = parse_variable_pyline( line )
    vardicts.append(vardict)

  # match both collections
  for var in variables:
    for vardict in vardicts:
      if var.name == vardict['name']:
        var.axtitle = vardict['title']
        var.unit = vardict['unit']

  # write to json
  fname = 'variables_autoparse.json'
  write_variables_json( variables, fname )

  # test if readable
  try:
    vartest = read_variables( fname )
    for var in vartest: print(var)
  except: 
    msg = 'WARNING: the produced json file could not be read properly!'
    msg += ' Here is the stack trace:'
    print(msg)
    read_variables( fname )
