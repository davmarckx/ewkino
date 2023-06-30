#########################################################
# plot the output of the trigger efficiency measurement #
#########################################################

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
    if tag in ['Run2016PreVFP','Run2016PostVFP','Run2017','Run2018']:
      return ROOT.kRed
    elif 'A' in tag: return ROOT.kRed-7
    elif 'B' in tag: return ROOT.kOrange-3
    elif 'C' in tag: return ROOT.kOrange
    elif 'D' in tag: return ROOT.kGreen+1
    elif 'E' in tag: return ROOT.kCyan+1
    elif 'F' in tag: return ROOT.kAzure
    elif 'G' in tag: return ROOT.kViolet
    elif 'H' in tag: return ROOT.kMagenta+3
  if cmode=='allyears':
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
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))

  # first find files to run on
  mergedfilename = 'combined_trigger_histograms.root'
  mergedfiles = []
  for root,dirs,files in os.walk(args.inputdir):
    for f in files:
      if( f==mergedfilename ):
        mergedfiles.append(os.path.join(root,f))
  print('Found {} files.'.format(len(mergedfiles)))

  # loop over all files
  for mergedfilepath in mergedfiles:
    print('Running on file {}...'.format(mergedfilepath))
    directory = os.path.dirname(mergedfilepath)
    figdir = os.path.join(directory,'plots')
    if not os.path.exists(figdir): os.makedirs(figdir)
    sfdir = os.path.join(directory,'scalefactors')
    if not os.path.exists(sfdir): os.makedirs(sfdir)

    # get year from path name
    year = 0
    if '2016PreVFP' in directory: year = '2016PreVFP'
    elif '2016PostVFP' in directory: year = '2016PostVFP'
    elif '2017' in directory: year = '2017'
    elif '2018' in directory: year = '2018'

    # set variables
    variables = [
        {'name':'leptonptleading','xaxtitle':r'Leading lepton p_{T} (GeV)'},
        {'name':'leptonptsubleading','xaxtitle':r'Subleading lepton p_{T} (GeV)'},
        {'name':'leptonpttrailing','xaxtitle':r'Trailing lepton p_{T} (GeV)'},
        {'name':'yield','xaxtitle':r'Total yield'}
    ]

    # load all objects
    objlist = ht.loadallhistograms(mergedfilepath, allow_tgraphs=True)
    sfhistlist = []

    # loop over variables
    for variable in variables:

      # get all relevant objects
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
      simgraph = simgraphlist[0]
      simhist = simhistlist[0]
      # set plot properties
      yaxtitle = 'Trigger efficiency'
      xaxtitle = variable['xaxtitle']
      lumi = lumidict[year]
      datacolorlist = [getcolor(graph,'singleyear') for graph in datagraphlist]
      datalabellist = [formathistname(graph.GetName()) for graph in datagraphlist]
      figname = os.path.join(figdir,variable['name']+'.png')
      plotefficiencies( datagraphlist, figname, 
			  datahistlist=datahistlist, 
			  datacolorlist=datacolorlist,
                          datalabellist=datalabellist,
			  simgraph=simgraph, simhist=simhist,
			  simcolor=ROOT.kBlack, simlabel='Simulation',
			  simsysthist=None, 
			  yaxtitle=yaxtitle, xaxtitle=xaxtitle,
			  lumi=lumi, extracmstext='Preliminary',
                          dodataavg=True, dosimavg=True )

      # calculate scale factors
      for datagraph in datagraphlist:
        era = datagraph.GetName().split('_')[1]
        sfhist = datahistlist[0].Clone()
        sfhist.Reset()
        sfuphist = sfhist.Clone()
        sfdownhist = sfhist.Clone()
        sfhist.SetName('scalefactors_nominal_{}_{}'.format(era,variable['name']))
        sfuphist.SetName('scalefactors_errorup_{}_{}'.format(era,variable['name']))
        sfdownhist.SetName('scalefactors_errordown_{}_{}'.format(era,variable['name']))
        for i in range(1, sfhist.GetNbinsX()+1):
          dataeff = datagraph.GetY()[i-1]
          simeff = simgraph.GetY()[i-1]
          dataerrup = datagraph.GetErrorYhigh(i-1)
          dataerrdown = datagraph.GetErrorYlow(i-1)
          simerrup = simgraph.GetErrorYhigh(i-1)
          simerrdown = simgraph.GetErrorYlow(i-1)
          if( simeff > 0 and dataeff > 0 ):
            sf = dataeff / simeff
            sferrup = sf*( (dataerrup/dataeff)**2 + (simerrup/simeff)**2 )**0.5
            sferrdown = sf*( (dataerrdown/dataeff)**2 + (simerrdown/simeff)**2 )**0.5
	    sfhist.SetBinContent(i,sf)
            sfhist.SetBinError(i,max(sferrup,sferrdown))
            sfuphist.SetBinContent(i,sferrup)
            sfdownhist.SetBinContent(i,sferrdown)
        sfhistlist.append(sfhist)
        sfhistlist.append(sfuphist)
        sfhistlist.append(sfdownhist)

    # write scale factor histograms to file
    outfile = os.path.join(sfdir, 'scalefactors_{}.root'.format(year))
    f = ROOT.TFile.Open( outfile, 'recreate' )
    for hist in sfhistlist: hist.Write()
    f.Close() 
