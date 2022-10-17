###################################################################
# merge and plot the output of the trigger efficiency measurement #
###################################################################
# note: proper merging depends on file naming and structuring conventions.

import os
import sys
import ROOT
import argparse
sys.path.append(os.path.abspath('../../constants'))
from luminosities import lumidict
sys.path.append(os.path.abspath('../../Tools/python'))
import histtools as ht
sys.path.append(os.path.abspath('../../plotting/python'))
from efficiencyplotter import plotefficiencies

def getcolor(hist, cmode):
  tag = hist.GetName().split('_')[1]
  if cmode=='singleyear':
    if 'A' in tag: return ROOT.kRed-7
    elif 'B' in tag: return ROOT.kOrange-3
    elif 'C' in tag: return ROOT.kOrange
    elif 'D' in tag: return ROOT.kGreen+1
    elif 'E' in tag: return ROOT.kCyan+1
    elif 'F' in tag: return ROOT.kAzure
    elif 'G' in tag: return ROOT.kViolet
    elif 'H' in tag: return ROOT.kMagenta+3
    elif 'Run201' in tag: return ROOT.kRed
  if mode=='allyears':
    if '2016' in tag: return ROOT.kOrange
    elif '2017' in tag: return ROOT.kGreen+1
    elif '2018' in tag: return ROOT.kAzure
    elif 'all' in tag: return ROOT.kRed
  return ROOT.kBlack

def formathistname(title):
  # get suitable legend entry out of histogram name
  # simulation is the easy part since only one graph is expected
  if('mc_' in title): return 'Simulation'
  # data: everything combined
  if('allyears' in title): return 'Data (all years)'  
  # data: single era or full year
  era = title.split('_')[1]
  return 'Data '+era.replace('Run','')

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot results of trigger efficiency measurement')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('--skipmerge', action='store_true')
  parser.add_argument('--skipplot', action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))

  # first find directories to run on
  directories = []
  for root,dirs,files in os.walk(args.inputdir):
    for dirname in dirs:
      contents = os.listdir(os.path.join(root,dirname))
      if( 'sim' in contents and 'data' in contents ):
        directories.append(os.path.join(root,dirname))

  # loop over all directories
  for directory in directories:
    # define the merged file containing all histograms to plot
    mergedfilename = 'combined_trigger_histograms.root'
    mergedfilepath = os.path.join(directory,mergedfilename)
    figdir = os.path.join(directory,'plots')
    if not os.path.exists(figdir): os.makedirs(figdir)

    # get year from path name
    year = 0
    if '2016PreVFP' in directory: year = '2016PreVFP'
    elif '2016PostVFP' in directory: year = '2016PostVFP'
    elif '2017' in directory: year = '2017'
    elif '2018' in directory: year = '2018'

    # do merging if requested
    if not args.skipmerge:
      if os.path.exists(mergedfilepath): os.system('rm '+mergedfilepath)
      datadir = os.path.join(directory,'data')
      simdir = os.path.join(directory,'sim')
      datafiles = [os.path.join(datadir,f) for f in os.listdir(datadir) if f[-5:]=='.root']
      simfiles = [os.path.join(simdir,f) for f in os.listdir(simdir) if f[-5:]=='.root']
      command = 'hadd '+mergedfilepath
      for f in datafiles+simfiles:
        command += ' '+os.path.join(f)
      os.system(command)

    # plotting
    if not args.skipplot:
      variables = [
        {'name':'leptonptleading','xaxtitle':r'lepton p_{T}^{leading} (GeV)'},
        {'name':'leptonptsubleading','xaxtitle':r'lepton p_{T}^{subleading} (GeV)'},
        {'name':'leptonpttrailing','xaxtitle':r'lepton p_{T}^{trailing} (GeV)'},
        {'name':'yield','xaxtitle':r'total efficiency'}
      ]
      objlist = ht.loadallhistograms(mergedfilepath, allow_tgraphs=True)
      for variable in variables:
        simgraphlist = ht.selecthistograms(objlist,mustcontainall=['mc',variable['name']+'_eff'])[1]
        datagraphlist = ht.selecthistograms(objlist,mustcontainall=['Run201',variable['name']+'_eff'])[1]
        simhistlist = ht.selecthistograms(objlist,mustcontainall=['mc',variable['name']+'_tot'])[1]
        datahistlist = ht.selecthistograms(objlist,mustcontainall=['Run201',variable['name']+'_tot'])[1]
	# make sure the order in datagraphlist and datahistlist corresponds
        datagraphlist.sort(key = lambda x: x.GetName())
        datahistlist.sort(key = lambda x: x.GetName())
        # check number of simulation contributions
        if len(simgraphlist)!=1:
          raise Exception('ERROR: wrong number of simulated efficiency graphs.')
        if len(simhistlist)!=1:
          raise Exception('ERROR: wrong number of simulated efficiency hists.')
	# set plot properties
        yaxtitle = 'Trigger efficiency'
        xaxtitle = variable['xaxtitle']
        lumi = lumidict[year]
        datacolorlist = [getcolor(graph,'singleyear') for graph in datagraphlist]
        datalabellist = [formathistname(graph.GetName()) for graph in datagraphlist]
        figname = os.path.join(directory,'plots',variable['name']+'.png')
        plotefficiencies( datagraphlist, figname, 
			  datahistlist=datahistlist, 
			  datacolorlist=datacolorlist,
                          datalabellist=datalabellist,
			  simgraph=simgraphlist[0], simhist=simhistlist[0],
			  simcolor=ROOT.kBlack, simlabel='Simulation',
			  simsysthist=None, 
			  yaxtitle=yaxtitle, xaxtitle=xaxtitle,
			  lumi=lumi, extracmstext='Preliminary',
                          dodataavg=True, dosimavg=True )
