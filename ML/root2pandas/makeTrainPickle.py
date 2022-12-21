#math and data packages
import pandas as pd
import numpy as np
import scipy
import random
from math import exp, log, floor
from random import random, randrange
from itertools import islice
from io import StringIO
import os
import sys

# Get the list of all files and directories
path = "../ML_dataframes/{}/{}/"#/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/{}/{}/"
year = sys.argv[1] 
region = sys.argv[2]

path = path.format(year, region)

dir_list = os.listdir(path)

alle = pd.DataFrame()

for i in range(len(dir_list)):
     np.random.seed(13)  # Uncomment to make results reproducible.
     if (dir_list[i].split("_")[0] == 'TTW'):
         df = pd.read_pickle(path + dir_list[i])
     else:
          df = pd.read_pickle(path + dir_list[i])

     df["class"] = dir_list[i].split("_")[0]
     alle = pd.concat([alle,df], ignore_index=True)

alle.to_pickle('../ML_dataframes/trainsets/trainset_NN2_' + year + '_' + region + '.pkl')

