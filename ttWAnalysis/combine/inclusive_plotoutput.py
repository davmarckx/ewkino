############################################
# Plot the results in a datacard directory #
############################################

import sys
import os
import fnmatch
import argparse
sys.path.append('../../plotting/python')
import channelplotter as cp

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Plot combine output')
  parser.add_argument('--channelfile', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True)
  parser.add_argument('--showvalues', default=False, action='store_true')
  parser.add_argument('--plotInclusive', default=False, action='store_true')
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # read the channels
  channels = cp.readchanneltxt(args.channelfile)

  inclusivechannels=None
  if args.plotInclusive:
    print("printing per channel, with the results from the inclusive measurement:")
    inclusivechannels = [['all', 'all', 1.168, 0.053, 0.053, 0.087, 0.087], ['ee', 'ee', 1.137, 0.157, 0.157, 0.217, 0.217], ['em', 'em', 1.34, 0.082, 0.082, 0.122, 0.122], ['me', 'me', 1.34, 0.082, 0.082, 0.122, 0.122], ['mm', 'mm', 1.168, 0.0848, 0.0848, 0.109, 0.109], ['trilepton', '3L', 0.873, 0.139, 0.139, 0.190, 0.190]]
    

  # set plot properties
  xaxtitle = 'Obs./Pred.'
  yaxtitle = 'Measurement channel'
  xaxcentral = 1
  xaxrange = None if not args.showvalues else (-0.8,1.8)
  lumi = 138

  # make plot
  cp.plotchannels(channels, args.outputfile,
                  showvalues=args.showvalues, font=None, fontsize=None,
                  xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                  lumi=lumi,
                  xaxcentral=xaxcentral,
                  xaxlinecoords=[], yaxlinecoords=[],
                  xaxrange=xaxrange, legendbox=None,inclusiveChannels=inclusivechannels,
                  extracmstext='Preliminary' )
