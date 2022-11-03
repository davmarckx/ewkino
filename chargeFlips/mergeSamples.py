#####################################################
# Merge samples for charge misid. measurement in MC #
#####################################################
# A charge misid. measurement was performed by Gianny for the 4tops analysis
# using the latest samples (UL, with UL TOP MVA).
# His samples are stored here: /pnfs/iihe/cms/store/user/gmestdac/4TTuples/singlelepton_MC_*v6/FINALV1/.
# But they are not yet completely merged
# (probably not needed in his framework, 
# but would be more convenient for us).
# Update: now also include data.

import sys
import os

if __name__=='__main__':

  # for MC
  '''
  years = ({
	    '2016PreVFP': '2016_ULpreVFPv6',
	    '2016PostVFP': '2016_ULpostVFPv6',
	    '2017': '2017_ULv6',
	    '2018': '2018_ULv6'
	  })
  inputdir = '/pnfs/iihe/cms/store/user/gmestdac/4TTuples/singlelepton_MC_{}/FINALV1'
  outputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_chargeflips/sim/{}'
  '''

  # for data
  years = ({
            '2016PreVFP': 'UL2016V2',
            '2016PostVFP': 'UL2016V2',
            '2017': 'UL2017V2',
            '2018': 'UL2018V2'
          })
  inputdir = '/pnfs/iihe/cms/store/user/gmestdac/4TTuples/singlelepton_data_{}/FINALV1'
  outputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_chargeflips/data/{}'

  cmds = []

  # loop over years
  for outputyear, inputyear in years.items():
    # configure input and output dirs
    thisinputdir = inputdir.format(inputyear)
    thisoutputdir = outputdir.format(outputyear)
    if not os.path.exists(thisoutputdir):
      os.makedirs(thisoutputdir)
    # loop over samples
    samples = os.listdir(thisinputdir)
    for sample in samples:
      thissampledir = os.path.join(thisinputdir,sample)
      # for data, 2016PreVFP and PostVFP samples are in same input directory,
      # but need to be separated in the output directory (for consistency)
      if( 'Pre_2016' in sample and outputyear!='2016PreVFP' ): continue
      if( 'Post_2016' in sample and outputyear!='2016PostVFP' ): continue
      # rename target file so the year can be extracted correctly by ewkino framework
      targetf = sample+'.root'
      targetf = targetf.replace('Pre_2016','_Summer20UL16MiniAODAPV')
      targetf = targetf.replace('Post_2016','_Summer20UL16MiniAOD')
      targetf = targetf.replace('_2017','_Summer20UL17')
      targetf = targetf.replace('_2018','_Summer20UL18')
      targetf = os.path.join(thisoutputdir,targetf)
      inputf = os.listdir(thissampledir)
      inputf = [os.path.join(thissampledir,f) for f in inputf]
      # configure merge command
      cmd = 'python ../skimmer/mergeHadd.py -f'
      cmd += ' {}'.format(targetf)
      for f in inputf: cmd += ' {}'.format(f)
      cmds.append(cmd)

  # run commands
  for cmd in cmds: os.system(cmd)
