##########################################
# Read and plot output for default cases #
##########################################

import sys
import os

if __name__=='__main__':

  datacarddirs = sys.argv[1:]
  years = ['2016PreVFP', '2016PostVFP', '2017', '2018', 'run2']
  variables = '../variables/variables_particlelevel_double.json'
  includeinclusive = True
  obstags = ['exp', 'obs']
  crtags = ['nocr', 'withcr']

  commands = []
  for datacarddir in datacarddirs:
    dirtag = datacarddir.split('_')[-1].strip('/')
    for year in years:
      for crtag in crtags:
        for obstag in obstags:
          outputfile = 'summary_{}_{}_{}_{}.png'.format(dirtag, year, crtag, obstag)
          outputfile = os.path.join(datacarddir, outputfile)
          cmd = 'python difftotal_readoutput.py'
          cmd += ' --datacarddir {}'.format(datacarddir)
          cmd += ' --variables {}'.format(variables)
          cmd += ' --year {}'.format(year)
          cmd += ' --outputfile {}'.format(outputfile)
          if includeinclusive: cmd += ' --includeinclusive'
          if obstag=='obs': cmd += ' --usedata'
          if crtag=='withcr': cmd += ' --usecr'
          commands.append(cmd)

  for cmd in commands:
    print(cmd)
    os.system(cmd)
