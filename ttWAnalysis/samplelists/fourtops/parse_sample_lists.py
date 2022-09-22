#############################################################
# parse sample lists from Niels' fourtops framework to here #
#############################################################

import sys
import os

if __name__=='__main__':

  dtypes = ['sim','data']
  years = ['2016PreVFP','2016PostVFP','2017','2018']
  giturl = 'https://raw.githubusercontent.com'
  baseurl = 'NielsVdBossche/ewkino/Experimental/sampleLists'

  for dtype in dtypes:
    for year in years:
      # define the filename to parse
      sname = 'samples_tttt_{}_{}.txt'.format(year,dtype)
      tempname = sname.replace('.txt','_temp.txt')
      # get the correct github url
      origin_name = '{}.txt'.format(year)
      if dtype=='data':
        origin_name = 'Data'+origin_name
        origin_name = origin_name.replace('20','')
      url = os.path.join(giturl, baseurl, origin_name)
      # download the file and rename it
      cmd = 'wget {}'.format(url)
      os.system(cmd)
      os.system('mv {} {}'.format(origin_name,tempname))
      # read all lines of the file
      with open(tempname,'r') as f:
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
        # first remove superfluous spaces
        lineparts = line.split(' ')
        lineparts = [p for p in lineparts if p!='']
        line = '  '.join(lineparts)
        # now remove path specifiers
        lineparts = line.split('  ')
        sample_name = lineparts[1]
        sample_name = sample_name.split('/')[-1]
        lineparts[1] = sample_name
        newline = '  '.join(lineparts)
        # add the new line
        newlines.append(newline)
      # write the final file
      with open(sname,'w') as f:
        for line in newlines: f.write('{}\n'.format(line))
      # remove temporary file
      os.system('rm {}'.format(tempname))
