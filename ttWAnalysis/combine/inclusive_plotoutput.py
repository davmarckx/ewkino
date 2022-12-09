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
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # read the channels
  channels = cp.readchanneltxt(args.channelfile)

  # set plot properties
  xaxtitle = 'Obs./Pred.'
  yaxtitle = 'Measurement channel'
  xaxcentral = 1

  # make plot
  cp.plotchannels(channels, args.outputfile,
                  showvalues=args.showvalues, font=None, fontsize=None,
                  xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                  lumi=None,
                  xaxcentral=xaxcentral,
                  xaxlinecoords=[], yaxlinecoords=[],
                  xaxrange=None, legendbox=None,
                  extracmstext='Preliminary' )
