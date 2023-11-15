import sys
import os

if __name__=='__main__':

  # arguments
  inputdir = 'output_TTW'
  inputdir2 = 'output_TTWEFT'
  outputdirect = "~/public_html/EFT_study"
  ssdirbase = '../combine/datacards_EFTstudy_{}'
  regions = {'signalregion_dilepton_inclusive': ssdirbase}# + 'dilepton', 
             #'signalregion_trilepton': ssdirbase + 'trilepton'}
  write_roots = False
  efts = ["EFTcQq81","EFTcQq83","EFTcQei","EFTcQl3i","EFTcQlMi","EFTcQq11","EFTcQq13","EFTcbW","EFTcpQ3","EFTcpQM","EFTcpt","EFTcptb","EFTctG","EFTctW","EFTctZ","EFTctei","EFTctlSi","EFTctlTi","EFTctli","EFTctp","EFTctq1"]
  efts = ["EFTctq8"]

  variables = "../variables/variables_particlelevel_single.json"

  # basic command
  basiccmd = 'python plotdifferential.py --year run2 --processes TTW2018 --xsecs xsecs/xsecs.json'
  basiccmd += ' --variables {}'.format(variables)

  # loop over configurations
  for region,ssdir in regions.items():
    for obstag in ['obs']:
      for crtag in ['nocr']:
       for eft in efts:
        inputfile = os.path.join(inputdir, region, 'particlelevel/merged.root')
        inputfile2 = os.path.join(inputdir2, region, 'particlelevel/TTW_EFT_Autumn18.root')
        outputdir = os.path.join(os.path.dirname(outputdirect), 'plots_{}_{}_{}'.format(eft,obstag,crtag))
        ssfile = os.path.join(ssdir.format(eft), 'summary_{}_{}.json'.format(obstag, crtag))
        # customize command
        cmd = basiccmd
        cmd += ' --eft {}'.format(eft)
        cmd += ' --inputfile {}'.format(inputfile)
        cmd += ' --inputfile2 {}'.format(inputfile2)
        cmd += ' --outputdir {}'.format(outputdir)
        cmd += ' --signalstrength {}'.format(ssfile)
        cmd += ' --region {}'.format(region)
        if write_roots:
            cmd += ' --write_rootfiles'
        # run command
        print('Now running:')
        print(cmd)
        os.system(cmd)
