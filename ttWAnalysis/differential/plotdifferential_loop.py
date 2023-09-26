import sys
import os

if __name__=='__main__':

  # arguments
  inputdir = 'output_20230922'
  ssdirbase = '../combine/datacards_20230922_double_'
  regions = {'signalregion_dilepton_inclusive': ssdirbase + 'dilepton', 
             'signalregion_trilepton': ssdirbase + 'trilepton'}

  # basic command
  basiccmd = 'python plotdifferential.py --year run2 --processes TTW2018 --xsecs xsecs/xsecs.json'
  basiccmd += ' --variables ../variables/variables_particlelevel_single.json'

  # loop over configurations
  for region,ssdir in regions.items():
    for obstag in ['exp', 'obs']:
      for crtag in ['nocr', 'withcr']:
        inputfile = os.path.join(inputdir, region, 'particlelevel/merged.root')
        outputdir = os.path.join(os.path.dirname(inputfile), 'plots_{}_{}'.format(obstag,crtag))
        ssfile = os.path.join(ssdir, 'summary_{}_{}.json'.format(obstag, crtag))
        # customize command
        cmd = basiccmd
        cmd += ' --inputfile {}'.format(inputfile)
        cmd += ' --outputdir {}'.format(outputdir)
        cmd += ' --signalstrength {}'.format(ssfile)
        cmd += ' --region {}'.format(region)
        # run command
        print('Now running:')
        print(cmd)
        os.system(cmd)
