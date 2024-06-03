######################################
# plot a goodness-of-fit test result #
######################################

import sys
import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import ROOT

def integral( binvalues, binedges, lowerbound=None, upperbound=None ):
    ### compute the integral of a histogram
    # check dimensions
    if len(binedges)!=len(binvalues)+1:
	raise Exception('ERROR in integral: binvalues and binedges are incompatible')
    # set bounds if not given
    if lowerbound is None: lowerbound = binedges[0]-1
    if upperbound is None: upperbound = binedges[-1]+1
    # loop over bins and calculate integral
    integral = 0
    for binnb in range(len(binvalues)):
	if binedges[binnb]<lowerbound: continue
	if binedges[binnb+1]>upperbound: continue
	binwidth = binedges[binnb+1]-binedges[binnb]
	integral += binvalues[binnb]*binwidth
    return integral

def makepvalplot( binvalues, binedges, tdata, pval=None,
                    title=None, 
		    yaxtitle='Probability density',
                    xaxtitle='Test statistic',
                    dolegend=True,
		    extrainfos=[],
                    fig=None, ax=None ):
    ### make a plot of the distribution of test statistics for toys and the test statistic in data
    if(fig is None or ax is None): fig,ax = plt.subplots()
    ymax = np.max(binvalues)
    binwidths = binedges[1:] - binedges[:-1]
    ax.bar( binedges[:-1], binvalues, width=binwidths, color='c', label='Toys' )
    ax.axvline( x=tdata, color='red', label='Data' )
    ax.set_ylim((0,ymax*1.2))
    if xaxtitle is not None: ax.set_xlabel(xaxtitle)
    if yaxtitle is not None: ax.set_ylabel(yaxtitle)
    if dolegend: ax.legend(loc='upper right')
    if title is not None: ax.set_title(title)
    if pval is not None: 
	ax.text(0.98,0.6,'p-value: {:.2f}'.format(pval),transform=ax.transAxes,ha='right')
    for i,info in enumerate(extrainfos):
	ax.text(0.98,0.6-(i+1)*0.05,info,transform=ax.transAxes,ha='right')
    return (fig,ax)


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Goodness of fit test')
    parser.add_argument('-w', '--workspace', required=True, type=os.path.abspath)
    # (note: workspace can be a single workspace (.root extension) or a directory.
    #  in the latter case, all goodness of fit output files in that directory will be used
    #  to create a summary plot)
    parser.add_argument('-i', '--includetags', default=[], nargs='+',
      help='Tags to select workspaces to include (in case --workspace is a directory)')
    parser.add_argument('-e', '--excludetags', default=[], nargs='+',
      help='Tags to deselect workspaces to include (in case --workspace is a directory)')
    parser.add_argument('-o', '--outputfile', default=None)
    parser.add_argument('--nbins', type=int, default=30)
    parser.add_argument('--extrainfos', default=None,
      help='Comma-separated list of extra info to display on plot')
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
          print('  - {}: {}'.format(arg,getattr(args,arg)))

    # argument checking
    if not os.path.exists(args.workspace):
        raise Exception('ERROR: workspace or directory {} does not exist.'.format(args.workspace))

    # check if input is a single workspace or a directory
    ismultiple = False
    if not args.workspace.endswith('.root'): ismultiple = True

    # find goodness of fit result files
    goffiles = {}
    if not ismultiple:
        # get directory and datacard from provided workspace
        card = args.workspace.replace('.root','.txt')
        (datacarddir, card) = os.path.split(card)
        name = card.replace('.txt','')
        # find data file
        datafile = os.path.join(datacarddir,
          'higgsCombine{}.GoodnessOfFit.mH120.root'.format(name))
        if not os.path.exists(datafile):
            raise Exception('ERROR: expected file {} does not exist.'.format(datafile))
        # find toy file
        toyfile = os.path.join(datacarddir,
            'higgsCombine{}.GoodnessOfFit.mH120.mergedtoys.root'.format(name))
        if not os.path.exists(toyfile):
            raise Exception('ERROR: expected file {} does not exist.'.format(toyfile))
        goffiles[name] = (datafile,toyfile)
    else:
        print('Finding goodness of fit files in directory {}...'.format(args.workspace))
        # find all goodness of fit files in directory
        datafiles = sorted([f for f in os.listdir(args.workspace)
                      if (f.startswith('higgsCombine') and f.endswith('GoodnessOfFit.mH120.root'))])
        toyfiles = sorted([f for f in os.listdir(args.workspace)
                      if (f.startswith('higgsCombine') and f.endswith('GoodnessOfFit.mH120.mergedtoys.root'))])
        if len(datafiles)!=len(toyfiles):
            print(datafiles)
            print(toyfiles)
            raise Exception('ERROR: numbers of data files and toy files do not agree.')
        print('Found {} files (before selection).'.format(len(datafiles)))
        names = [el.split('.')[0].replace('higgsCombine','') for el in datafiles]
        for name,df,tf in zip(names,datafiles,toyfiles):
            # do selection
            if len(args.excludetags)>0:
                select = True
                for tag in args.excludetags:
                    if tag in name: select = False
                if not select: continue
            if len(args.includetags)>0:
                select = False
                for tag in args.includetags:
                    if tag in name: select = True
                if not select: continue
            # add to file structure
            goffiles[name] = (os.path.join(args.workspace,df), os.path.join(args.workspace,tf))
        print('Selected {} files.'.format(len(goffiles)))

    # loop over goodness of fit files and collect results
    info = {}
    for name, (datafile,toyfile) in goffiles.items():
        print('Running on {}...'.format(name))
        # read output test statistic for data
        try:
            f = ROOT.TFile.Open(datafile)
            tree = f.Get('limit')
            tree.GetEntry(0)
            tdata = getattr(tree,'limit')
            print('Found data test statistic: {}'.format(tdata))
        except:
            print('Could not read data test statistic, skipping this instance.')
            continue
        # read output test statistics for toys
        try:
            f = ROOT.TFile.Open(toyfile)
            tree = f.Get('limit')
            ttoys = []
            for i in range(tree.GetEntries()):
	        tree.GetEntry(i)
	        ttoys.append(getattr(tree,'limit'))
            print('Found {} toy test statistics.'.format(len(ttoys)))
        except:
            print('Could not read toys test statistics, skipping this instance.')
            continue
        # calculate mean and variance
        ttoys = np.array(ttoys)
        ttoys_mean = np.mean(ttoys)
        ttoys_std = np.std(ttoys)
        # remove some numerical outliers
        # (this is done to improve the stability of the numerical integration,
        #  at the cost of a small bias which should be negligible if the threshold is high enough)
        ttoys = ttoys[np.nonzero(ttoys < ttoys_mean + 10*ttoys_std)]
        # recalculate mean and variance (just for printing)
        ttoys_mean = np.mean(ttoys)
        ttoys_std = np.std(ttoys)
        print('Mean: {}, std: {}'.format(ttoys_mean, ttoys_std))
        # compute p-value
        (hist,binedges) = np.histogram( ttoys, bins=args.nbins, density=True )
        pval = integral( hist, binedges, lowerbound=tdata )
        print('Calculated p value: {}'.format(pval))
        # add to info
        info[name] = {'tdata': tdata, 'ttoys': ttoys, 'pval': pval}
    
    # make a plot
    if args.outputfile is not None:
        # parse extra info
        extrainfos = []
        if args.extrainfos is not None:
            for el in args.extrainfos.split(','): extrainfos.append(el)
        # case of single plot
        if not ismultiple:
            name = info.keys()[0]
            (hist,binedges) = np.histogram( info[name]['ttoys'], bins=args.nbins, density=True )
            fig,ax = makepvalplot( hist, binedges, info[name]['tdata'], pval=info[name]['pval'],
		          extrainfos=extrainfos )
            fig.savefig(args.outputfile)
        # case of summary plot
        else:
            names = sorted(info.keys())
            # determine size of figure
            figsize = None
            if len(names)>8: figsize = (6.4, 4.8 + 0.4*(len(names)-8))
            fig,axs = plt.subplots(nrows=len(names), ncols=1, figsize=figsize)
            if len(names)>8: fig.subplots_adjust(bottom=0.11-0.003*(len(names)-8), top=0.88+0.005*(len(names)-8))
            # fill individual axes
            for i,name in enumerate(names):
                ax = axs[i]
                (hist,binedges) = np.histogram( info[name]['ttoys'], bins=args.nbins, density=True )
                fig,ax = makepvalplot( hist, binedges, info[name]['tdata'], pval=info[name]['pval'],
                                       xaxtitle=None, yaxtitle=None, dolegend=False,
                                       fig=fig, ax=ax )
                ax.set_xticks([])
                ax.set_yticks([])
                # modify name to display
                # (note: depends on naming convention, not generally applicable)
                displayname = name.replace('datacard_','').replace('dc_combined_','')
                displayname = displayname.replace('_', ' ')
                displayname = displayname.replace('signalregion','SR')
                displayname = displayname.replace('controlregion','CR')
                displayname = displayname.replace('dilepton','2L')
                displayname = displayname.replace('trilepton','3L')
                displayname = displayname.replace('fourlepton','4L')
                displayname = displayname.replace('inclusive', '')
                ax.text(0.98, 0.1, 'Fit: {}'.format(displayname), transform=ax.transAxes, ha='right')
            fig.text(0.5, 0.03, 'Test statistic', ha='center', fontsize=15)
            fig.text(0.03, 0.5, 'Probability', va='center', rotation='vertical', fontsize=15)
            axs[0].legend(loc='lower center', bbox_to_anchor=(0., 1., 1.,1.), ncol=2)
            fig.savefig(args.outputfile)
