################################################################
# Plot the results of a simultaneous ttW+ and ttW- measurement #
################################################################

import sys
import os
import argparse
import numpy as np
import ROOT
from array import array
sys.path.append('../../plotting/python')
import plottools as pt

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Plot combine output')
  parser.add_argument('-d', '--datacarddir', required=True, type=os.path.abspath)
  args = parser.parse_args()

  # fixed arguments
  xaxbins = '0.5,1.5,30'
  yaxbins = '0.5,1.5,30'

  # find suitable input files in directory
  inputfiles = [f for f in os.listdir(args.datacarddir)
                if( '_likelihoodscan_' in f and f.endswith('.MultiDimFit.mH120.root') )]
  
  # loop over input files
  for inputfile in inputfiles:
    inputfile = os.path.join(args.datacarddir, inputfile)
    outputfile = inputfile.replace('.root', '.png')
    # make the command
    cmd = 'python plusminus_plotoutput.py'
    cmd += ' --inputfile {}'.format(inputfile)
    cmd += ' --outputfile {}'.format(outputfile)
    cmd += ' --xaxbins {}'.format(xaxbins)
    cmd += ' --yaxbins {}'.format(yaxbins)

    cmd += ' --addinclusiveresult'
    #cmd += ' --datacarddir {}'.format(args.datacarddir)
    # run the command
    os.system(cmd)
