##################################
# skim tuples from a sample list #
##################################
# functionality is similar to skimTuples.py, 
# but with usage of a sample list instead of automatically running over all samples in a directory.
# note: copied (with some updates and modifications) from tZq analysis code!

# command line arguments (in sequence):
# - input directory (e.g. /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino)
# - sample list (e.g. tzq_2016MC_samplelist.txt)
# - output directory (e.g. /storage_mnt/storage/user/llambrec/Files)
# - skim condition (see src/skimSelections.cc for valid options)
# - optional: ntuple version (e.g. singlelepton_MC_2016_v1) 
#             (overwrites ntuple version that may be present in the sample list!)
# - optional: files per job
# - optional: wall time per job
# - optional: run mode (condor, qsub or local)

import sys
import os
from fileListing import listSampleDirectories, listSampleFiles, listParts
from skimTuples import yearIdentifierFromPath
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
CMSSW_VERSION = '~/CMSSW_12_4_6'
sys.path.append(os.path.abspath('../Tools/python'))
from samplelisttools import readsamplelist

# check number of command line args
if len(sys.argv) < 5:
    print('### WARNING ###: too few command line arguments.')
    print('Use the following arguments:')
    print('  - input directory')
    print('  - sample list')
    print('  - output directory')
    print('  - skim condition')
    print('  - version = ntuple version [optional]')
    print('  - filesperjob = number of files per job [optional]')
    print('  - walltime = maximum wall time [optional]')
    print('  - runmode = run mode (condor, qsub or local) [optional]')
    sys.exit()

# read required command line args
input_directory = os.path.abspath(sys.argv[1])
sample_list = os.path.abspath(sys.argv[2])
output_directory_base = os.path.abspath(sys.argv[3])
skim_condition = sys.argv[4]

# read optional command line args
version_name = None
files_per_job = 50
wall_time = '24:00:00'
runmode = 'condor'
if len(sys.argv)>5:
    for sysarg in sys.argv[5:]:
        sysarg = sysarg.split('=')
        if sysarg[0]=='filesperjob': files_per_job = int(sysarg[1])
        elif sysarg[0]=='walltime': wall_time = sysarg[1]
        elif sysarg[0]=='version': version_name = sysarg[1]
	elif sysarg[0]=='runmode': runmode = sysarg[1]
        else: 
            raise Exception('ERROR: optional argument '+sysarg[0]+' not recognized!')

# print command line args for checking
print('Running with the follwing options:')
print('  - input directory: {}'.format(input_directory))
print('  - sample list: {}'.format(sample_list))
print('  - output directory: {}'.format(output_directory_base))
print('  - skim condition: {}'.format(skim_condition))
print('  - version: {}'.format(version_name))
print('  - files per job: {}'.format(files_per_job))
print('  - walltime: {}'.format(wall_time))
print('  - runmode: {}'.format(runmode))

# check if executable exists
exe = './skimmer'
if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not exist.'.format(exe))

# check if input directory exists
if not os.path.exists(input_directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(input_directory))

# check if sample list exist
if not os.path.exists(sample_list):
    raise Exception('ERROR: sample list {} does not exist.'.format(sample_list))

# check skim condition
allowed_skim_conditions = ["noskim", "singlelepton", "dilepton", 
			    "trilepton", "fourlepton", "fakerate",
			    "dilightlepton", "multilightlepton"]
if skim_condition not in allowed_skim_conditions:
    raise Exception('ERROR: skim condition {} not valid. Options are {}'.format(
			skim_condition, allowed_skim_conditions))

# check if output directory is empty and ask permission to clean it
if os.path.exists(output_directory_base):
    if not len(os.listdir(output_directory_base))==0:
	print('WARNING: output directory {} not empty.'.format(output_directory_base)
		+' Permission to clean it? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()
	os.system('rm -r {}'.format(os.path.join(output_directory_base,'*')))
else: os.makedirs(output_directory_base)

# make a list of sample names to use
print('Extracting sample names from the provided sample list...')
samples_to_use = []
useall = False
sample_collection = readsamplelist(sample_list)
for sample in sample_collection.get_samples():
    samples_to_use.append(sample.name)
if(len(samples_to_use)==1 and samples_to_use[0]=='all'):
    useall = True

# print sample names to use
print('Extracted {} sample names from the sample list:'.format(len(samples_to_use)))
for sample_to_use in samples_to_use: print('  - {}'.format(sample_to_use))

# make a list of samples (the main directories) 
# and an equally long list of subdirectories with the latest version of the ntuples 
# (one for each main directory)
print('Finding samples in provided input directory...')
sample_directories = []
sample_sub_directories = []
# case 1: version name is given as fixed argument for all samples
if version_name is not None:
    for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
        # extra selection using samplelist:
        sample_name = sample_directory.rstrip( os.path.sep ).split( os.path.sep )[-1]
        if(useall or sample_name in samples_to_use):
            sample_directories.append( sample_directory )
            sample_sub_directories.append( subdirectory )
# case 2: version name is extracted from the sample list
else:
    for sample in sample_collection.get_samples():
	if sample.version is None:
	    raise Exception('ERROR: sample version was not specified as a command line argument,'
			+' so it should be specified in the sample list, but found None'
			+' for sample {}'.format(sample.name))
	fullpath = os.path.join(input_directory, sample.name, sample.version)
        if os.path.exists(fullpath):
            sample_directories.append(os.path.join(input_directory,sample.name))
            sample_sub_directories.append(sample.version)
        else:
            print('WARNING: sample {} not found in input directory {}'.format(
		    os.path.join(sample.name,sample.version), input_directory))
            print('Continue without? (y/n)')
            go = raw_input()
            if not go=='y': sys.exit()

# printouts for testing
print('Found {} valid sample directories:'.format(len(sample_directories)))
for directory,subdirectory in zip(sample_directories,sample_sub_directories):
    print('  - {}/{}'.format(directory,subdirectory))

# determine the number of files for each sample
print('Finding number of files to process...')
nfiles = []
for sample_directory, sub_directory in zip(sample_directories, sample_sub_directories):
    nfiles.append( len(list(listSampleFiles(os.path.join(sample_directory,sub_directory)))) )
nfiles = sum(nfiles)
njobs = max(1,int(nfiles/files_per_job))
print('Found a total of {} files, which will result in approximately {} jobs.'.format(
	nfiles,njobs))
print('Continue with the submission? (y/n)')
go = raw_input()
if not go=='y': sys.exit()

# make output directory for each sample
print('Making output directories...')
sample_output_directories = []
for directory, subdirectory in zip(sample_directories, sample_sub_directories):
    sample_name = directory.rstrip( os.path.sep ).split( os.path.sep )[-1] 
    sample_name += '_' + yearIdentifierFromPath( subdirectory )
    output_directory = os.path.join( output_directory_base, 'ntuples_skimmed_{}_version_{}'
					.format( sample_name, subdirectory ) )
    if not os.path.exists( output_directory ):
        os.makedirs( output_directory )
    sample_output_directories.append( output_directory )

# loop over samples and submit skimming jobs
print('Starting submission...')
cwd = os.getcwd()
itlist = zip(sample_directories, sample_sub_directories, sample_output_directories)
for sample_directory, sub_directory, output_directory in itlist:
    #find the files to process for this sample 
    root_files = list( listSampleFiles( os.path.join( sample_directory, sub_directory ) ) )
    print('----------------------------')
    print('Now processing the following sample:')
    print('  {}'.format(sample_directory))
    print('  {}'.format(sub_directory))
    print('  Number of root files: {}'.format(len(root_files)))
    # split files in lists of files_per_job
    chunks = list(listParts( root_files, files_per_job ))
    for chunk in chunks:
	# make the commands to execute for this chunk
	commands = []
	commands.append( 'cd {}'.format(cwd) )
	for f in chunk:
	    command = './skimmer {} {} {}'.format(f,output_directory,skim_condition)
	    commands.append(command)
        # submission via qsub or local
	if( runmode=='qsub' or runmode=='local' ):
	    script_name = 'qjob_skimTuplesFromList.sh'
	    with open( script_name, 'w') as script:
		initializeJobScript( script )
		for command in commands:
		    script.write( command+'\n' )
	    # case of qsub 
	    if runmode=='qsub': submitQsubJob( script_name, wall_time )
	    # case of local (mainly intended for testing and debugging)
	    if runmode=='local': os.system('bash '+script_name)
	# submission via condor
	if runmode=='condor': ct.submitCommandsAsCondorJob( 'cjob_skimTuplesFromList', commands,
				    cmssw_version=CMSSW_VERSION )
