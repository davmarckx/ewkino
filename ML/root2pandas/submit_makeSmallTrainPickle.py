import os
import sys
import uproot

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random


def submitCondorJob(jobDescription):
    ### submit a job description file as a condor job
    fname = os.path.splitext(jobDescription)[0]+'.sub'
    if not os.path.exists(fname):
        print('ERROR: job description file {} not found'.format(fname))
        sys.exit()
    # maybe later extend this part to account for failed submissions etc!
    os.system('condor_submit {}'.format(fname))



def initJobScript_orig(name,
                  home=None,
                  cmssw_version=None,
                  proxy=None):
    ### initialize an executable bash script with:
    # - setting HOME environment variable
    #   note: use 'auto' to extract the HOME from os.environ
    # - sourcing the shared cms software
    # - setting correct cmssw release
    # - exporting proxy

    # parse filename
    name = os.path.splitext(name)[0]
    fname = name+'.sh'
    if os.path.exists(fname): os.system('rm {}'.format(fname))
    cwd = os.path.abspath(os.getcwd())
    # parse home
    if home=='auto': home = os.environ['HOME']
    # write script
    with open(fname,'w') as script:
        # write bash shebang
        script.write('#!/bin/bash\n')
        # write echo script name
        script.write("echo '###exename###: {}'\n".format(fname))
        # write export home
        if home is not None:
            script.write('export HOME={}\n'.format(home))
        # write sourcing of common software
        script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
        # write setting correct cmssw release
        if cmssw_version is not None:
            script.write('cd {}\n'.format( os.path.join( '~/' + cmssw_version,'src' ) ) )
            script.write('eval `scram runtime -sh`\n')
        # write export proxy
        if proxy is not None:
            script.write('export X509_USER_PROXY={}\n'.format( proxy ))
        script.write('cd {}\n'.format( cwd ) )
    # make executable
    os.system('chmod +x '+fname)
    print('initJobScript created {}'.format(fname))

def makeJobDescription_orig(name, exe, argstring=None,
                       stdout=None, stderr=None, log=None,
                       cpus=1, mem=1024, disk=10240,
                       proxy=None, jobflavour=None):
    ### create a single job description txt file
    # note: exe can for example be a runnable bash script
    # note: argstring is a single string containing the arguments to exe (space-separated)
    # note: for job flavour: see here: https://batchdocs.web.cern.ch/local/submit.html

    # parse arguments
    name = os.path.splitext(name)[0]
    fname = name+'.sub'
    if os.path.exists(fname): os.system('rm {}'.format(fname))
    if stdout is None: stdout = 'submitlog/' + name+'_out_$(ClusterId)_$(ProcId)'
    if stderr is None: stderr = 'submitlog/' + name+'_err_$(ClusterId)_$(ProcId)'
    if log is None: log = 'submitlog/' + name+'_log_$(ClusterId)_$(ProcId)'
    # write file
    with open(fname,'w') as f:
        f.write('executable = {}\n'.format(exe))
        if argstring is not None: f.write('arguments = "{}"\n\n'.format(argstring))
        f.write('output = {}\n'.format(stdout))
        f.write('error = {}\n'.format(stderr))
        f.write('log = {}\n\n'.format(log))
        f.write('request_cpus = {}\n'.format(cpus))
        f.write('request_memory = {}\n'.format(mem))
        f.write('request_disk = {}\n'.format(disk))
        if proxy is not None:
            f.write('x509userproxy = {}\n'.format(proxy))
            f.write('use_x509userproxy = true\n\n')
        #f.write('should_transfer_files = yes\n\n') 
        # (not fully sure whether to put 'yes', 'no' or omit it completely)
        if jobflavour is not None:
            f.write('+JobFlavour = "{}"\n\n'.format(jobflavour))
        f.write('queue\n\n')
    print('makeJobDescription created {}'.format(fname))

def submitCommandsAsCondorCluster(name, commands, stdout=None, stderr=None, log=None,
                        cpus=1, mem=1024, disk=10240,
                        home=None,
                        proxy=None,
                        cmssw_version=None,
                        jobflavour=None):
    ### run several similar commands within a single cluster of jobs
    # note: each command must have the same executable and number of args, only args can differ!
    # note: commands can be a list of commands (-> a job will be submitted for each command)

    # parse arguments
    name = os.path.splitext(name)[0]
    shname = name+'.sh'
    jdname = name+'.sub'
    [exe,argstring] = commands[0].split(' ',1) # exe must be the same for all commands
    nargs = len(argstring.split(' ')) # nargs must be the same for all commands
    # first make the executable
    initJobScript_orig(name,home=home, cmssw_version=cmssw_version, proxy=proxy)
    with open(shname,'a') as script:
        script.write(exe)
        script.write(' $@')
        script.write('\n')
    # then make the job description
    # first job:
    makeJobDescription_orig(name,shname,argstring=argstring,stdout=stdout,stderr=stderr,log=log,
                       cpus=cpus,mem=mem,disk=disk,proxy=proxy,
                       jobflavour=jobflavour)
    # add other jobs:
    with open(jdname,'a') as script:
        for command in commands[1:]:
            [thisexe,thisargstring] = command.split(' ',1)
            thisnargs = len(thisargstring.split(' '))
            if( thisexe!=exe or thisnargs!=nargs):
                print('### ERROR ###: commands are not compatible to put in same cluster')
                return
            script.write('arguments = "{}"\n'.format(thisargstring))
            script.write('queue\n\n')
    # finally submit the job
    submitCondorJob(jdname)
#################################################################################################################

years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
regions = ['dilep_BDT']

for year in years:
    commands = []
    for region in regions:
        commands.append("python3.6 makeSmallTrainPickle.py " + year + ' ' + region)


    submitCommandsAsCondorCluster('cjob_makeSmallTrainPickle', commands, stdout=None, stderr=None, log=None,
                        cpus=1, mem=2048, disk=10240,
                        home=None,
                        proxy=None,
                        cmssw_version="CMSSW_10_6_28",
                        jobflavour=None)

