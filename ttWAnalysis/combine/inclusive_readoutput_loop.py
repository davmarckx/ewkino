##########################################
# Read and plot output for default cases #
##########################################

import sys
import os

if __name__=='__main__':

  datacarddir = sys.argv[1]

  commands = []
  for obstag in ['exp','obs']:
    outtxtfile = os.path.join(datacarddir,'summary_{}.txt'.format(obstag))
    outpngfile = outtxtfile.replace('.txt','.png')
    cmd1 = 'python inclusive_readoutput.py'
    cmd1 += ' --datacarddir ' + datacarddir
    cmd1 += ' --outputfile ' + outtxtfile
    if obstag=='obs': cmd1 += ' --usedata'
    cmd1 += ' --usecombined'
    cmd2 = 'python inclusive_plotoutput.py'
    cmd2 += ' --channelfile ' + outtxtfile
    cmd2 += ' --outputfile ' + outpngfile
    cmd2 += ' --showvalues'
    commands.append(cmd1)
    commands.append(cmd2)

for cmd in commands:
  os.system(cmd)
