##########################################
# Read and plot output for default cases #
##########################################

import sys
import os

if __name__=='__main__':

  datacarddirs = sys.argv[1:-1]
  prediction = sys.argv[-1]
  obstags = ['exp', 'obs']
  crtags = ['nocr', 'withcr']

  commands = []
  for datacarddir in datacarddirs:
    dirtag = datacarddir.split('_')[-1].strip('/')
    for crtag in crtags:
      for obstag in obstags:
          inputfile = 'summary_{}_{}.json'.format(obstag, crtag)
          inputfile = os.path.join(datacarddir, inputfile)
          outputfile = 'summary_diffsum_{}_{}.png'.format(obstag, crtag)
          outputfile = os.path.join(datacarddir, outputfile)
          cmd = 'python diffsum_readoutput.py'
          cmd += ' --diffjson {}'.format(inputfile)
          cmd += ' --prediction {}'.format(prediction)
          cmd += ' --outputfile {}'.format(outputfile)
          commands.append(cmd)

  for cmd in commands:
    print(cmd)
    os.system(cmd)
