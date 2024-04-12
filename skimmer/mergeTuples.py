#########################################
# merge tuples into one file per sample #
#########################################
# note: this functionality is supposed to be run on the output of skimTuples.py 
#       or skimTuplesFromList.py (important to extract sample name from folder name)
# note: the merging is done using simple hadd; it results in one file per sample / primary dataset.
#       for merging primary datasets, a more involved procedure should be applied
#	(see mergeAndRemoveOverlap)

# import python library classes 
import os
import sys
import fnmatch
import argparse

# import other parts of code 
from jobSubmission import submitQsubJob, initializeJobScript
from fileListing import walkLimitedDepth, listSampleDirectories
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION


# help functions

def listSkimmedSampleDirectories( input_directory ):
    # list all directories containing unmerged skimmed ntuples
    # note: naming convention depends on skimTuples.py / skimTuplesFromList.py
    for _, sample in listSampleDirectories( input_directory, 'ntuples_skimmed' ):
        yield  sample


def sampleNameIsData( sample_name ):
    # check if a sample name is data (as opposed to simulation).
    # it is checked whether the sample name starts with the name of a primary dataset.
    tags = ['SingleElectron', 'SingleMuon', 'EGamma',
	    'DoubleEG', 'DoubleMuon',
	    'MuonEG',
	    'JetHT', 'MET', 'HTMHT']
    for tag in tags: 
	if sample_name.startswith(tag): return True
    return False


def sampleName( directory_name ):
    # extract sample name
    # note: naming convention depends on skimTuples.py / skimTuplesFromList.py
    # for simulation, the part between "ntuples_skimmed_" and "_version_" is used as sample name;
    # for data, it is extended by an era identifier. 
    sample_name = directory_name.replace( 'ntuples_skimmed_','' )
    if('_version_' in sample_name): [sample_name, version] = sample_name.split( '_version_' )
    elif('_crab_' in sample_name): [sample_name, version] = sample_name.split( '_crab_' )
    else: raise Exception('ERROR in sampleName:'
           +' unrecognized folder structure: {}.'.format(sample_name))
    # in case of simulation: return current sample name
    if not sampleNameIsData(sample_name): return sample_name
    # in case of data: append era identifier
    idx = version.find('Run')
    era = version[idx:idx+8]
    if 'HIPM' in version: era += '_HIPM'
    sample_name = sample_name+'_'+era
    return sample_name


if __name__ == '__main__':

  # parse arguments
  parser = argparse.ArgumentParser('Merge tuples')
  parser.add_argument('-i', '--inputdir', required=True, type=os.path.abspath)
  parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath)
  parser.add_argument('--include_recovery', default=False )
  parser.add_argument('--runmode', default='condor', choices=['condor','local'])
  parser.add_argument('--searchkey', default=None)
  args = parser.parse_args()

  # print arguments
  print('Running with following configuration:')
  for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

  # argument checks and parsing
  if not os.path.exists(args.inputdir):
    raise Exception('ERROR: input directory {} does not exist.'.format(args.inputdir))
  if os.path.exists(args.outputdir):
    raise Exception('ERROR: output directory {} already exists.'.format(args.outputdir))

  # define the samples to merge
  mergedict = {}
  # loop over all directories in the provided top directory
  #for sample_directory in listSkimmedSampleDirectories( args.inputdir ):
  for sample_directory in os.listdir( args.inputdir ):
    # check if this sample should be taken into account
    if args.searchkey is not None:
      if not fnmatch.fnmatch(sample_directory,args.searchkey): continue
    # make corresponding full path and output file
    inputdir = os.path.join( args.inputdir, sample_directory )
    outputfile = os.path.join( args.outputdir, sampleName(sample_directory)+'.root' )
    # if requested: remove the 'recoveryTask' identifier
    # (this will have the effect that the original and recovery will be merged together)
    if( args.include_recovery ): outputfile.replace('recoveryTask','')
    # add to the merging dict
    if( outputfile in mergedict.keys() ): mergedict[outputfile].append(inputdir)
    else: mergedict[outputfile] = [inputdir]

  # print before continuing
  print('Found following tuples to merge:')
  for key, val in sorted(mergedict.items()):
    for el in val: print(' - {}'.format(el))
    print('  --> {}'.format(key))
  print('Continue? (y/n)')
  go = raw_input()
  if go!='y': sys.exit()

  # continue with the submission
  for outputfile, inputdirs in mergedict.items():
    cmd = 'hadd'
    cmd += ' {}'.format(outputfile)
    for inputdir in inputdirs: cmd += ' {}'.format(os.path.join(inputdir,'*.root'))
    outputdir = os.path.dirname(outputfile)
    if not os.path.exists(outputdir): os.makedirs(outputdir)
    if args.runmode=='local': os.system(cmd)
    elif args.runmode=='condor':
      ct.submitCommandAsCondorJob('cjob_mergeTuples', cmd, cmssw_version=CMSSW_VERSION)
