##################################
# tools for reading sample lists #
##################################

import sys
import os
import argparse
import argparsetools as apt

class Sample(object):

    def __init__( self ):
	self.name = None
	self.process = None
	self.version = None
	self.path = None
	self.xsec = 0.

    def read_from_line( self, line, sampledir=None, **kwargs ):
	### read sample properties from a sample list line.
	# input arguments:
	# - line: string representing a line from a sample list.
	#         expected format: <process> <sample name>/<version name> <xsec>
	#         where the version name and cross-section are optional.
	# - sampledir: if specified, set the full sample path and check if it exists.
	# - kwargs: passed down to Sample.set_path

	# split the line by spaces
        line = line.strip(' ').split(' ')
	# remove extra spaces
	line = [el for el in line if el!='']
	# first extract the tag (short name) of the process
        self.process = line[0]
	# now extract sample name (and version name if present)
        self.name = line[1].rstrip('\n')
        if '/' in self.name: self.name, self.version = self.name.split('/')
        # finally extract cross-section
        if len(line)>2:
	    xsstr = line[2].rstrip('\n')
            try: self.xsec = float(xsstr)
            except: print('WARNING in Sample.read_from_line:'
			    +' found incompatible cross-section "{}";'.format(xsstr)
			    +' using zero as default.')
	# set the path for this sample
	if sampledir is not None: self.set_path( sampledir, **kwargs )

    def set_path( self, sampledir, suppress_exception=False ):
	### set the path attribute
	path = os.path.join(sampledir, self.name)
	if not os.path.exists(path):
	    if suppress_exception: return
	    raise Exception('ERROR in Sample.set_path:'
			    +' path {} does not exist.'.format(path))
	self.path = path

    def __str__( self ):
	res = 'Sample( name: {}'.format(self.name)
	res += ', process: {}'.format(self.process)
	res += ', version: {}'.format(self.version)
	res += ', xsec: {}'.format(self.xsec)
	res += ', path: {} )'.format(self.path)
	return res
	

class SampleCollection(object):

    def __init__( self ):
	self.samples = []

    def read_from_file( self, samplelistpath, **kwargs ):
	self.read_from_files( [samplelistpath], **kwargs )

    def read_from_files( self, samplelistpaths, **kwargs ):
	### read sample collection from a sample list file.
	# input arguments:
	# - samplelistpaths: list of sample list paths
	# - kwargs: passed down to Sample.read_from_line
	for samplelist in samplelistpaths:
	    with open(samplelist) as f:
		for line in f:
		    # strip
		    line = line.strip(' \t\n')
		    # ignore blank or commented lines
		    if(len(line)<=1): continue
		    if(line[0] == '#'): continue
		    # make a sample
		    sample = Sample()
		    sample.read_from_line(line, **kwargs)
		    # add the sample to the list
		    self.samples.append(sample)

    def get_samples( self, unique=False ):
	### return the sample collection
	# input arguments:
	# - unique: if True, return only samples with different sample names
	#           (will return only one instance of samples with same name
	#           but different version names)
	if not unique: return self.samples
	unique_sample_names = []
	unique_samples = []
	for sample in self.samples:
	    if sample.name not in unique_sample_names:
		unique_sample_names.append(sample.name)
		unique_samples.append(sample)
	return unique_samples

    def number( self, unique=False ):
	### return number of samples
	# input arguments:
	# - unique: see get_samples
	return len(self.get_samples(unique=unique))

    def check_paths( self, return_list=False ):
	### check if all samples have an existing path assigned
	# input arguments:
	# - return list: if False, return a bool (True if all good, False otherwise);
	#                if True, return a list of samples with no valid path.
	missing_samples = []
	for sample in self.samples:
	    if( sample.path is None or not os.path.exists(sample.path) ):
		missing_samples.append(sample)
	if return_list: return missing_samples
	else: return (len(missing_samples)==0)

    def __str__( self ):
	return '\n'.join(['{}'.format(s) for s in self.samples])


def readsamplelist( samplelistpaths, sampledir=None ):
    ### returns a SampleCollection from a list of sample list files
    # input arguments:
    # - samplelistpaths: either string or list of strings 
    #                    representing path(s) to sample list file(s).
    # - sampledir: path to directory containing the samples.
    #              if not specified, the path attribute of each sample is None
    #              and no check on sample existence is done.
    # note: each line of the sample list is assumed to be of the form
    #       <process_name> <sample_name>/<version_name> <cross_section>
    #       where the individual elements are separated by spaces 
    #       and should not contain spaces,
    #	    the version_name is optional (defaults to empty string),
    #       and the cross_section is optional (defaults to 0)
    
    collection = SampleCollection()
    if isinstance(samplelistpaths, str): samplelistpaths = [samplelistpaths]

    for s in samplelistpaths:
	if not os.path.exists(s):
	    raise Exception('ERROR in readsamplelist:'
		    +' sample list {} does not exist.'.format(s))
    if( sampledir is not None and not os.path.exists(sampledir) ):
	raise Exception('ERROR in readsamplelist:'
		    +' sample directory {} does not exist.'.format(sampledir))

    collection.read_from_files( samplelistpaths, sampledir=sampledir,
		suppress_exception=True )

    if sampledir is not None:
	missing_samples = collection.check_paths( return_list=True )
	if len(missing_samples)>0:
	    msg = 'ERROR in readsamplelist:'
	    msg += ' the following samples were not found in {}:\n'.format(sampledir)
	    for s in missing_samples: msg += '{}\n'.format(s)
	    raise Exception(msg)

    return collection


if __name__=='__main__':

    parser = argparse.ArgumentParser(description='Read sample list')
    parser.add_argument('--samplelist', required=True, type=os.path.abspath)
    parser.add_argument('--sampledir', type=apt.path_or_none)
    args = parser.parse_args()

    samples = readsamplelist( args.samplelist, sampledir=args.sampledir )
    print(samples)
