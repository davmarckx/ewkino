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
    if stdout is None: stdout = name+'_dout_$(ClusterId)_$(ProcId)'
    if stderr is None: stderr = name+'_derr_$(ClusterId)_$(ProcId)'
    if log is None: log = name+'_dlog_$(ClusterId)_$(ProcId)'
    # write file
    with open(fname,'w') as f:
        f.write('executable = {}\n'.format(exe))
        if argstring is not None: f.write('arguments = "{}"\n\n'.format(argstring))
        f.write('output = submitlog/{}\n'.format(stdout))
        f.write('error = submitlog/{}\n'.format(stderr))
        f.write('log = submitlog/{}\n\n'.format(log))
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

#largely incomplete
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

def initJobScript(name,filename, filetype, nr,
                  home=None,
                  cmssw_version="CMSSW_10_6_28",
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
        script.write("echo 'Start job!'\n")
        # write echo script name
        script.write("echo '###exename###: {}'\n".format(fname))
        # write export home
        if home is not None:
            script.write('export HOME={}\n'.format(home))
        # write sourcing of common software
        script.write('cd {}\n'.format( "~/CMSSW_10_6_28/src" ) )
        script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
        # write setting correct cmssw release
        if cmssw_version is not None:
            #script.write('cd {}\n'.format( os.path.join( cmssw_version,'src' ) ) )
            script.write('eval `scram runtime -sh`\n')
        # write export proxy
        if proxy is not None:
            script.write('export X509_USER_PROXY={}\n'.format( proxy ))
        script.write('cd {}\n'.format( cwd ) )
        script.write('python3.6 data2parquet.py ' + filename + ' ' + filetype + ' ' + str(nr) + '\n')
        script.write("echo 'End job!'\n")
    # make executable
    os.system('chmod +x '+fname)
    print('initJobScript created {}'.format(fname))


def makeJobDescription(name, exe, argstring=None,
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
    if stdout is None: stdout = 'submitlog/'+name+'_out_$(ClusterId)_$(ProcId)'
    if stderr is None: stderr = 'submitlog/'+name+'_err_$(ClusterId)_$(ProcId)'
    if log is None: log = 'submitlog/'+name+'_log_$(ClusterId)_$(ProcId)'
    # write file
    with open(fname,'w') as f:
        f.write('executable = {}\n'.format(exe + '.sh'))
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

years = ['2016PreVFP']#,'2016PostVFP','2017','2018' ]#CURRENTLY IMPLEMENTED FOR ALL YEARS, JUST RUN ONCE
dropval = 0.25
regdropval =  0.2
learr = 0.0001
beta1 = 0.05
beta2 = 0.1
batchsize = 50
epochs = 100
fract = 0.0001
njobs = 4


for year in years:
    commands = []
    commands.append("python3.9 trainGNN_selobj.py " + str(dropval) + " " + str(regdropval) + " " + str(learr) + " " + str(beta1) + " " + str(beta2) + " " + str(batchsize) + " " + str(epochs) + " " + str(fract) + " " + str(year) + " " + str(njobs) + ' True')

submitCommandsAsCondorCluster('cjob_trainGNN_selobjsparse', commands, stdout=None, stderr=None, log=None,
                        cpus=njobs, mem=2048, disk=10240,
                        home=None,
                        proxy=None,
                        cmssw_version="CMSSW_12_4_6",
                        jobflavour=None)
