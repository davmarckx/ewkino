import torch
from torch_geometric.data import Data
import torch_geometric
import networkx as nx

import sys
#math and data packages
import pandas as pd
import numpy as np
import random
import math
import pickle

#sklearn imports
import sklearn
from sklearn.model_selection import train_test_split, KFold

import json
fract = 0.2

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

print("load datasets")
#load dataset######################################
alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2018_dilep_sNN.pkl').sample(frac = fract)
alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2017_dilep_sNN.pkl').sample(frac = fract)
alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2016PostVFP_dilep_sNN.pkl').sample(frac = fract)
alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2016PreVFP_dilep_sNN.pkl').sample(frac = fract)

other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_2018_dilep_sNN.pkl').sample(frac = fract)
other2 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_2017_dilep_sNN.pkl').sample(frac = fract)
other3 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_2016PostVFP_dilep_sNN.pkl').sample(frac = fract)
other4 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_2016PreVFP_dilep_sNN.pkl').sample(frac = fract)

alle1["datayear"] = 4
alle2["datayear"] = 3
alle3["datayear"] = 2
alle4["datayear"] = 1

other1["datayear"] = 4
other2["datayear"] = 3
other3["datayear"] = 2
other4["datayear"] = 1

alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)
alle1["buffer"] = 0
other1 = pd.concat([other1, other2,other3,other4], ignore_index=True)
other1["buffer"] = 0
print(alle1.shape[0])
print(other1.shape[0])



alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})


other1 = other1[other1["_weight"]>0]
other1 = other1.replace({"class": dictio})
other1 = other1[other1["class"]!='TTW'] #remove signal samples to only inject the test signal samples
print(other1["class"])

X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventNN',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','_chargeflipweight','_fakeRateFlavour', '_bestZMass'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

X_other =  other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventNN',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield','_chargeflipweight','_fakeRateFlavour', '_bestZMass'], axis=1)
X_other.loc[X_other['class'] != 1, 'class'] = 0

weight_other = X_other['_weight']#contains gen reweighting


y = X['class']



#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.1)


sums = X_train.groupby('class')["_weight"].sum()
class_imbalance_SF = sums[0]/sums[1]


X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1


weight_train = X_train['_weight'] #contains gen reweighting
weight_test = X_test['_weight'] #contains gen reweighting
classweight = np.array([1, class_imbalance_SF])

# add the test signal samples back to the other backgrounds dataframe
X_other = pd.concat([X_other, X_test[X_test["class"]==1]],ignore_index=True)
y_other = X_other["class"]
print(X_other["class"])

#for when decor would be needed
#y_reg_train = X_train['NJets']
#y_reg_test = X_test['NJets']


#normalize dataset based on training set info
X_mean = X_train.mean()
X_std = X_train.std()
X_train_norm = (X_train - X_mean)/X_std
X_test_norm = (X_test - X_mean)/X_std
X_other_norm = (X_other - X_mean)/X_std
X_norm = (X - X_mean)/X_std
for i in X_test.keys():
    if(X_std[i] < 0.00001):
        X_train_norm.loc[:, i] = 0
        X_test_norm.loc[:, i] = 0
        X_other_norm.loc[:, i] = 0
        X_norm.loc[:, i] = 0


################################################################make graph networki structure################################################
#make feature selections
jet1F = ['_jetPt1','_jetEta1','_jetE1','_jetMass1','_deepCSV_1','_deepCSVc_1','_deepCSVudsg_1','_deepFlavor_1']
jet2F = ['_jetPt2','_jetEta2','_jetE2','_jetMass2','_deepCSV_2','_deepCSVc_2','_deepCSVudsg_2','_deepFlavor_2']
jet3F = ['_jetPt3','_jetEta3','_jetE3','_jetMass3','_deepCSV_3','_deepCSVc_3','_deepCSVudsg_3','_deepFlavor_3']
jet4F = ['_jetPt4','_jetEta4','_jetE4','_jetMass4','_deepCSV_4','_deepCSVc_4','_deepCSVudsg_4','_deepFlavor_4']
jet5F = ['_jetPt5','_jetEta5','_jetE5','_jetMass5','_deepCSV_5','_deepCSVc_5','_deepCSVudsg_5','_deepFlavor_5']
lep1F = ['_leptonPt1','_leptonEta1','_leptonE1', '_leptonCharge1','_leptonFlavor1','_l1_3dIP', '_l1sip3d','_leptonMVATOP_l1']
lep2F = ['_leptonPt2','_leptonEta2','_leptonE2', '_leptonCharge2','_leptonFlavor2','_l2_3dIP', '_l2sip3d','_leptonMVATOP_l2']
lep3F = ['_leptonPt3','_leptonEta3','_leptonE3', '_leptonCharge3','_leptonFlavor3','_l3_3dIP', '_l3sip3d','_leptonMVATOP_l3']
metF = ['_MET_pt','_nJets',"_nBJets",'_lT','datayear','_HT',"_nMuons","_nElectrons"]
#nrsF = ['_nJets',"_nBJets","_nMuons","_nElectrons",'_leptonMVAttH_min','_leptonMVATOP_min','_deepCSV_max','_deepFlavor_max']
#edgeF = ['_jetdR12','_jetdR13','_jetdR14','_jetdR15' ]




# make edges
edge_index = torch.tensor([[0, 1],[0,2],[0,3],[0,4],[0,5],[0,6],[0,7],[0,8],
                           [1, 0],[2,0],[3,0],[4,0],[5,0],[6,0],[7,0],[8,0],
                           [1, 2],[1,3],[1,4],[1,5],[1,6],[1,7],[1,8],
                           [2, 1],[3,1],[4,1],[5,1],[6,1],[7,1],[8,1],
                           [2,3],[2,4],[2,5],[2,6],[2,7],[2,8],
                           [3,2],[4,2],[5,2],[6,2],[7,2],[8,2],
                           [3,4],[3,5],[3,6],[3,7],[3,8],
                           [4,3],[5,3],[6,3],[7,3],[8,3],
                           [4,5],[4,6],[4,7],[4,8],
                           [5,4],[6,4],[7,4],[8,4],
                           [5,6],[5,7],[5,8],
                           [6,5],[7,5],[8,5],
                           [6,7],[6,8],
                           [7,6],[8,6],
                           [7,8],
                           [8,7]], dtype=torch.long)

# fill the graphs in the dataframe
traindata = []
print("start train graph construction:")
for i in range(len(X_train)):
    x = torch.tensor([ list(X_train_norm[jet1F].iloc[i]),
                       list(X_train_norm[jet2F].iloc[i]),
                       list(X_train_norm[jet3F].iloc[i]),
                       list(X_train_norm[jet4F].iloc[i]),
                       list(X_train_norm[jet5F].iloc[i]),
                       list(X_train_norm[lep1F].iloc[i]),
                       list(X_train_norm[lep2F].iloc[i]),
                       list(X_train_norm[lep3F].iloc[i]),
                       list(X_train_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
#    data.xGL = torch.tensor(list(X_train[nrsF].iloc[i]), dtype=torch.int)
    data.y = torch.tensor([X_train["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_train["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_train[["_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1',"_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1'
    ,'_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2','_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2'
    ,'_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3','_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3'
    ,'_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4','_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4'
    ,'_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5','_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5'
    ,'_leptondR12','_leptondR13','_lepmetdP1','_leptondR12','_leptondR13','_lepmetdP1'
    ,'_leptondR23','_lepmetdP2','_leptondR23','_lepmetdP2'
    ,'_lepmetdP3','_lepmetdP3']].iloc[i]], dtype=torch.float)
    data.validate(raise_on_error=True)
    traindata.append(data)
    if i % 100000 == 0:
        print(i)

testdata = []
print("start test graph construction:")
for i in range(len(X_test)):
    x = torch.tensor([ list(X_test_norm[jet1F].iloc[i]),
                       list(X_test_norm[jet2F].iloc[i]),
                       list(X_test_norm[jet3F].iloc[i]),
                       list(X_test_norm[jet4F].iloc[i]),
                       list(X_test_norm[jet5F].iloc[i]),
                       list(X_test_norm[lep1F].iloc[i]),
                       list(X_test_norm[lep2F].iloc[i]),
                       list(X_test_norm[lep3F].iloc[i]),
                       list(X_test_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
#    data.xGL = torch.tensor(list(X_test[nrsF].iloc[i]), dtype=torch.int)
    data.y = torch.tensor([X_test["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_test["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_test[["_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1',"_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1'
    ,'_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2','_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2'
    ,'_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3','_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3'
    ,'_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4','_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4'
    ,'_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5','_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5'
    ,'_leptondR12','_leptondR13','_lepmetdP1','_leptondR12','_leptondR13','_lepmetdP1'
    ,'_leptondR23','_lepmetdP2','_leptondR23','_lepmetdP2'
    ,'_lepmetdP3','_lepmetdP3']].iloc[i]], dtype=torch.float)
    data.validate(raise_on_error=True)
    testdata.append(data)
    if i % 100000 == 0:
        print(i)


otherdata = []
print("start other graph construction:")
for i in range(len(X_other)):
    x = torch.tensor([ list(X_other_norm[jet1F].iloc[i]),
                       list(X_other_norm[jet2F].iloc[i]),
                       list(X_other_norm[jet3F].iloc[i]),
                       list(X_other_norm[jet4F].iloc[i]),
                       list(X_other_norm[jet5F].iloc[i]),
                       list(X_other_norm[lep1F].iloc[i]),
                       list(X_other_norm[lep2F].iloc[i]),
                       list(X_other_norm[lep3F].iloc[i]),
                       list(X_other_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
#    data.xGL = torch.tensor(list(X_other[nrsF].iloc[i]), dtype=torch.int)
    data.y = torch.tensor([X_other["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_other["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_other[["_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1',"_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jet1l3dR','_jetmetdP1'
    ,'_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2','_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jet2l3dR','_jetmetdP2'
    ,'_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3','_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jet3l3dR','_jetmetdP3'
    ,'_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4','_jetdR45','_jet4l1dR','_jet4l2dR','_jet4l3dR','_jetmetdP4'
    ,'_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5','_jet5l1dR','_jet5l2dR','_jet5l3dR','_jetmetdP5'
    ,'_leptondR12','_leptondR13','_lepmetdP1','_leptondR12','_leptondR13','_lepmetdP1'
    ,'_leptondR23','_lepmetdP2','_leptondR23','_lepmetdP2'
    ,'_lepmetdP3','_lepmetdP3']].iloc[i]], dtype=torch.float)
    data.validate(raise_on_error=True)
    otherdata.append(data)
    if i % 100000 == 0:
        print(i)


print("start plotting graph")
g = torch_geometric.utils.to_networkx(data, to_undirected=True)

nx.draw_networkx(g, node_color = ['#1f78b4','#1f78b4','#1f78b4','#1f78b4','#1f78b4','#b41f3f','#b41f3f','#b41f3f','#116105'], edge_color= (X_other_norm["_jetdR12"][1], X_other_norm["_jetdR13"][1], X_other_norm["_jetdR14"][1],
                                               X_other_norm["_jetdR15"][1], X_other_norm["_jet1l1dR"][1], X_other_norm["_jet1l2dR"][1], X_other_norm["_jet1l3dR"][1], X_other_norm["_jetmetdP1"][1], X_other_norm["_jetdR23"][1],
                                               X_other_norm["_jetdR24"][1], X_other_norm["_jetdR25"][1], X_other_norm["_jet2l1dR"][1], X_other_norm["_jet2l2dR"][1], X_other_norm["_jet2l3dR"][1], X_other_norm["_jetmetdP2"][1],
                                               X_other_norm["_jetdR34"][1], X_other_norm["_jetdR35"][1], X_other_norm["_jet3l1dR"][1], X_other_norm["_jet3l2dR"][1], X_other_norm["_jet3l3dR"][1], X_other_norm["_jetmetdP3"][1],
                                               X_other_norm["_jetdR45"][1], X_other_norm["_jet4l1dR"][1], X_other_norm["_jet4l2dR"][1], X_other_norm["_jet4l3dR"][1], X_other_norm["_jetmetdP4"][1], X_other_norm["_jet5l1dR"][1],                                                     X_other_norm["_jet5l2dR"][1], X_other_norm["_jet5l3dR"][1], X_other_norm["_jetmetdP5"][1], X_other_norm["_leptondR12"][1], X_other_norm["_leptondR13"][1],X_other_norm["_lepmetdP1"][1],
                                               X_other_norm["_leptondR23"][1], X_other_norm["_lepmetdP2"][1],X_other_norm["_lepmetdP3"][1] ),edge_vmin=0,edge_vmax=6,
                                               labels={0:"jet 1",1:"jet 2",2:"jet 3",3:"jet 4",4:"jet 5",5:"l1",6:"l2",7:"l3",8:"MET"}, edge_cmap = plt.cm.get_cmap('Blues_r'))
plt.savefig("/user/dmarckx/public_html/ML/NN/graph_examples/graphtest_fancy2.png")
plt.close()




########################### save them
file_name = "../ML_dataframes/trainsets/graphtrainset_smallGNN_dilep_sNN.pkl"
# save boostfeaturemap to keep up to date
pickle.dump(traindata, open(file_name, "wb"))

file_name = "../ML_dataframes/trainsets/graphtestset_smallGNN_dilep_sNN.pkl"
# save boostfeaturemap to keep up to date
pickle.dump(testdata, open(file_name, "wb"))

file_name = "../ML_dataframes/trainsets/graphotherset_smallGNN_dilep_sNN.pkl"
# save boostfeaturemap to keep up to date
pickle.dump(otherdata, open(file_name, "wb"))
