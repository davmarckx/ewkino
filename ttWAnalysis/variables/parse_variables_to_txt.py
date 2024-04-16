#######################
# convert json to txt #
#######################

import sys
import os
sys.path.append('../../Tools/python')
import variabletools as vt

if __name__=='__main__':

  # read input file(s)
  inputfile = sys.argv[1]
  if inputfile=='all':
    inputfiles = [f for f in os.listdir('.') if f.endswith('.json')]
  else: inputfiles = [inputfile]

  # loop over input files
  for inputfile in inputfiles:
    print('Parsing file {} to txt file...'.format(inputfile))
    
    # define and check output file
    outputfile = inputfile.replace('.json','.txt')
    if os.path.exists(outputfile):
      raise Exception('ERROR: output file already exists.')

    # read variables from json and write to txt
    varlist = vt.read_variables( inputfile )
    vt.write_variables_txt( varlist, outputfile ) 
