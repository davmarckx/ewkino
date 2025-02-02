###################################
# (Re-) plot the charge flip maps #
###################################

import sys
import os
import argparse
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
from hist2dplotter import plot2dhistogram

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Plot charge flip maps')
  parser.add_argument('--inputdir', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))

  # find and loop over all root files in the input directory
  inputfiles = [f for f in os.listdir(args.inputdir) if f.endswith('.root')]
  for f in inputfiles:
    # get properties from filename
    nameparts = f.replace('.root','').split('_')
    flavour = nameparts[2]
    year = nameparts[3]
    process = nameparts[5]
    binning = nameparts[7]
    print('Running on file {} with following properties:'.format(f))
    print('  - year: {}'.format(year))
    print('  - flavour: {}'.format(flavour))
    print('  - process: {}'.format(process))
    print('  - binning: {}'.format(binning))
    # load the charge flip map
    histlist = ht.loadhistograms(os.path.join(args.inputdir,f), 
		mustcontainall=['chargeFlipRate_{}_{}'.format(flavour,year)])
    if len(histlist)!=1:
	raise Exception('ERROR in file {}:'.format(f)
		+' found {} histograms while 1 was expected.'.format(len(histlist)))
    hist = histlist[0]
    # set plot properties
    title = 'Simulated charge misid. map for {} {}s'.format(year,flavour)
    extrainfos = (['(Measurement process: {},'.format(process)
                 +' binning: {})'.format(binning)])
    figname = f.replace('.root','')
    figname = os.path.join(args.inputdir,figname)
    xaxtitle = hist.GetXaxis().GetTitle()
    yaxtitle = hist.GetYaxis().GetTitle()
    zaxtitle = 'Charge misid. rate'
    # make plot
    plot2dhistogram(hist, figname, outfmts=['.png','.pdf'],
                    histtitle=title,
                    xtitle=xaxtitle, ytitle=yaxtitle, ztitle=zaxtitle,
                    xtitleoffset=None, ytitleoffset=1.2, ztitleoffset=None,
                    axtitlesize=None,
                    titlesize=None,
                    drawoptions='colztexte', cmin=None, cmax=None,
		    bintextoptions='4.2e',
                    docmstext=False,
                    topmargin=None, bottommargin=None, leftmargin=0.1, rightmargin=0.2,
                    extrainfos=extrainfos, infoleft=0.1, infotop=0.92)
