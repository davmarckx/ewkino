import torch
from torch_geometric.data import Data
import torch_geometric
import networkx as nx

from Networks import *

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

from sklearn import metrics
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score
from sklearn import __version__

# imports from pytorch
import torch
import json



status = "GATsmalltrainselobj_allyearsfinal"
fract = float(sys.argv[8])
year = sys.argv[9]
Njobs = int(sys.argv[10])
sparse = sys.argv[11]


def ROC_plot(GNN,traindata,testdata,otherdata):
    trainloader = DataLoader(traindata, batch_size=1000, shuffle=False)
    testloader = DataLoader(testdata, batch_size=1000, shuffle=False)
    otherloader = DataLoader(otherdata, batch_size=1000, shuffle=False)
    with torch.no_grad():
        y_pred = []
        y_true = []
        testweight = []
        for testdat in testloader:
            predictions = GNN(testdat.x, testdat.edge_index, testdat.batch)
            y_pred.extend(predictions.numpy().tolist())
            testweight.extend(testdat.w.numpy().tolist())
            y_true.extend(testdat.y.numpy().tolist())
    
    y_pred = [item[1] for item in y_pred]
    

    with torch.no_grad():#
        y_predtrain = []
        y_truetrain = []
        trainweight = []
        for traindat in trainloader:
            predictions = GNN(traindat.x, traindat.edge_index, traindat.batch)
            y_predtrain.extend(predictions.numpy().tolist())
            trainweight.extend(traindat.w.numpy().tolist())
            y_truetrain.extend(traindat.y.numpy().tolist())
    y_predtrain = [item[1] for item in y_predtrain]
    

    with torch.no_grad():#
        y_predother = []
        y_trueother = []
        otherweight = []
        for otherdat in otherloader:
            predictions = GNN(otherdat.x, otherdat.edge_index, otherdat.batch)
            y_predother.extend(predictions.numpy().tolist())
            otherweight.extend(otherdat.w.numpy().tolist())
            y_trueother.extend(otherdat.y.numpy().tolist())
    y_predother = [item[1] for item in y_predother]



    fpr, tpr, threshold = metrics.roc_curve(y_true, y_pred, sample_weight=testweight)
    roc_auc = metrics.auc(fpr, tpr)
    fpr2, tpr2, threshold2 = metrics.roc_curve(y_truetrain, y_predtrain, sample_weight=trainweight)
    roc_auc2 = metrics.auc(fpr2, tpr2)
    fpr3, tpr3, threshold3 = metrics.roc_curve(y_trueother, y_predother, sample_weight=otherweight)
    roc_auc2 = metrics.auc(fpr3, tpr3)


    plt.title('weighted Receiver Operating Characteristic')
    plt.plot(fpr, tpr, 'b', label = 'test AUC = %0.2f' % roc_auc)
    plt.plot(fpr2, tpr2, 'k',ls='dotted', label = 'train AUC = %0.2f' % roc_auc2)
    plt.plot(fpr3, tpr3, 'g', label = 'other background AUC = %0.2f (vs test signal samples)' % roc_auc2)
    plt.legend(loc = 'lower right')
    plt.plot([0, 1], [0, 1],'r--')
    plt.xlim([0, 1])
    plt.ylim([0, 1])
    plt.ylabel('True Positive Rate')
    plt.xlabel('False Positive Rate')
    if sparse == 'True':
        plt.savefig("/user/dmarckx/public_html/ML/NN/ROCs/sparseconnectionsfinalallyears" + status + str(fract) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "ROCNN2" + ".png")
    if sparse == 'False':
        plt.savefig("/user/dmarckx/public_html/ML/NN/ROCs/fullconnectionsfinalallyears" + status + str(fract) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "ROCNN2" + ".png")

torch.set_num_threads(Njobs)


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

print("load datasets")
#load dataset######################################
alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_selobj_2018_dilep_GNN_selobj.pkl').sample(frac = fract)
alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_selobj_2017_dilep_GNN_selobj.pkl').sample(frac = fract)
alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_selobj_2016PostVFP_dilep_GNN_selobj.pkl').sample(frac = fract)
alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_selobj_2016PreVFP_dilep_GNN_selobj.pkl').sample(frac = fract)

other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_selobj_2018_dilep_GNN_selobj.pkl')
other2 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_selobj_2017_dilep_GNN_selobj.pkl')
other3 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_selobj_2016PostVFP_dilep_GNN_selobj.pkl')
other4 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_selobj_2016PreVFP_dilep_GNN_selobj.pkl')


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

# Get one hot encoding of columns B
#one_hot = pd.get_dummies(alle1['year'])
# Join the encoded df
#alle1 = alle1.join(one_hot).drop('year',axis = 1)

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
#X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
X_other.loc[X_other['class'] != 1, 'class'] = 0

weight_other = X_other['_weight']#contains gen reweighting


y = X['class']
#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.9)


sums = X_train.groupby('class')["_weight"].sum()
class_imbalance_SF = sums[0]/sums[1]


X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1


weight_train = X_train['_weight'] #contains gen reweighting
weight_test = X_test['_weight'] #contains gen reweighting
#weight_train_balanced = X_train['_weight_balanced']#contains class imbalance reweighting, which seems to be more usefull as a np.array in pytorch
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
jet1F = ['_jetPt1','_jetEta1','_jetMass1','_deepCSV_1','_deepCSVc_1','_deepCSVudsg_1','_deepFlavor_1']
jet2F = ['_jetPt2','_jetEta2','_jetMass2','_deepCSV_2','_deepCSVc_2','_deepCSVudsg_2','_deepFlavor_2']
jet3F = ['_jetPt3','_jetEta3','_jetMass3','_deepCSV_3','_deepCSVc_3','_deepCSVudsg_3','_deepFlavor_3']
jet4F = ['_jetPt4','_jetEta4','_jetMass4','_deepCSV_4','_deepCSVc_4','_deepCSVudsg_4','_deepFlavor_4']
jet5F = ['_jetPt5','_jetEta5','_jetMass5','_deepCSV_5','_deepCSVc_5','_deepCSVudsg_5','_deepFlavor_5']
lep1F = ['_leptonPt1','_leptonEta1','_leptonE1', '_leptonCharge1','_leptonFlavor1','_l1_3dIP','_leptonMVATOP_l1']
lep2F = ['_leptonPt2','_leptonEta2','_leptonE2', '_leptonCharge2','_leptonFlavor2','_l2_3dIP','_leptonMVATOP_l2']
metF = ['_MET_pt','_lT','_HT','_nJets',"_nBJets","_nMuons","datayear"]

if sparse == 'True':
    edges = ["_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR',"_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR'
        ,'_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR'
        ,'_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR'
    	,'_jetdR45','_jet4l1dR','_jet4l2dR','_jetdR45','_jet4l1dR','_jet4l2dR'
    	,'_jet5l1dR','_jet5l2dR','_jet5l1dR','_jet5l2dR'
   	,'_leptondR12','_lepmetdP1','_leptondR12','_lepmetdP1'
   	,'_lepmetdP2','_lepmetdP2']
if sparse == 'False':
    edges = ["_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jetmetdP1',"_jetdR12",'_jetdR13','_jetdR14','_jetdR15','_jet1l1dR','_jet1l2dR','_jetmetdP1'
        ,'_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jetmetdP2','_jetdR23','_jetdR24','_jetdR25','_jet2l1dR','_jet2l2dR','_jetmetdP2'
        ,'_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jetmetdP3','_jetdR34','_jetdR35','_jet3l1dR','_jet3l2dR','_jetmetdP3'
        ,'_jetdR45','_jet4l1dR','_jet4l2dR','_jetmetdP4','_jetdR45','_jet4l1dR','_jet4l2dR','_jetmetdP4'
        ,'_jet5l1dR','_jet5l2dR','_jetmetdP5','_jet5l1dR','_jet5l2dR','_jetmetdP5'
        ,'_leptondR12','_lepmetdP1','_leptondR12','_lepmetdP1'
        ,'_lepmetdP2','_lepmetdP2']





# make edges
if sparse == 'True':
    edge_index = torch.tensor([[0, 1],[0,2],[0,3],[0,4],[0,5],[0,6],#[0,7],
                           [1, 0],[2,0],[3,0],[4,0],[5,0],[6,0],#[7,0],
                           [1, 2],[1,3],[1,4],[1,5],[1,6],#[1,7],
                           [2, 1],[3,1],[4,1],[5,1],[6,1],#[7,1],
                           [2,3],[2,4],[2,5],[2,6],#[2,7],
                           [3,2],[4,2],[5,2],[6,2],#[7,2],
                           [3,4],[3,5],[3,6],#[3,7],
                           [4,3],[5,3],[6,3],#[7,3],
                           [4,5],[4,6],#[4,7],
                           [5,4],[6,4],#[7,4],
                           [5,6],[5,7],
                           [6,5],[7,5],
                           [6,7],
                           [7,6],
                           ], dtype=torch.long)
if sparse == 'False':
    edge_index = torch.tensor([[0, 1],[0,2],[0,3],[0,4],[0,5],[0,6],[0,7],
                           [1, 0],[2,0],[3,0],[4,0],[5,0],[6,0],[7,0],
                           [1, 2],[1,3],[1,4],[1,5],[1,6],[1,7],
                           [2, 1],[3,1],[4,1],[5,1],[6,1],[7,1],
                           [2,3],[2,4],[2,5],[2,6],[2,7],
                           [3,2],[4,2],[5,2],[6,2],[7,2],
                           [3,4],[3,5],[3,6],[3,7],
                           [4,3],[5,3],[6,3],[7,3],
                           [4,5],[4,6],[4,7],
                           [5,4],[6,4],[7,4],
                           [5,6],[5,7],
                           [6,5],[7,5],
                           [6,7],
                           [7,6],
                           ], dtype=torch.long)



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
                       list(X_train_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
    data.y = torch.tensor([X_train["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_train["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_train[edges].iloc[i]], dtype=torch.float)
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
                       list(X_test_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
    data.y = torch.tensor([X_test["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_test["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_test[edges].iloc[i]], dtype=torch.float)
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
                       list(X_other_norm[metF].iloc[i])], dtype=torch.float)

    data = Data(x=x, edge_index=edge_index.t().contiguous(), edge_attr=None , y=None )
    data.y = torch.tensor([X_other["class"].iloc[i]], dtype=torch.int)
    data.w = torch.tensor([X_other["_weight"].iloc[i]], dtype=torch.float)
    data.edge_attr = torch.tensor([X_other[edges].iloc[i]], dtype=torch.float)
    data.validate(raise_on_error=True)
    otherdata.append(data)
    if i % 100000 == 0:
        print(i)



g = torch_geometric.utils.to_networkx(data, to_undirected=True)

if sparse == 'True':
    nx.draw_networkx(g, node_color = ['#1f78b4','#1f78b4','#1f78b4','#1f78b4','#1f78b4','#b41f3f','#b41f3f','#116105'], edge_color= (X_other_norm["_jetdR12"][-1], X_other_norm["_jetdR13"][-1], X_other_norm["_jetdR14"][-1],
                                               X_other_norm["_jetdR15"][-1], X_other_norm["_jet1l1dR"][-1], X_other_norm["_jet1l2dR"][-1], X_other_norm["_jet1l3dR"][-1], X_other_norm["_jetdR23"][-1],
                                               X_other_norm["_jetdR24"][-1], X_other_norm["_jetdR25"][-1], X_other_norm["_jet2l1dR"][-1], X_other_norm["_jet2l2dR"][-1], X_other_norm["_jet2l3dR"][-1],
                                               X_other_norm["_jetdR34"][-1], X_other_norm["_jetdR35"][-1], X_other_norm["_jet3l1dR"][-1], X_other_norm["_jet3l2dR"][-1], X_other_norm["_jet3l3dR"][-1],
                                               X_other_norm["_jetdR45"][-1], X_other_norm["_jet4l1dR"][-1], X_other_norm["_jet4l2dR"][-1], X_other_norm["_jet4l3dR"][-1], X_other_norm["_jet5l1dR"][-1],
                                               X_other_norm["_jet5l2dR"][-1], X_other_norm["_jet5l3dR"][-1], X_other_norm["_leptondR12"][-1], X_other_norm["_leptondR13"][-1],X_other_norm["_lepmetdP1"][-1],
                                               X_other_norm["_leptondR23"][-1], X_other_norm["_lepmetdP2"][-1],X_other_norm["_lepmetdP3"][-1] ),edge_vmin=0,edge_vmax=6,
                                               labels={0:"jet 1",1:"jet 2",2:"jet 3",3:"jet 4",4:"jet 5",5:"l1",6:"l2",7:"l3",8:"MET"}, edge_cmap = matplotlib.cm.get_cmap('Blues_r'))
if sparse == 'False':
    nx.draw_networkx(g, node_color = ['#1f78b4','#1f78b4','#1f78b4','#1f78b4','#1f78b4','#b41f3f','#b41f3f','#116105'], edge_color= (X_other_norm["_jetdR12"][-1], X_other_norm["_jetdR13"][-1], X_other_norm["_jetdR14"][-1],
                                               X_other_norm["_jetdR15"][-1], X_other_norm["_jet1l1dR"][-1], X_other_norm["_jet1l2dR"][-1], X_other_norm["_jet1l3dR"][-1], X_other_norm["_jetmetdP1"][-1], X_other_norm["_jetdR23"][-1],
                                               X_other_norm["_jetdR24"][-1], X_other_norm["_jetdR25"][-1], X_other_norm["_jet2l1dR"][-1], X_other_norm["_jet2l2dR"][-1], X_other_norm["_jet2l3dR"][-1], X_other_norm["_jetmetdP2"][-1],
                                               X_other_norm["_jetdR34"][-1], X_other_norm["_jetdR35"][-1], X_other_norm["_jet3l1dR"][-1], X_other_norm["_jet3l2dR"][-1], X_other_norm["_jet3l3dR"][-1], X_other_norm["_jetmetdP3"][-1],
                                               X_other_norm["_jetdR45"][-1], X_other_norm["_jet4l1dR"][-1], X_other_norm["_jet4l2dR"][-1], X_other_norm["_jet4l3dR"][-1], X_other_norm["_jetmetdP4"][-1], X_other_norm["_jet5l1dR"][-1],                                                     X_other_norm["_jet5l2dR"][-1], X_other_norm["_jet5l3dR"][-1], X_other_norm["_jetmetdP5"][-1], X_other_norm["_leptondR12"][-1], X_other_norm["_leptondR13"][-1],X_other_norm["_lepmetdP1"][-1],
                                               X_other_norm["_leptondR23"][-1], X_other_norm["_lepmetdP2"][-1],X_other_norm["_lepmetdP3"][-1] ),edge_vmin=0,edge_vmax=6,
                                               labels={0:"jet 1",1:"jet 2",2:"jet 3",3:"jet 4",4:"jet 5",5:"l1",6:"l2",7:"l3",8:"MET"}, edge_cmap = matplotlib.cm.get_cmap('Blues_r'))
if sparse == 'True':
    plt.savefig("/user/dmarckx/public_html/ML/NN/graph_examples/graphtest_selobj_sparseconnections.png")
if sparse == 'False':
    plt.savefig("/user/dmarckx/public_html/ML/NN/graph_examples/graphtest_selobj_fullconnect.png")
plt.close()




print("start training")
##############
dropval = float(sys.argv[1])
regdropval = float(sys.argv[2])
learr = float(sys.argv[3])
beta1 = float(sys.argv[4])
beta2 = float(sys.argv[5])
batchsize = int(sys.argv[6])
epochs = int(sys.argv[7])
nheads=4
self_loops = True

GCN = trainGCN(traindata,testdata,classweight, dropval,learr,beta1,beta2,batchsize,status,nheads=nheads,self_loops=self_loops, epochs=epochs, manualNjobs=4)
if sparse == 'True':
    torch.save(GCN.state_dict(), '/user/dmarckx/ewkino/ML/models/trained_' + str(fract) + 'GCNselobj_sparseconnectionsfinalallyears' + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize)  + '.sav')
if sparse == 'False':
    torch.save(GCN.state_dict(), '/user/dmarckx/ewkino/ML/models/trained_' + str(fract) + 'GCNselobj_fullyconnectedfinalallyears' + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize)  + '.sav')

ROC_plot(GCN,traindata,testdata,otherdata)
