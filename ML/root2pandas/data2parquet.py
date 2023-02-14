import uproot
import sys
import os

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random



sampledirectory = "/user/dmarckx/ewkino/ttWAnalysis/eventselection/output_dummy_test/{}_sim/signalregion_dilepton_inclusive_tight_nominal/"
def makePickle(filename, filetype, nr, year):
    events = uproot.open(sampledirectory.format(year) + filename)
    print(events["blackJackAndHookers/blackJackAndHookersTree"].keys())
    df = events["blackJackAndHookers/blackJackAndHookersTree"].arrays(list(events["blackJackAndHookers/blackJackAndHookersTree"].keys())[0:], library="pd")
    if type(df) is not pd.DataFrame:
        print("warning: it's a tuple of DFs! your root file isn't flattened and can't be guaranteed to be merged correctly")
        for i in range(len(df)):
            print(df[i])

        for j in range(len(df)-1):
            df[0] = df[0].merge(df[i+1], left_index=True, right_index=True, how="outer")
            df[0] = df[0].merge(df[j+1], left_index=True, right_index=True, how='outer', suffixes=('_y', ''))
            np.random.seed(13)  # Uncomment to make results reproducible.
            print(df[0])

            df[0].drop(df[0].filter(regex='_y$').columns, axis=1, inplace=True)
            print(df[0])
        df[0].to_pickle('/user/dmarckx/ewkino/ML/ML_dataframes/{}/GNN_withBDTvars/'.format(year) + filetype + '_' + str(nr) + '.bz2')

    else:
        df.to_pickle('/user/dmarckx/ewkino/ML/ML_dataframes/{}/GNN_withBDTvars/'.format(year) + filetype + '_' + str(nr) + '.bz2')


print(sys.argv[1] + sys.argv[2] + sys.argv[3], sys.argv[4])
makePickle(sys.argv[1],sys.argv[2],sys.argv[3], sys.argv[4])
