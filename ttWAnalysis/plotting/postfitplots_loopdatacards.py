##############################
# Looper for postfitplots.py #
##############################

# Use case: make postfit (and prefit) plots of all datacards in a directory.
# Note: this is similar to plotdatacards/datacardplots_loop.py,
#       but including postfit plots instead of only prefit plots.
# Note: this loop assumes all workspaces already exist (in the datacard directory),
#       and no custom workspaces for plotting need to be produced
#       (contrary to postfitplots_loop.py).


import os
import sys
import argparse


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Make postfit plots')
    parser.add_argument('-d', '--datacarddir', required=True, type=os.path.abspath)
    parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath)
    parser.add_argument('-f', '--fitresultfile', default=None)
    parser.add_argument('-v', '--variables', default=None,
                        help='Path to json file holding variable definition.')
    parser.add_argument('--colormap', default=None,
                        help='Name of the color map to use.')
    parser.add_argument('--signals', default=None,
                        help='Comma-separated list of signal process names (will be put on top).')
    parser.add_argument('--regroup_processes', default=False, action='store_true',
                        help='Regroup some processes for less crowded plots.')
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
        print('  - {}: {}'.format(arg,getattr(args,arg)))

    # find all datacards in the input directory
    datacards = [os.path.join(args.datacarddir,f) for f in os.listdir(args.datacarddir)
                 if( f.startswith('datacard_') and f.endswith('.txt') and not '_out_' in f)]
    print('Found following datacards: {}'.format(datacards))

    # loop over datacards
    cmds = []
    for datacard in datacards:
        print('Now running on datacard {}...'.format(datacard))
  
        # find workspace
        workspace = datacard.replace('.txt', '.root')
        if not os.path.exists(workspace):
            raise Exception('ERROR: workspace {} does not exist.'.format(workspace))

        # find fit result file
        dopostfit = False
        if args.fitresultfile is not None:
            dopostfit = True
            fitresultfile = args.fitresultfile
            if args.fitresultfile=='multidimfit':
                fitresultfile = os.path.join(args.datacarddir,
                  'multidimfit' + os.path.basename(datacard).replace('.txt', '_out_multidimfit_obs.root'))
                msg = 'INFO: using automatic fit result file {} for datacard {}'.format(fitresultfile, datacard)
                print(msg)
            elif args.fitresultfile=='fitdiagnostics': pass # to do
            if not os.path.exists(fitresultfile):
                raise Exception('ERROR: fit result file {} does not exist.'.format(fitresultfile))
        
        # extract year and region from datacard name
        year = None
        year = [el for el in os.path.basename(datacard).split('_') 
                 if( el.startswith('20') or el.startswith('run') )][0]
        year = year.replace('.txt', '')
        region = None
        region = os.path.basename(datacard).split(year, 1)[0].replace('datacard_', '').strip('_')
        print('Extracted region {} and year {} from datacard {}'.format(region, year, datacard))

        # make the base command
        basecmd = 'python postfitplots.py'
        basecmd += ' -w {}'.format(workspace)
        basecmd += ' -d {}'.format(datacard)
        basecmd += ' -o {}'.format(args.outputdir)
        if args.variables is not None: basecmd += ' -v {}'.format(args.variables)
        if year is not None: basecmd += ' -y {}'.format(year)
        if region is not None: basecmd += ' -r {}'.format(region)
        if args.colormap is not None: basecmd += ' --colormap {}'.format(args.colormap)
        if args.signals is not None: basecmd += ' --signals {}'.format(args.signals)
        if args.regroup_processes: basecmd += ' --regroup_processes'
        basecmd += ' --extracmstext Preliminary'
        basecmd += ' --unblind --dolog --doclean'
        
        # make the command for the prefit plot
        cmd = basecmd + ' --extrainfos Prefit'

        # make the command for the postfit plot
        cmd2 = basecmd + ' --fitresultfile {}'.format(fitresultfile)
        cmd2 += ' --extrainfos Postfit'

        cmds.append(cmd)
        if dopostfit: cmds.append(cmd2)

    # run commands (todo: job submission)
    for cmd in cmds: os.system(cmd)
