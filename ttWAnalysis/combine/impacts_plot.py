###################################################################
# Impact plotting for differential measurement (non-default POIs) #
###################################################################

import sys
import os

if __name__=='__main__':

  # read input json file(s) from command line
  inputfiles = sorted(sys.argv[1:])

  # print input files
  print('Found following input files ({}):'.format(len(inputfiles)))
  for inputfile in inputfiles: print('  - {}'.format(inputfile))

  # loop over input files
  for inputfile in inputfiles:
    # make command
    cmd = 'python ../../plotting/python/plotimpacts.py'
    cmd += ' -i {}'.format(inputfile)
    cmd += ' -o {}'.format(inputfile.replace('.json',''))
    cmd += ' --POIs auto'
    # run command
    os.system(cmd)
