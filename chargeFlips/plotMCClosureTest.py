#########################################
# plot the results of a MC closure test #
#########################################

import os
import sys
import ROOT
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
import histplotter as hp

# global properties
systunc = 0.3 
# (relative systematic uncertainty to add)

# read input files from the command line or else read all input files in the current directory
inputfiles = []
if len(sys.argv)>1:
  inputfiles = sys.argv[1:]
else: 
  inputfiles = ([f for f in os.listdir(os.getcwd()) 
         if ('closurePlots_MC' in f and f[-5:]=='.root') ])
inputfiles = [os.path.abspath(f) for f in inputfiles]

# loop over input files
for f in inputfiles:
  print('now running on {}...'.format(f))
  fdir,fbase = os.path.split(f)

  # set output directory
  outdir = f.replace('.root','')
  if not os.path.exists(outdir): os.makedirs(outdir)

  # determine instance properties from filename
  # (the filename is assumed to be of the following form: 
  # closurePlots_MC_<process>_<year>.root)
  instancename = fbase.replace('closurePlots_MC_','').replace('.root','')
  instanceparts = instancename.split('_')
  process = instanceparts[0]
  year = instanceparts[1]
  print('this file is found to have the following properties:')
  print('  - process: {}'.format(process))
  print('  - year: {}'.format(year))

  # determine luminosity value to display
  lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
             '2016PreVFP':19520, '2016PostVFP':16810 }
  lumi = lumimap[year]

  # load all histograms
  histlist = ht.loadallhistograms(f)

  # get a list of variables
  observedhists = ht.selecthistograms(histlist, mustcontainall=['observed'])[1]
  names = [h.GetName() for h in observedhists]
  variables = []
  for name in names:
    var = name.split(instancename)[0].strip('_')
    if var not in variables: variables.append(var)
  print('found following variables: '+str(variables))

  # loop over variables
  for var in variables:
    print('running on variable {}'.format(var))

    # get the histograms
    observedhists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'observed'])[1]
    predictedhists = ht.selecthistograms(histlist,
      mustcontainall=[var+'_',instancename,'predicted'],
      maynotcontainone=['heavy','light','other'])[1]
    print('found {} histograms for observed'.format(len(observedhists)))
    print('found {} histograms for predicted'.format(len(predictedhists)))
    observedhist = observedhists[0].Clone()
    for h in observedhists[1:]: observedhist.Add(h)
    predictedsum = predictedhists[0].Clone()
    for h in predictedhists[1:]: predictedsum.Add(h)

    # add the systematic uncertainty
    predictedsyst = predictedsum.Clone()
    for i in range(0,predictedsum.GetNbinsX()+2):
      predictedsyst.SetBinContent(i,predictedsum.GetBinContent(i)*systunc)

    # axis titles: get from histograms
    xaxtitle = observedhist.GetXaxis().GetTitle()
    yaxtitle = observedhist.GetYaxis().GetTitle()

    # set histogram titles
    for h in predictedhists:
      h.SetTitle(process)
    observedhist.SetTitle("MC Observed")

    # set other plot options
    colormap = {'TT': ROOT.kMagenta-7,'DY':ROOT.kMagenta+1}
    labelmap = ({'TT':'Charge misid. rate prediction',
                 'DY':'Charge misid. rate prediction'})
    legendbox = [0.5,0.5,0.92,0.9]
    extracmstext = 'Preliminary'
    extrainfos = []
    if process=='DY': extrainfos=['Drell-Yan simulation']
    if process=='TT': extrainfos=['t#bar{t} simulation']
  
    hp.plotdatavsmc( os.path.join(outdir,var+'_'+instancename), observedhist, 
      predictedhists, mcsysthist=predictedsyst,
      datalabel='MC Obs.', p2yaxtitle='#frac{MC Obs.}{Pred.}',
      colormap=colormap, labelmap=labelmap,
      xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
      p1legendncols=1,p1legendbox=legendbox,
      extracmstext=extracmstext,
      extrainfos=extrainfos)
    hp.plotdatavsmc( os.path.join(outdir,var+'_'+instancename+'_log'), observedhist, 
      predictedhists, mcsysthist=predictedsyst, 
      datalabel='MC Obs.', p2yaxtitle='#frac{MC Obs.}{Pred.}',
      colormap=colormap, labelmap=labelmap,
      xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
      p1legendncols=1,p1legendbox=legendbox,
      extracmstext=extracmstext,
      extrainfos=extrainfos,
      yaxlog=True)
