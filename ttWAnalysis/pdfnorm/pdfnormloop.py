#########################
# Looper for pdfnorm.py #
#########################

import os
import sys

years = ['2016PreVFP', '2016PostVFP', '2017', '2018']

inputdirbase = '/pnfs/iihe/cms/store/user/nivanden/skims_v4/{}'

samplelistbase = 'samplelists/samples_tttt_{}_sim.txt'

outputdirbase = 'output_test/{}'

for year in years:
  inputdir = inputdirbase.format(year)
  outputdir = outputdirbase.format(year)
  samplelist = samplelistbase.format(year)
  cmd = 'python pdfnorm.py'
  cmd += ' --inputdir {}'.format(inputdir)
  cmd += ' --samplelist {}'.format(samplelist)
  cmd += ' --outputdir {}'.format(outputdir)
  cmd += ' --runmode local'
  os.system(cmd)
  cmd = 'python pdfnormplot.py'
  cmd += ' --inputdir {}'.format(outputdir)
  cmd += ' --samplelist {}'.format(samplelist)
  os.system(cmd)
