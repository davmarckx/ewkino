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

import json


status = "GATsmalltrainfinalallyears"
fract = float(sys.argv[8])
year = sys.argv[9]
njobs = int(sys.argv[10])


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
            #predictions = predictions.to(torch.long)
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
            #predictions = predictions.to(torch.long)
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
            #predictions = predictions.to(torch.long)
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
    plt.savefig("/user/dmarckx/public_html/ML/NN/ROCs/" + status + str(fract) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "ROCNN2" + ".png")


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

print("load datasets")
#load dataset######################################
traindata = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2018_dilep_sNN.pkl')
testdata = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallGNN_2017_dilep_sNN.pkl')
otherdata = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallGNN_2018_dilep_sNN.pkl')



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

GCN = trainGCN(traindata, testdata, classweight, dropval, learr, beta1, beta2, batchsize, status, nheads=nheads, self_loops=self_loops, epochs=epochs, manualNjobs=njobs)

torch.save(GCN.state_dict(), '/user/dmarckx/ewkino/ML/models/trained_' + status + str(fract) + 'GCN+_' + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize)  + '.sav')
ROC_plot(GCN,traindata,testdata,otherdata)
