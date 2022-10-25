#####################################################
# Merge samples for charge misid. measurement in MC #
#####################################################
# A charge misid. measurement was performed by Gianny for the 4tops analysis
# using the latest samples (UL, with UL TOP MVA).
# His samples are stored here: /pnfs/iihe/cms/store/user/gmestdac/4TTuples/singlelepton_MC_*v6/FINALV1/.
# But they are not yet completely merged
# (probably not needed in his framework, 
# but would be more convenient for us).

import sys
import os

if __name__=='__main__':

  years = ({
	    '2016PreVFP': '2016_ULpreVFPv6','Pre_2016',
	    '2016PostVFP': '2016_ULpostVFPv6','Post_2016',
	    '2017': '2017_ULv6',
	    '2018': '2018_ULv6'
	  })
  inputdir = '/pnfs/iihe/cms/store/user/gmestdac/4TTuples/singlelepton_MC_{}/FINALV1'
  outputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_sim/{}'

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
      # configure input and output files
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
