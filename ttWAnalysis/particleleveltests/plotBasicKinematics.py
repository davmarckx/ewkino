################################################
# plot histograms obtained with fillComparison #
################################################

import sys
import os
import argparse
sys.path.append('../../Tools/python')
import histtools as ht
sys.path.append('../../plotting/python')
import singlehistplotter as shp
import multihistplotter as mhp
sys.path.append(os.path.abspath('../eventselection'))
from eventselector import event_selections


if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot basic particle level kinematics')
  parser.add_argument('--inputfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputdir', required=True)
  parser.add_argument('--event_selection', required=True, choices=event_selections, nargs='+')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # hard-coded arguments
  # list of variables (check with fillBasicKinematics.cc)
  variables = ({
    'electronPt': 'Electron p_{T} (GeV)',
    'electronEta': 'Electron #eta',
    'muonPt': 'Muon p_{T}',
    'muonEta': 'Muon #eta',
    'jetPt': 'Jet p_{T}',
    'jetEta': 'Jet #eta',
    'jetLeptonMinDR': '#Delta R(j,l)_{min}',
    'nLeptons': 'Number of leptons',
    'nJets': 'Number of jets'
  })

  # argument checks and parsing
  if not os.path.exists(args.inputfile):
    raise Exception('ERROR: input file {} does not exist.'.format(args.inputfile))
  if not os.path.exists(args.outputdir):
    os.makedirs(args.outputdir)
  
  # read all histograms
  histlist = ht.loadallhistograms(args.inputfile)

  # loop over event selections
  for event_selection in args.event_selection:
    # loop over variables
    for varname, varlayout in variables.items():
      xaxtitle = varlayout
      print('now running on {} / {}...'.format(event_selection,varname))

      # select histograms
      thishists = ht.selecthistograms(histlist, mustcontainall=[event_selection,varname])[1]
      # additional selections for overlapping histogram names
      thishists = ([hist for hist in thishists if
                    (hist.GetName().endswith(varname) or varname+'_' in hist.GetName())])
      if len(thishists)==0:
        print('ERROR: histogram list for variable {} is empty,'.format(varname)
              +' skipping this variable.')
        continue
      if len(thishists)!=1:
        msg = 'ERROR: {} histograms found for variable {}'.format(len(thishists),varname)
        msg += ' while 2 were expected; check file content.'
        raise Exception(msg)
      hist = thishists[0]
    
      # set plot properties
      yaxtitle = 'Events (normalized)'
      outfile = os.path.join(args.outputdir, '{}_{}'.format(event_selection,varname))
      lumi = None
      extrainfos = []
    
      # make plot
      shp.plotsinglehistogram( hist, outfile, normalize=True,
                title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                label=None, color=None, logy=False, drawoptions='',
                do_cms_text=False, lumitext='', extralumitext='',
                xaxlabelfont=None, xaxlabelsize=None,
                yaxmin=None, yaxmax=None,
                extrainfos=extrainfos, infosize=None, infoleft=None, infotop=None )
