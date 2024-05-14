#########################
# Plot signal strengths #
#########################

# Note: use the plotting function in ttWAnalysis/differential
#       for full differential plots;
#       this script just serves to get a quick visual overview
#       of the signal strengths.


import os
import sys
import json
import argparse
import matplotlib as mpl
import matplotlib.pyplot as plt


def makeplot(info):
    # make the plot
    fig, ax = plt.subplots(figsize=(9,12))
    rmax = 3
    varnames = sorted(list(info.keys()))
    for varidx, varname in enumerate(varnames):
      (rs, statdownerrors, statuperrors, downerrors, uperrors) = info[varname]
      for ridx in range(len(rs)):
          r = rs[ridx]
          statdownerror = min(statdownerrors[ridx], r)
          statuperror = min(statuperrors[ridx], rmax-r)
          downerror = min(downerrors[ridx], r)
          uperror = min(uperrors[ridx], rmax-r)
          ax.plot([ridx, ridx], [rmax*varidx+r-downerror, rmax*varidx+r+uperror], color='blue', linewidth=2)
          ax.plot([ridx, ridx], [rmax*varidx+r-statdownerror, rmax*varidx+r+statuperror], color='red', linewidth=1)
          ax.scatter([ridx], [rmax*varidx+r], color='blue', s=20)
          ax.text(-0.6, rmax*varidx, varname, ha='right', va='bottom', fontsize=12)
    # plot aesthetics
    ax.grid()
    fig.subplots_adjust(left=0.45)
    fig.subplots_adjust(top=0.95)
    fig.subplots_adjust(bottom=0.07)
    ax.xaxis.set_ticks([i for i in range(0,5)])
    ax.xaxis.set_ticklabels([])
    ax.set_xlim((-0.5,5.5))
    ax.set_xlabel('Bin number', fontsize=12)
    ax.yaxis.set_ticks([])
    ax.yaxis.set_ticklabels([])
    ax.set_ylim((0., rmax*(len(varnames)+1)))
    ylims = ax.get_ylim()
    ax.hlines([rmax*varidx+1 for varidx in range(0,len(varnames)+1)], -0.5, 5.5, colors='gray', zorder=0, linestyle='dashed')
    ax.hlines([rmax*varidx for varidx in range(0,len(varnames)+1)], -0.5, 5.5, colors='gray')
    return (fig, ax)


if __name__=='__main__':

    # read command line args
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--signalstrengths', required=True)
    parser.add_argument('-o', '--outputfile', required=True)
    args = parser.parse_args()

    # print arguments
    print('Running with following configuration:')
    for arg in vars(args):
        print('  - {}: {}'.format(arg,getattr(args,arg)))

    # load signal strength json
    with open(args.signalstrengths,'r') as f:
        signalstrengths = json.load(f)

    # extract variables
    variablenames = signalstrengths.keys()
    
    # loop over variables
    info = {}
    for variablename in variablenames:
        # get pois
        thisss = signalstrengths[variablename]['pois']
        pois = sorted(list(thisss.keys()))
        # make arrays of central values and uncertainties
        rs = [thisss[poi][0] for poi in pois]
        statdownerrors = [thisss[poi][1] for poi in pois]
        statuperrors = [thisss[poi][2] for poi in pois]
        downerrors = [thisss[poi][3] for poi in pois]
        uperrors = [thisss[poi][4] for poi in pois]
        # add to info struct
        info[variablename] = (rs, statdownerrors, statuperrors, downerrors, uperrors)

    # make plot
    fig, ax = makeplot(info)
    fig.savefig(args.outputfile)
