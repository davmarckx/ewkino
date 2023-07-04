##########################################################
# hadd goodness-of-fit test result files before plotting #
##########################################################

import sys
import os
import fnmatch
import argparse

if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Goodness of fit test')
    parser.add_argument('--workspace', required=True, type=os.path.abspath)
    # (note: workspace can be a single workspace (.root extension) or a directory.
    #  in the latter case, all goodness of fit output files in that directory will be used
    #  to create a summary plot)
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
          print('  - {}: {}'.format(arg,getattr(args,arg)))

    # argument checking
    if not os.path.exists(args.workspace):
        raise Exception('ERROR: workspace or directory {} does not exist.'.format(args.workspace))

    # get workspaces
    workspaces = []
    if args.workspace.endswith('.root'): workspaces = [args.workspace]
    else:
        workspaces = [f for f in os.listdir(args.workspace)
            if( f.endswith('.root') and (f.startswith('datacard_') or f.startswith('dc_')) )]
        workspaces = [os.path.join(args.workspace, f) for f in workspaces]
    workspaces = sorted(workspaces)
    print('Found following workspaces:')
    for f in workspaces: print('  - {}'.format(f))

    # loop over workspaces
    for workspace in workspaces:
        (datacarddir, basename) = os.path.split(workspace)
        corename = os.path.splitext(basename)[0]
        # find goodness of fit toy result files for this workspace
        toyfiles = sorted([f for f in os.listdir(datacarddir)
                    if( fnmatch.fnmatch(f, 'higgsCombine'+corename+'.GoodnessOfFit.mH120.*.root')
                        and 'mergedtoys' not in f) ])
        toyfiles = [os.path.join(datacarddir,f) for f in toyfiles]
        # define merged file
        mergedfile = 'higgsCombine'+corename+'.GoodnessOfFit.mH120.mergedtoys.root'
        mergedfile = os.path.join(datacarddir, mergedfile)
        # make hadd command
        cmd = 'hadd -f {}'.format(mergedfile)
        for f in toyfiles: cmd += ' {}'.format(f)
        # run hadd command
        os.system(cmd)
