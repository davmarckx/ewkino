#########################################
# Make plots on the basis of a datacard #
#########################################
# Use case: not intended for mass production of plots,
# more for debugging, by plotting exactly what is in a datacard.


import sys
import os
import argparse
sys.path.append(os.path.abspath('../../plotting/python'))
import histplotter as hp
sys.path.append(os.path.abspath('../plotting'))
import colors
import infodicts
sys.path.append(os.path.abspath('../analysis/python'))
from processinfo import ProcessInfoCollection, ProcessCollection


if __name__=="__main__":

  # parse arguments
  parser = argparse.ArgumentParser(description='Make datacard plots')
  parser.add_argument('-d', '--datacard', required=True, type=os.path.abspath)
  parser.add_argument('-f', '--histfile', required=True, type=os.path.abspath)
  parser.add_argument('-o', '--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('-t', '--tags', default=None,
                      help='Comma-separated list of additional info to display on plot'
                          +' (e.g. simulation year or selection region).'
                          +' Use underscores for spaces.')
  parser.add_argument('-c', '--colormap', default='default')
  parser.add_argument('-s', '--signals', default=None, nargs='+')
  parser.add_argument('--processtagmod', default=None)
  parser.add_argument('--extracmstext', default='Preliminary')
  parser.add_argument('--unblind', action='store_true')
  parser.add_argument('--dolog', action='store_true')
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

  # get the nominal simulated histograms
  simhists = []
  for process in PC.plist:
    simhists.append( PC.processes[process].hist )

  # get the uncertainty histogram
  mcsysthist = PC.get_systematics_rss()

  # get data histogram
  datahist = PC.datahist

  # modify the process names
  if args.processtagmod is not None:
    for hist in simhists:
      if args.processtagmod in hist.GetTitle():
        hist.SetTitle(hist.GetTitle().replace(args.processtagmod,''))

  # blind data histogram
  if not args.unblind:
    for i in range(0,datahist.GetNbinsX()+2):
      datahist.SetBinContent(i, 0)
      datahist.SetBinError(i, 0)

  # set plot properties
  xaxtitle = 'Fit variable'
  yaxtitle = 'Number of events'
  colormap = colors.getcolormap(style=args.colormap)
  extrainfos = extratags
  labelmap = None
  lumi = None
  binlabels = None
  labelsize = None
  labelangle = None
  canvaswidth = None
  canvasheight = None
  p1legendbox = None
  p1legendncols = None

  # make the plot
  outfile = os.path.splitext(args.outputfile)[0]
  hp.plotdatavsmc(outfile, datahist, simhists,
	    mcsysthist=mcsysthist, 
	    xaxtitle=xaxtitle,
	    yaxtitle=yaxtitle,
	    colormap=colormap,
            labelmap=labelmap,
            signals=args.signals,
            extrainfos=extrainfos,
	    lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            labelangle=labelangle,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox,
            p1legendncols=p1legendncols )

  if args.dolog:
    # make plot in log scale
    outfile = os.path.splitext(args.outputfile)[0]+'_log'
    hp.plotdatavsmc(outfile, datahist, simhists,
            mcsysthist=mcsysthist,
            xaxtitle=xaxtitle,
            yaxtitle=yaxtitle,
            colormap=colormap,
            labelmap=labelmap,
            signals=args.signals,
            extrainfos=extrainfos,
            lumi=lumi, extracmstext=args.extracmstext,
            binlabels=binlabels, labelsize=labelsize,
            labelangle=labelangle,
            canvaswidth=canvaswidth, canvasheight=canvasheight,
            p1legendbox=p1legendbox,
            p1legendncols=p1legendncols,
            yaxlog=True )
