#############################################
# rename processes from Niels' sample lists #
#############################################

import sys
import os

def get_rename_dict():
  renamedict = {}
  renamedict['SSWW'] = 'SSWW'
  renamedict['TTTo'] = 'TT'
  renamedict['TTGamma_'] = 'TTG'
  renamedict['ttHJetTo'] = 'TTH'
  renamedict['ST_s-channel_'] = 'ST-S'
  renamedict['ST_t-channel_'] = 'ST-T'
  renamedict['ST_tW_'] = 'ST-TW'
  renamedict['THQ_'] = 'THQ'
  renamedict['TTTJ_'] = 'TTTJ'
  renamedict['TTTW_'] = 'TTTW'
  renamedict['tZq_'] = 'TZQ'
  renamedict['WW_'] = 'WW'
  renamedict['WWTo'] = 'WW'
  renamedict['WWW_'] = 'WWW'
  renamedict['WWZ_'] = 'WWZ'
  renamedict['WZZ'] = 'WZZ'
  renamedict['ZZZ'] = 'ZZZ'
  renamedict['WZTo'] = 'WZ'
  renamedict['DYJetsTo'] = 'DY'
  renamedict['ZGTo'] = 'ZG'
  renamedict['GluGluHTo'] = 'GGH'
  renamedict['GluGluToContin'] = 'GGZZ'
  renamedict['VHTo'] = 'VBFH'
  renamedict['ZZTo'] = 'ZZ'
  return renamedict

if __name__=='__main__':

  dtypes = ['sim','data']
  years = ['2016PreVFP','2016PostVFP','2017','2018']
  renamedict = get_rename_dict()

  for dtype in dtypes:
    for year in years:
      # define the filename to parse
      sname = 'samples_tttt_{}_{}.txt'.format(year,dtype)
      # read all lines of the file
      with open(sname,'r') as f:
        lines = f.readlines()
      # loop over lines
      newlines = []
      for line in lines:
        line = line.strip(' \t\n')
        # copy empty lines
        if len(line)<=1:
          newlines.append('')
          continue
        # remove commented lines
        if line[0]=='#': continue
        # get the parts
        lineparts = line.split('  ')
        process_name = lineparts[0]
        sample_name = lineparts[1]
        # remove path specifiers
        new_sample_name = sample_name.split('/')[-1]
        lineparts[1] = new_sample_name
        # replace the process name
        new_process_name = process_name
        for key,val in renamedict.items():
          if new_sample_name.startswith(key): new_process_name = val
        lineparts[0] = new_process_name
        # merge the modified parts back together
        newline = '  '.join(lineparts)
        # add the new line
        newlines.append(newline)
      # write the final file
      with open(sname,'w') as f:
        for line in newlines: f.write('{}\n'.format(line))
