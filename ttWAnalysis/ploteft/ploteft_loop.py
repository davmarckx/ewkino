#############################
# Simple looper for ploteft #
#############################

import sys
import os


if __name__=='__main__':

  # input directory from command line
  inputdir = sys.argv[1]
  
  # other settings
  years = 'auto'
  regions = 'auto'
  selectiontypes = 'auto'
  #variables = os.path.abspath('../variables/variables_particlelevel_single.json')
  variables = os.path.abspath('../variables/variables_particlelevel_double.json')

  cmds = []
  # loop over years, regions and selection types
  yeardirs = years
  if years=='auto': yeardirs = os.listdir(inputdir)
  for yeardir in yeardirs:
    yeardir = os.path.join(inputdir, yeardir)
    regiondirs = regions
    if regions=='auto': regiondirs = os.listdir(yeardir)
    for regiondir in regiondirs:
      regiondir = os.path.join(yeardir, regiondir)
      selectiontypedirs = selectiontypes
      if selectiontypes=='auto': selectiontypedirs = os.listdir(regiondir)
      for selectiontypedir in selectiontypedirs:
        selectiontypedir = os.path.join(regiondir, selectiontypedir)
        # loop over root files
        rootfiles = [f for f in os.listdir(selectiontypedir) if f.endswith('.root')]
        for rootfile in rootfiles:
          rootfile = os.path.join(selectiontypedir, rootfile)
          # set output directory
          outputdir = rootfile.replace('.root','_plots')
          # other settings
          regiontag = os.path.basename(regiondir) + '_' + os.path.basename(selectiontypedir)
          # make command
          cmd = 'python ploteft.py'
          cmd += ' --inputfile {}'.format(rootfile)
          cmd += ' --year {}'.format(os.path.basename(yeardir).split('_')[0])
          cmd += ' --region {}'.format(regiontag)
          cmd += ' --process TTW'
          cmd += ' --variables {}'.format(variables)
          cmd += ' --outputdir {}'.format(outputdir)
          cmds.append(cmd)

  # run the commands
  for cmd in cmds:
    print(cmd)
    os.system(cmd)
