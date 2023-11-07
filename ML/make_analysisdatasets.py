import os
import sys

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random



Name = sys.argv[1]
Type = sys.argv[2]
Index = sys.argv[3]
year = sys.argv[4]

samplelist = sys.argv[5]
inputdir = sys.argv[6]
storefolder = sys.argv[7]

fract = 0.2

trainset = 'trainset_sampleindices_robustBDT_{}_robust_BDT.pkl'.format(year)

completeset = pd.read_pickle('ML_dataframes/trainsets/' + trainset)
alle1 = pd.read_pickle('ML_dataframes/trainsets/' + trainset).sample(frac=fract, random_state=13)
regions = []
for r in ['signalregion_dilepton_inclusive']: regions.append(r)
for r in ['ee','em','me','mm']: regions.append('signalregion_dilepton_{}'.format(r))
for r in ['plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))


print(alle1["class"].unique())

#filter nominal
completeset = completeset[completeset["nominal"].isnull()]
alle1 = alle1[alle1["nominal"].isnull()]

#now filter index
completeset = completeset[completeset["index"]==Index]
alle1 = alle1[alle1["index"]==Index]
lumiblocks = alle1["_lumiBlock"].unique()


with open('trainindices/' + Type + '_{}_{}.txt'.format(Index,year), 'w') as f:
  for lumiblock in lumiblocks:
    this_eventnrs = list(alle1[alle1["_lumiBlock"]==lumiblock]["_eventNb"])
    f.write("\n")
    f.write(str(lumiblock))
    for nr in this_eventnrs:
        f.write(" " + str(nr))
   
#write the reweighting factor
with open('trainindices/weights_' + Type + '_{}_{}.txt'.format(Index,year), 'w') as f:
  for region in regions:
    print(region)
    print(Index)
    f.write(region + " ")
    if region == 'signalregion_dilepton_inclusive':
      if len(completeset) != len(alle1):
        f.write(str(len(completeset)/(len(completeset) - len(alle1))))
      else:
        f.write("1.0")
      print(str(len(alle1)))
      print(str(len(completeset)))
    elif region == 'signalregion_dilepton_plus':
      if len(completeset[completeset["_leptonChargeLeading"]>0]) != len(alle1[alle1["_leptonChargeLeading"]>0]):
        f.write(str(len(completeset[completeset["_leptonChargeLeading"]>0])/(len(completeset[completeset["_leptonChargeLeading"]>0]) - len(alle1[alle1["_leptonChargeLeading"]>0]))))
      else:
        f.write("1.0")
      print(str(len(alle1[alle1["_leptonChargeLeading"]>0])))
      print(str(len(completeset[completeset["_leptonChargeLeading"]>0])))
    elif region == 'signalregion_dilepton_minus':
      if len(completeset[completeset["_leptonChargeLeading"]<0]) != len(alle1[alle1["_leptonChargeLeading"]<0]):
        f.write(str(len(completeset[completeset["_leptonChargeLeading"]<0])/(len(completeset[completeset["_leptonChargeLeading"]<0]) - len(alle1[alle1["_leptonChargeLeading"]<0]))))
      else:
        f.write("1.0")
      print(str(len(alle1[alle1["_leptonChargeLeading"]<0])))
      print(str(len(completeset[completeset["_leptonChargeLeading"]<0])))
    elif region == 'signalregion_dilepton_ee':
      if len(completeset[completeset["_nMuons"]==0]) !=len(alle1[alle1["_nMuons"]==0]):
        f.write(str(len(completeset[completeset["_nMuons"]==0])/(len(completeset[completeset["_nMuons"]==0]) - len(alle1[alle1["_nMuons"]==0]))))
      else:
        f.write("1.0")
      print(str(len(alle1[alle1["_nMuons"]==0])))
      print(str(len(completeset[completeset["_nMuons"]>0])))
    elif region == 'signalregion_dilepton_mm':
      if len(completeset[completeset["_nMuons"]==2]) != len(alle1[alle1["_nMuons"]==2]):
        f.write(str(len(completeset[completeset["_nMuons"]==2])/(len(completeset[completeset["_nMuons"]==2]) - len(alle1[alle1["_nMuons"]==2]))))
      else:
        f.write("1.0")
      print(str(len(alle1[alle1["_nMuons"]==2])))
      print(str(len(completeset[completeset["_nMuons"]==2])))
    else:
      if len(completeset[completeset["_nMuons"]==1]) != len(alle1[alle1["_nMuons"]==1]):
        f.write(str(len(completeset[completeset["_nMuons"]==1])/(len(completeset[completeset["_nMuons"]==1]) - len(alle1[alle1["_nMuons"]==1]))))
      else:
        f.write("1.0")
      print(str(len(alle1[alle1["_nMuons"]==1])))
      print(str(len(completeset[completeset["_nMuons"]==1])))
    f.write("\n")

#now run command
cmd = "./makeAnalysisdatasets {} {} {} {} {} {} {}".format(Name, Type, Index, year, samplelist, inputdir, storefolder)
os.system(cmd)
