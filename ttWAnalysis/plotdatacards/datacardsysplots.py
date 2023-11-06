#######################################
# Make plots of systematic variations #
#######################################
# This script is supposed to be used on an output file of runanalysis.py,
# i.e. a root file containing histograms with the following naming convention:
# <process tag>_<selection region>_<variable name>_<systematic>
# where the systematic is either "nominal" or a systematic name followed by "Up" or "Down".

import sys
import os
import json
import argparse
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
import listtools as lt
from variabletools import read_variables
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection
sys.path.append(os.path.abspath('../plotting'))
import colors
import infodicts
sys.path.append(os.path.abspath('../plotsystematics'))
from systtools import category
from systplotter import plotsystematics


if __name__=="__main__":
    
  # parse arguments
  parser = argparse.ArgumentParser(description='Make datacard systematic plots')
  parser.add_argument('--datacard', required=True, type=os.path.abspath)
  parser.add_argument('--histfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  parser.add_argument('--group', default=False, action='store_true',
                      help='Group systematics in categories before plotting.')
  parser.add_argument('--includetotal', default=False, action='store_true',
                      help='Include total systematic uncertainty.')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # check input files
  if not os.path.exists(args.datacard):
    raise Exception('ERROR: datacard {} does not exist.'.format(args.datacard))
  if not os.path.exists(args.histfile):
    raise Exception('ERROR: histogram file {} does not exist.'.format(args.histfile))

  # parse tags
  extratags = []
  if args.tags is not None: extratags = args.tags.split(',')
  extratags = [t.replace('_',' ') for t in extratags]

  # build a ProcessInfoCollection from the datacard
  PIC = ProcessInfoCollection.fromdatacard(args.datacard, adddata=True)
  #print('Extracted the following ProcessInfoCollection from the datacard:')
  #print(PIC)
  print('Processes:')
  for p in PIC.plist: print('  - {}'.format(p))
  print('Systematics:')
  for s in PIC.slist: print('  - {}'.format(s))

  # make a ProcessCollection
  PC = ProcessCollection( PIC, args.histfile )

  # group systematics
  if args.group:
    categories = [category(systematic) for systematic in PIC.slist]
    catdict = {}
    for cat in set(categories):
        catdict[cat] = [PIC.slist[i] for i in range(len(PIC.slist)) if categories[i]==cat]
    print('Will group systematics as follows:')
    for cat in catdict.keys():
        print('  {}'.format(cat))
        for sys in catdict[cat]: print('    {}'.format(sys))

  # get the nominal histogram
  nominalhist = PC.get_nominal()
  nominalhist.SetTitle('nominal')

  # get the systematics histograms
  syshistlist = []
  # make a list of all systematics
  for systematic in sorted(PIC.slist):
      uphist = PC.get_systematic_up(systematic)
      uphist.SetTitle(systematic+'Up')
      if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
      downhist = PC.get_systematic_down(systematic)
      downhist.SetTitle(systematic+'Down')
      if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
      syshistlist.append(uphist)
      syshistlist.append(downhist)
  # re-group systematics if requested
  if args.group:
      newsyshistlist = []
      for cat in catdict.keys():
        rss = PC.get_systematics_rss(systematics=catdict[cat], correlate_processes=True)
        uphist = nominalhist.Clone()
        uphist.Add(rss)
        uphist.SetTitle(cat+'Up')
        if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
        downhist = nominalhist.Clone()
        downhist.Add(rss, -1)
        downhist.SetTitle(cat+'Down')
        if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
        newsyshistlist.append(uphist)
        newsyshistlist.append(downhist)
      syshistlist = newsyshistlist
  # add total if requested
  if args.includetotal:
        rss = PC.get_systematics_rss(correlate_processes=True)
        uphist = nominalhist.Clone()
        uphist.Add(rss)
        uphist.SetTitle('totalUp')
        if uphist.GetName().endswith('Down'):
          uphist.SetName(uphist.GetName()[:-4]+'Up')
        downhist = nominalhist.Clone()
        downhist.Add(rss, -1)
        downhist.SetTitle('totalDown')
        if downhist.GetName().endswith('Up'):
          downhist.SetName(downhist.GetName()[:-2]+'Down')
        syshistlist.append(uphist)
        syshistlist.append(downhist)

  # re-order histograms to put individual pdf, qcd and jec variations in front
  # (so they will be plotted in the background)
  firsthistlist = []
  secondhistlist = []
  for hist in syshistlist:
      if( 'ShapeVar' in hist.GetName() 
	  or 'JECAll' in hist.GetName() 
          or 'JECGrouped' in hist.GetName() ):
        firsthistlist.append(hist)
      else: secondhistlist.append(hist)
  syshistlist = firsthistlist + secondhistlist

  # format the labels
  # (remove year and process tags for more readable legends)
  for hist in syshistlist:
      label = str(hist.GetTitle())
      baselabel = label
      if label.endswith('Up'): baselabel = label[:-2]
      if label.endswith('Down'): baselabel = label[:-4]
      for p in PIC.plist:
        p = str(p)
        if baselabel.endswith(p):
          label = label.replace(p,'')
      for y in ['2016PreVFP','2016PostVFP','2017','2018']:
        if baselabel.endswith(y): label = label.replace(y,'')
      hist.SetTitle(label)

  # make extra infos to display on plot
  extrainfos = []
  # processes
  #pinfohead = 'Processes:'
  #if doallprocesses:
  #    pinfohead += ' all'
  #    extrainfos.append(pinfohead)
  #else:
  #    pinfostr = ','.join([str(p) for p in PIC.plist])
  #    extrainfos.append(pinfohead)
  #    extrainfos.append(pinfostr)
  # year
  #yeartag = args.year.replace('run2', 'Run 2')
  #extrainfos.append(yeartag)
  # region
  #regiontag = get_region_dict().get(args.region, args.region)
  #extrainfos.append(regiontag)
  # others
  for tag in extratags: extrainfos.append(tag)

  # choose color map
  colormap = None
  if args.group: colormap = colors.getcolormap('systematics_grouped')

  # set plot properties
  yaxtitle = 'Events'
  relyaxtitle = 'Normalized'
  xaxtitle = 'Fit variable'
  figname = os.path.splitext(args.outputfile)[0]
  # make absolute plot
  plotsystematics(nominalhist, syshistlist, figname+'_abs', 
                    colormap=colormap,
                    yaxtitle=yaxtitle, xaxtitle=xaxtitle,
                    style='absolute', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
  # make normalized plot
  plotsystematics(nominalhist, syshistlist, figname+'_nrm',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='normalized', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
  # make relative plot
  plotsystematics(nominalhist, syshistlist, figname+'_rel',
                    colormap=colormap,
                    yaxtitle=relyaxtitle, xaxtitle=xaxtitle,
                    style='relative', staterrors=True,
                    extrainfos=extrainfos, infoleft=0.19, infotop=0.85, infosize=15,
                    remove_duplicate_labels=True,
                    remove_down_labels=True)
