import os
import sys

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random

years = ["2016PreVFP", "2016PostVFP","2017","2018"]
trainlist = ["TT", "TTG","TTZ","TTH","TTW"]

fract = 0.2


for year in years:
  trainset = 'trainset_robustBDT_{}_robust_BDT.pkl'.format(year)
  alle1 = pd.read_pickle('ML_dataframes/trainsets/' + trainset)
  selected1 = pd.read_pickle('ML_dataframes/trainsets/' + trainset).sample(frac=fract, random_state=13)

  f= open("trainindices/training_rescaling_weights/{}.txt".format(year),"w+")
  for cat in trainlist:
    events = len(alle1[alle1["class"]==cat])
    selectedevents = len(selected1[selected1["class"]==cat])

    print(events)
    print("\n")
    print(selectedevents)
    print("\nweight:\n")
    print(selectedevents/events)
    
    f.write("{} {}".format(cat, str(selectedevents/events)))
  f.close()
