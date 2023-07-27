#######################################
# Merge output files from eventbinner #
#######################################
# Note: corresponds to new convention with one file per sample
#       (inclusve in event selections and selection types)
# The input consists of a collection of ROOT files containing histograms
# with the following naming convention (see also eventbinner.cc):
# <process tag>_<event selection>_<selection type>_<variable>.
# The output is a single file that is the hadd of the input files.
# Apart from the hadding, the following operations are also performed:
# - renaming processes (happens before merging) (optional)
# - remove redundant histograms (depending on npmode)
# - remove selection type from histogram name
#   (output file contains histograms named <process tag>_<event selection>_<variable>.)
# - clip all histograms (optional)


import sys
import os
import ROOT
import argparse
import json
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION
sys.path.append('../../Tools/python')
import histtools as ht
import listtools as lt
import argparsetools as apt


def rename_processes_in_file(renamedict, rfile, mode='custom'):
  ### rename processes for all histograms in a file
  # currently recommended mode: "fast"
  # (works and is sufficiently fast even for files with many histograms)

  if mode=='custom':
    # standard implementation.
    # works, but can be slow for files with many histograms.
    histlist = ht.loadallhistograms(rfile)
    for hist in histlist:
      histname = hist.GetName()
      pname,rem = histname.split('_',1)
      newpname = renamedict.get(pname,pname)
      newhistname = '_'.join([newpname,rem])
      hist.SetTitle(newpname)
      hist.SetName(newhistname)
      rf = ROOT.TFile.Open(rfile,'recreate')
      for hist in histlist: hist.Write()
      rf.Close()

  elif mode=='rootmv':
    # experimental implementation using "rootmv".
    # note that neither histogram names nor titles are updated,
    # only the key names.
    # seems to be even much slower than above.
    histnamelist = ht.loadallhistnames(rfile)
    for histname in histnamelist:
      pname,rem = histname.split('_',1)
      newpname = renamedict.get(pname,pname)
      newhistname = '_'.join([newpname,rem])
      cmd = 'rootmv {}:{} {}:{}'.format(rfile,histname,rfile,newhistname)
      print(cmd)
      os.system(cmd)

  elif mode=='fast':
    # implementation using SetName on the key rather than the histogram.
    # very fast compared to other methods.
    # note that neither histogram names nor titles are updated,
    # only the key names.
    f = ROOT.TFile.Open(rfile,'update')
    keylist = f.GetListOfKeys()
    for key in keylist:
      keyname = key.GetName()
      pname,rem = keyname.split('_',1)
      newpname = renamedict.get(pname,pname)
      newkeyname = '_'.join([newpname,rem])
      key.SetName(newkeyname)
    f.Close()

  else:
    raise Exception('ERROR in rename_processes_in_file:'
      +' mode {} not recognized.'.format(mode))


def decorrelate_systematics_in_file(renamedict, rfile, 
  year=None, allyears=None, mode='fast'):
  ### decorrelate systematics
  # currently recommended mode: "fast"
  # (works and is sufficiently fast even for files with many histograms)

  if mode=='fast':
    f = ROOT.TFile.Open(rfile,'update')
    keylist = f.GetListOfKeys()
    keynamelist = [str(key.GetName()) for key in keylist]
    def write_other_years(nomkeyname, keyname):
      ### inline help function to write nominal other years
      if nomkeyname not in keynamelist:
        raise Exception('ERROR in decorrelate_systematics_in_file:'
                        +' nominal histogram {} not found'.format(nomkeyname)
                        +' (required for found key {}'.format(keyname)
                        +' in file {})'.format(rfile))
      hist = f.Get(str(nomkeyname))
      hist = hist.Clone()
      for otheryear in allyears:
        if otheryear==year: continue
        hist.Write(newkeyname.replace(year,otheryear))
    # loop over all histogram keys
    for key, keyname in zip(keylist, keynamelist):
      newkeyname = keyname
      pname,rem = keyname.split('_',1)
      tag = keyname
      # find if histogram key belongs to up, down or other
      if tag.endswith('Up'): tag = tag[:-2]
      elif tag.endswith('Down'): tag = tag[:-4]
      else: continue
      # decorrelate by year
      # (change the key name to include the process,
      # but also add copies of nominal for the other years,
      # in order to merge several years correctly)
      if( year is not None and allyears is not None ):
        for s in renamedict['decorrelate_years']:
          if tag.endswith(s):
            newkeyname = keyname.replace(s, s+year)
            key.SetName(newkeyname)
            nomkeyname = tag.replace(s, '_nominal')
            write_other_years(nomkeyname, keyname)
      # special case: decorrelate JEC sources by year
      # (some JEC sources are already decorrelated per year in the input ntuples,
      # and not listed in renamedict so they are skipped above.)
      # (as an additional complication, the split JEC sources are combined for 2016
      # instead of being split in 2016PreVFP and 2016PostVFP.)
      if( 'JECGrouped' in tag and year is not None and allyears is not None ):
        if tag.endswith('2016'):
          newkeyname = keyname.replace('2016', year)
          key.SetName(newkeyname)
          tag = tag.replace('2016', year)
        if tag.endswith(year):
          # extract the full name of the systematic, e.g. JECGroupd_HF_2017
          systematic = 'JECGrouped' + tag.split('JECGrouped')[1]
          nomkeyname = tag.replace(systematic, 'nominal')
          write_other_years(nomkeyname, keyname)
      # decorrelate by process
      # (simply change the key name to include the process)
      for s in renamedict['decorrelate_processes']:
        if tag.endswith(s): newkeyname = keyname.replace(s, s+pname)
      key.SetName(newkeyname)
    f.Close()
  else:
    raise Exception('ERROR in decorrelate_systematics_in_file:'
      +' mode {} not recognized.'.format(mode))
 

def select_histograms_in_file(rfile, npmode, cfmode, mode='custom'):
  ### remove unneeded histograms from a file
  # and remove selection type tag from histogram name.
  # currently recommended mode: "custom"
  # (works but is very slow; to investigate if speedups are possible).
  # better to use mode "noselect" if no selection is needed
  # (e.g. if previous steps were split in selection types
  #  so hadd can work on needed files/histograms only).

  tags = []
  if( npmode=='npfromsim' and cfmode=='cffromsim' ): tags = ['_tight_']
  elif( npmode=='npfromdata' and cfmode=='cffromsim' ): 
    tags = ['_prompt_', '_fakerate_']
  elif( npmode=='npfromdatasplit' and cfmode=='cffromsim' ):
    tags = ['_prompt_', '_efakerate_', '_mfakerate_']
  elif( npmode=='npfromsim' and cfmode=='cffromdata' ): 
    tags = ['_chargegood_', '_chargeflips_']
  elif( npmode=='npfromdata' and cfmode=='cffromdata' ): 
    tags = ['_irreducible_', '_fakerate_', '_chargeflips_']
  elif( npmode=='npfromdatasplit' and cfmode=='cffromdata' ):
    tags = ['_irreducible_', '_efakerate_', '_mfakerate_', '_chargeflips_']
  else:
    raise Exception('ERROR in select_histograms_in_file:'
            +' invalid combination of npmode {}'.format(npmode)
            +' and cfmode {}'.format(cfmode))

  if mode=='custom':
    # standard implementation reading all histograms
    # and writing out the needed ones.
    histlist = ht.loadhistograms(rfile, mustcontainone=tags)
    if len(histlist)==0:
      print('WARNING in select_histograms_in_file: list of selected histograms is empty!')
    for hist in histlist:
      for tag in tags:
        hist.SetName( hist.GetName().replace(tag,'_') )
    rf = ROOT.TFile.Open(rfile,'recreate')
    for hist in histlist: hist.Write()
    rf.Close()

  elif mode=='fast':
    # implementation operating on keys rather than histograms.
    # works, but does not seem to be faster than custom method.
    # moreover, file size is not reduced despite the histogram removal.
    f = ROOT.TFile.Open(rfile,'update')
    keylist = f.GetListOfKeys()
    for key in keylist:
      keyname = key.GetName()
      hastag = False
      for tag in tags:
        if tag in keyname: hastag = True
      if not hastag:
        f.Delete('{};*'.format(keyname))
      else:
        newkeyname = keyname
        for tag in tags:
          newkeyname = newkeyname.replace(tag,'_')
        key.SetName(newkeyname)
    f.Close()

  elif mode=='noselect':
    # implementation without performing selection, only renaming.
    # works very fast (see also rename_processes_in_file)
    # but only works well if there are no unneeded histograms in the file.
    f = ROOT.TFile.Open(rfile,'update')
    keylist = f.GetListOfKeys()
    for key in keylist:
      keyname = key.GetName()
      hastag = False
      for tag in tags:
        if tag in keyname: hastag = True
      if not hastag:
        msg = 'ERROR in select_histograms_in_file:'
        msg += ' found histogram {}'.format(keyname)
        msg += ' which should be removed according to'
        msg += ' npmode {} and cfmode {},'.format(npmode, cfmode)
        msg += ' but this is not possible in mode "noselect".'
        raise Exception(msg)
      newkeyname = keyname
      for tag in tags: newkeyname = newkeyname.replace(tag,'_')
      key.SetName(newkeyname)
    f.Close()

  else:
    raise Exception('ERROR in select_histograms_in_file:'
      +' mode {} not recognized.'.format(mode))


def clip_histograms_in_file( rfile, mode='custom' ):
  ### clip all histograms in a file
  # currently recommended mode: "custom"
  # (works but is very slow; to investigate if speedups are possible).
  # better to not do clipping and instead do that downstream when needed
  # (e.g. in prefitplots.py or datacardtools.py)
  # (but not possible for e.g. mergeyears).
  
  if mode=='custom':
    histlist = ht.loadallhistograms(rfile)
    ht.cliphistograms(histlist)
    # re-write output file
    f = ROOT.TFile.Open(rfile,'recreate')
    for hist in histlist: hist.Write()
    f.Close()


def mergehists( selfiles, args ):
  ### main function

  # printouts
  print('Will merge the following files:')
  for f in selfiles: print('  - {}'.format(f))
  print('into {}'.format(args.outputfile))

  # make output directory
  outputdirname, outputbasename = os.path.split(args.outputfile)
  if not os.path.exists(outputdirname):
    os.makedirs(outputdirname)

  # make a temporary directory to store intermediate results
  tempdir = os.path.join(args.directory, 'temp_{}_{}'.format(args.npmode, args.cfmode))
  if not os.path.exists(tempdir):
    os.makedirs(tempdir)

  # copy all files to temporary directory
  tempfiles = []
  for i,f in enumerate(selfiles):
    filename = 'tempfile{}.root'.format(i)
    tempfile = os.path.join(tempdir,filename)
    os.system('cp {} {}'.format(f, tempfile))
    tempfiles.append(tempfile)

  # rename processes if requested
  if args.rename is not None:
    print('Renaming processes in all selected files...')
    with open(args.rename,'r') as f:
      renamedict = json.load(f)
    for i,f in enumerate(tempfiles):
      print('  - file {}/{}'.format(i+1,len(tempfiles)))
      sys.stdout.flush()
      rename_processes_in_file(renamedict, f, mode=args.renamemode)

  # decorrelate systematics if requested
  if args.decorrelate is not None:
    print('Decorrelating systematics in all selected files...')
    with open(args.decorrelate,'r') as f:
      renamedict = json.load(f)
    for i,f in enumerate(tempfiles):
      print('  - file {}/{}'.format(i+1,len(tempfiles)))
      sys.stdout.flush()
      decorrelate_systematics_in_file(renamedict, f, 
        year=args.decorrelateyear, allyears=args.decorrelateyears,
        mode=args.decorrelatemode)

  # do hadd to merge histograms
  # with same process, variable, and systematic.
  print('Running hadd...')
  sys.stdout.flush()
  cmd = 'hadd -f {}'.format(args.outputfile)
  for f in tempfiles: cmd += ' {}'.format(f)
  os.system(cmd)
  sys.stdout.flush()

  # delete temporary files
  os.system('rm -r {}'.format(tempdir))

  # select histograms to keep in the output
  # and remove selection type tag, as it is not needed anymore.
  if args.selectmode is not None:
    print('Selecting histograms to keep and removing selection type tag...')
    sys.stdout.flush()
    select_histograms_in_file(args.outputfile, args.npmode, args.cfmode, mode=args.selectmode)

  # clip all resulting histograms to minimum zero
  if args.doclip:
    print('Clipping all histograms to minimum zero...')
    sys.stdout.flush()
    clip_histograms_in_file(args.outputfile, mode='custom')

if __name__=='__main__':

  # parse arguments
  parser = argparse.ArgumentParser(description='Merge histograms')
  parser.add_argument('--directory', required=True, type=os.path.abspath)
  parser.add_argument('--outputfile', required=True, type=os.path.abspath)
  parser.add_argument('--npmode', required=True, choices=['npfromsim','npfromdata','npfromdatasplit'])
  parser.add_argument('--cfmode', required=True, choices=['cffromsim','cffromdata'])
  parser.add_argument('--rename', default=None, type=apt.path_or_none)
  parser.add_argument('--renamemode', default='fast', choices=['custom','rootmv','fast'])
  parser.add_argument('--decorrelate', default=None, type=apt.path_or_none)
  parser.add_argument('--decorrelatemode', default='fast', choices=['fast'])
  parser.add_argument('--decorrelateyear', default=None)
  parser.add_argument('--decorrelateyears', 
    default=['2016PreVFP','2016PostVFP','2017', '2018'], nargs='+')
  parser.add_argument('--selectmode', default='fast', choices=['custom','fast','noselect'])
  parser.add_argument('--doclip', default=False, action='store_true')
  parser.add_argument('--runmode', default='local', choices=['local','condor'])
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checking
  if not os.path.exists(args.directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.directory))
  if( args.rename is not None and not os.path.exists(args.rename) ):
    raise Exception('ERROR: rename file {} does not exist.'.format(args.rename))

  # handle job submission if requested
  if args.runmode=='condor':
    cmd = 'python mergehists.py'
    cmd += ' --directory '+args.directory
    cmd += ' --outputfile '+args.outputfile
    cmd += ' --npmode '+args.npmode
    cmd += ' --cfmode '+args.cfmode
    if args.rename is not None: 
      cmd += ' --rename '+args.rename
      cmd += ' --renamemode '+args.renamemode
    if args.decorrelate is not None:
      cmd += ' --decorrelate '+args.decorrelate
      cmd += ' --decorrelatemode '+args.decorrelatemode
      cmd += ' --decorrelateyear '+args.decorrelateyear
      cmd += ' --decorrelateyears '
      for y in args.decorrelateyears: cmd += ' '+y
    cmd += ' --selectmode '+args.selectmode
    if args.doclip: cmd += ' --doclip'
    cmd += ' --runmode local'
    ct.submitCommandAsCondorJob( 'cjob_mergehists', cmd,
                                 cmssw_version=CMSSW_VERSION )
    sys.exit()

  # get files to merge
  selfiles = ([os.path.join(args.directory,f) for f in os.listdir(args.directory) 
               if f[-5:]=='.root'])

  # call main function
  mergehists( selfiles, args )
