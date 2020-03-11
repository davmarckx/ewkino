####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['signalregion'] #,'wzcontrolregion','zzcontrolregion','zgcontrolregion']
years = ['2017']
events = ['MC']

for region in regions:
    for year in years:
        for eventtype in events:
            inputfolder = '~/Files/trileptonskim/'+year+eventtype
            outputfolder = '~/Files/'+region+'/'+year+eventtype
            cmd = 'python eventselector.py '+inputfolder+' '+outputfolder+' '+region
            print('executing '+cmd)
            os.system(cmd)

