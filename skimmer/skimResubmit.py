################################
# resubmit stuck skimming jobs #
################################

import os
import sys
sys.path.append('../jobSubmission')
import jobCheck as jc
 
if __name__=='__main__':

    skimsubmitter = sys.argv[1]
    
    # parse the skim submitter name
    skimsubmitter = skimsubmitter.replace('.py','').strip(' ')

    # find the relevant error log files, for now restricted to:
    # - only condor (not qsub)
    # - only skimming (not merging)
    allefiles = []
    allefiles += [fname for fname in os.listdir(os.getcwd()) 
		    if ('cjob_' in fname and '_err_' in fname and skimsubmitter in fname)]
    print('found {} error log files.'.format(len(allefiles)))
    print('start scanning...')

    # loop over all error log files found above
    # and find those corresponding to unfinished jobs
    efiles = []
    for efile in allefiles:
        # error checking
        if jc.check_start_done(efile): efiles.append(efile)
    print('found {} error log files corresponding to unfinished jobs.'.format(len(efiles)))

    # sort the list of error log files
    efiles = sorted(efiles)
    
    # get corresponding job ids
    jobids = []
    for efile in efiles:
	jobid = efile.split('_')[3]
        jobids.append(jobid)

    # check corresponding output log files and find corresponding executable
    shfiles = []
    for efile in efiles:
	ofile = efile.replace('_err_', '_out_')
	with open(ofile,'r') as f:
	   firstline = f.readline()
	shfile = firstline.replace('###exename###: ', '').strip(' \n')
	shfiles.append(shfile)

    # do some printing and checks
    print('found {} executables to resubmit:'.format(len(shfiles)))
    for i in range(len(shfiles)):
	print(' - job {} -> executable {}'.format(jobids[i], shfiles[i]))
    print('continue with resubmission?')
    go = raw_input()
    if go!='y': sys.exit()

    # remove the corresponding jobs
    print('canceling jobs and corresponding log files that will be resubmitted...')
    for jobid in jobids:
	os.system('condor_rm {}'.format(jobid))

    # modify the job description file and submit jobs
    print('resubmitting...')
    jobfile = 'cjob_'+skimsubmitter+'.txt'
    with open(jobfile,'r') as f:
	lines = f.readlines()
    for shfile in shfiles:
	lines[0] = 'executable = {}\n'.format(shfile)
	with open(jobfile,'w') as f:
	    for line in lines: f.write(line)
	os.system('condor_submit {}'.format(jobfile))

    # remove old version of error, output and log files
    print('removing old log files...')
    for i in range(len(shfiles)):
        efile = efiles[i]
        ofile = efile.replace('_err_', '_out_')
        lfile = efile.replace('_err_', '_log_')
        os.system('rm {}'.format(efile))
        os.system('rm {}'.format(ofile))
        os.system('rm {}'.format(lfile))
    
    print('done')
