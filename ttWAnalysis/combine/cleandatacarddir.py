#############################
# Clean data card directory #
#############################

import sys
import os
sys.path.append(os.path.abspath('../../Tools/python'))
import combinetools as cbt


if __name__=='__main__':

  datacarddir = sys.argv[1]

  # argument checking
  if not os.path.exists(datacarddir):
    raise Exception('ERROR: input directory {} does not exist.'.format(datacarddir))
  
  # clean the data card dir
  _ = cbt.cleandatacarddir( datacarddir )
