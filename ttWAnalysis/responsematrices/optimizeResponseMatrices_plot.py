########################################################
# Plotter for the output of optimizeResponsMatrices.py #
########################################################

import sys
import os
import json
import argparse
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt


if __name__=='__main__':

    # parse arguments
    parser = argparse.ArgumentParser('Optimize response matrix binning')
    parser.add_argument('--inputfiles', required=True, type=os.path.abspath, nargs='+')
    parser.add_argument('--outputfile', required=True, type=os.path.abspath)
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
      print('  - {}: {}'.format(arg,getattr(args,arg)))

    # loop over input files to extract info
    histnames = []
    info = {}
    for inputfile in args.inputfiles:
	with open(inputfile,'r') as f:
	    thisinfo = json.load(f)
	histname = thisinfo['histname']
	histname = histname.split('__')[1]
	histnames.append(histname)
	info[histname] = thisinfo

    # summary printouts
    for histname in histnames:
        thisinfo = info[histname]
        infostr = histname + ':\n'
        fbinstr = '[' + ', '.join('{:.2f}'.format(el) for el in thisinfo['finalbins']) + ']'
        infostr += '  bins: {} -> {}'.format(
          thisinfo['initialbins'], fbinstr)
        #infostr += '\n  metric: {} -> {}'.format(
        #  thisinfo['initial']['metric'], thisinfo['final']['metric'])
        print(infostr)

    sys.exit()

    # initialize the figure
    (fig,axs) = plt.subplots(nrows=1, ncols=3, sharey=True)

    # first axis: stability
    ax = axs[0]
    for i,histname in enumerate(histnames):
	istability = info[histname]['initial']['stability']
	istability.append(istability[-1])
	fstability = info[histname]['final']['stability']
	fstability.append(fstability[-1])
	xax = np.linspace(0,1,num=len(istability),endpoint=True)
	ax.step(xax, np.array(istability)+i, color='r', where='post')
	ax.step(xax, np.array(fstability)+i, color='g', where='post')

    # draw separators
    ax.hlines(np.linspace(0,len(histnames),num=len(histnames)+1),0,1, linestyles='dashed')

    # set y-axis labels
    ax.set_yticks(np.linspace(0.5,len(histnames)-0.5,num=len(histnames)))
    ax.set_yticklabels(histnames)

    # format x-axis
    ax.set_xlabel('Stability')
    ax.set_xticks([])

    # second axis: purity
    ax = axs[1]
    for i,histname in enumerate(histnames):
        ipurity = info[histname]['initial']['purity']
	ipurity.append(ipurity[-1])
        fpurity = info[histname]['final']['purity']
	fpurity.append(fpurity[-1])
        xax = np.linspace(0,1,num=len(ipurity),endpoint=True)
        ax.step(xax, np.array(ipurity)+i, color='r', where='post')
        ax.step(xax, np.array(fpurity)+i, color='g', where='post')

    # draw separators
    ax.hlines(np.linspace(0,len(histnames),num=len(histnames)+1),0,1, linestyles='dashed')

    # format x-axis
    ax.set_xlabel('Purity')
    ax.set_xticks([])

    # third axis: yield ratio
    ax = axs[2]
    for i,histname in enumerate(histnames):
        idiagonal = info[histname]['initial']['diagonal']
        idiagonal.append(idiagonal[-1])
	imax = max(idiagonal)
        fdiagonal = info[histname]['final']['diagonal']
        fdiagonal.append(fdiagonal[-1])
	fmax = max(fdiagonal)
        xax = np.linspace(0,1,num=len(idiagonal),endpoint=True)
        ax.step(xax, np.array(idiagonal)/imax+i, color='r', where='post')
        ax.step(xax, np.array(fdiagonal)/fmax+i, color='g', where='post')

    # draw separators
    ax.hlines(np.linspace(0,len(histnames),num=len(histnames)+1),0,1, linestyles='dashed')

    # format x-axis
    ax.set_xlabel('Yield ratio')
    ax.set_xticks([])

    # add metric scores
    ax.text(1.1, -1.85, 'Total metric')
    for i, histname in enumerate(histnames):
	txt = '{:.2f}'.format(info[histname]['initial']['metric'])
	ax.text(1.1, i+0.5, txt, color='red', verticalalignment='center')
	txt = r'$\rightarrow$'
	ax.text(1.35, i+0.5, txt, verticalalignment='center')
	txt = '{:.2f}'.format(info[histname]['final']['metric'])
	ax.text(1.5, i+0.5, txt, color='green', verticalalignment='center')

    fig.savefig(args.outputfile, bbox_inches='tight')
