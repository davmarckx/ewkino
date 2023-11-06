######################################################
# Simple folder merger for the output of eventbinner #
######################################################
# Run after eventbinner and before mergehists,
# in order to merge the folders <year>_sim and <year>_data into one.
# The files in those folders and the histograms they contain are left untouched.

import sys
import os

def mergefolders( inputdir, years, remove ):
  # loop over years
  for year in years:
    simdir = os.path.join(inputdir, year+'_sim')
    datadir = os.path.join(inputdir, year+'_data')
    sigdir = os.path.join(inputdir, year+'_sig')
    bkgdir = os.path.join(inputdir, year+'_bkg')
    mergeddir = os.path.join(inputdir, year)
    # case 1: data and sim:
    if( os.path.exists(simdir) and os.path.exists(datadir) ):
        # make the commands
        cmds = []
        if remove:
          cmds.append( 'cp -rl {} {}'.format(os.path.join(datadir,'*'),simdir) )
          cmds.append( 'rm -r {}'.format(datadir) )
          cmds.append( 'mv {} {}'.format(simdir,mergeddir) )
          # run the commands
          for cmd in cmds: os.system(cmd)
        else:
          cmds.append( 'mkdir {}'.format(mergeddir) )
          cmds.append( 'cp -rl {} {}'.format(os.path.join(datadir,'*'),mergeddir) )
          cmds.append( 'cp -rl {} {}'.format(os.path.join(simdir,'*'),mergeddir) )
          # run the commands
          for cmd in cmds: os.system(cmd)
    # case 2: data, sig and bkg
    if( os.path.exists(sigdir) and os.path.exists(bkgdir) and os.path.exists(datadir) ):
        # make the commands
        cmds = []
        if remove:
          cmds.append( 'cp -rl {} {}'.format(os.path.join(datadir,'*'),bkgdir) )
          cmds.append( 'rm -r {}'.format(datadir) )
          cmds.append( 'cp -rl {} {}'.format(os.path.join(sigdir,'*'),bkgdir) )
          cmds.append( 'rm -r {}'.format(sigdir) )
          cmds.append( 'mv {} {}'.format(bkgdir,mergeddir) )
          # run the commands
          for cmd in cmds: os.system(cmd)
        else: raise Exception('Not yet implemented')
    else:
        print('Some required directories not found for year {}, skipping.'.format(year))
        continue


if __name__=='__main__':

  # settings
  inputdir = sys.argv[1]
  if len(sys.argv) == 3:
      remove = (sys.argv[2] == "1")
  else:
      remove = True
  years = ['2016PreVFP','2016PostVFP','2017','2018']

  # call main function
  mergefolders(inputdir, years, remove)
