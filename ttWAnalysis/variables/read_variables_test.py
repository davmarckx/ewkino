########################################
# test the reading of a variables json #
########################################

import sys
import os
sys.path.append('../../Tools/python')
import variabletools as vt

if __name__=='__main__':

  inputfile = sys.argv[1]
  
  varlist = vt.read_variables( inputfile )
  for var in varlist:
    print(var)
