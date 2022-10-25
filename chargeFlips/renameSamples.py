import sys
import os

inputdir = '/pnfs/iihe/cms/store/user/llambrec/dileptonskim_ttw_sim'
for root, dirs, files in os.walk(inputdir):
  for f in files:
    if f.endswith('.root'):
      oldf = os.path.join(root,f)
      f = f.replace('Pre_2016','_Summer20UL16MiniAODAPV')
      f = f.replace('Post_2016','_Summer20UL16MiniAOD')
      f = f.replace('_Summer20UL17','_Summer20UL17MiniAOD')
      f = f.replace('_Summer20UL18','_Summer20UL18MiniAOD')
      newf = os.path.join(root,f)
      os.system('mv {} {}'.format(oldf, newf))
