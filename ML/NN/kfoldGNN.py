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
from sklearn.model_selection import StratifiedKFold

from sklearn import metrics
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score
from sklearn import __version__

import json


nr_events = int(sys.argv[8]) #should e.g. be set to -1 for final training
year = sys.argv[9]
sparse = sys.argv[10]
njobs = int(sys.argv[11])
status = "GATsmalltrainfinaltests{}".format(year)


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


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

print("load datasets")
#load dataset######################################
if nr_events > 0:
    traindata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse)), nr_events)
    testdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse)), nr_events)
    otherdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse)), nr_events)
else:
    traindata = pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse))
    testdata = pd.read_pickle('../ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse))
    otherdata = pd.read_pickle('../ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse))
# add the train and test data to split in different cv's
traindata.extend(testdata)
# these are needed for startifiedkfold to split the dataset in a smart way
traintargets = [ int(x.y[0]) for x in traindata]


# remove the signal samples in otherdata, the cv test signal samples will be injected in each iteration to give an unbiased other background ROC performance
otherdata = [ x for x in otherdata if x.y[0] == 0]
print(traindata)



figu, axi = plt.subplots(figsize=(20,20))
tprs = []
aucs = []
tprsother = []
aucsother = []
mean_fpr = np.linspace(0, 1, 100)

cv = StratifiedKFold(n_splits=5)
for train_index, test_index in cv.split(traindata, traintargets):
    traindata2, testdata2 = [ x for x in traindata if traindata.index(x) in train_index], [ x for x in traindata if traindata.index(x) in test_index]
    # add test signals to the otherdata2 dataset to have a fair ROC performance idea
    otherdata2 = otherdata.extend([ x for x in testdata if x.y[0] == 1])
    sum_sig = 0.
    sum_back = 0.
    for i in range(len(traindata)):
        if traindata[i].y[0] == 1:
            sum_sig += traindata[i].w[0]
        elif traindata[i].y[0] == 0:
            sum_back += traindata[i].w[0]

    print("\nbackground:\n")
    print(sum_back)
    print("\nsignal:\n")
    print(sum_sig)
    classweight = np.array([1, sum_back / sum_sig])
    

    GCN = trainGCN(traindata2, testdata2, classweight, dropval, learr, beta1, beta2, batchsize, status, nheads=nheads, self_loops=self_loops, epochs=epochs, manualNjobs=njobs)

    trainloader = DataLoader(traindata2, batch_size=1000, shuffle=False)
    testloader = DataLoader(testdata2, batch_size=1000, shuffle=False)
    otherloader = DataLoader(otherdata, batch_size=1000, shuffle=False)

    with torch.no_grad():
        y_pred = []
        y_true = []
        testweight = []
        for testdat in testloader:
            predictions = GCN(testdat.x, testdat.edge_index, testdat.batch)
            #predictions = predictions.to(torch.long)
            y_pred.extend(predictions.numpy().tolist())
            testweight.extend(testdat.w.numpy().tolist())
            y_true.extend(testdat.y.numpy().tolist())


        y_predother = []
        y_trueother = []
        otherweight = []
        for otherdat in otherloader:
            otherpredictions = GCN(otherdat.x, otherdat.edge_index, otherdat.batch)
            #predictions = predictions.to(torch.long)
            y_predother.extend(otherpredictions.numpy().tolist())
            otherweight.extend(otherdat.w.numpy().tolist())
            y_trueother.extend(otherdat.y.numpy().tolist())

    y_score = [item[1] for item in y_pred]
    y_otherscore = [item[1] for item in y_predother]


    axi.text(0.05, 0.98, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=28,
        fontweight='bold',
        transform=axi.transAxes)
    axi.text(0.05, 0.92, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=20,
        transform=axi.transAxes)

    fpr, tpr, thresholds = roc_curve(y_true, y_score)
    axi.plot(fpr,tpr,lw=1,alpha=0.3, label = "ROC fold {}".format(str(i)))
    interp_tpr = np.interp(mean_fpr, fpr, tpr)
    interp_tpr[0] = 0.0
    tprs.append(interp_tpr)
    aucs.append(roc_auc_score(y_true, y_score))

    fprother, tprother, thresholds = roc_curve(y_trueother, y_otherscore)
    axi.plot(fprother,tprother,lw=1,alpha=0.3, label = "other backgrd ROC fold {}".format(str(i)))
    interp_tprother = np.interp(mean_fpr, fprother, tprother)
    interp_tprother[0] = 0.0
    tprsother.append(interp_tpr)
    aucsother.append(roc_auc_score(y_trueother, y_otherscore))

    i += 1
axi.plot([0, 1], [0, 1], linestyle="--", lw=1, color="r", label="Chance", alpha=0.8)

mean_tpr = np.mean(tprs, axis=0)
mean_tpr[-1] = 1.0
mean_auc = metrics.auc(mean_fpr, mean_tpr)
std_auc = np.std(aucs)
axi.plot(
    mean_fpr,
    mean_tpr,
    color="b",
    label=r"Mean ROC (AUC = %0.2f $\pm$ %0.4f)" % (mean_auc, std_auc),
    lw=2,
    alpha=0.8,
)

std_tpr = np.std(tprs, axis=0)
tprs_upper = np.minimum(mean_tpr + std_tpr, 1)
tprs_lower = np.maximum(mean_tpr - std_tpr, 0)
axi.fill_between(
    mean_fpr,
    tprs_lower,
    tprs_upper,
    color="grey",
    alpha=0.2,
    label=r"$\pm$ 1 std. dev.",
)

mean_tprother = np.mean(tprsother, axis=0)
mean_tprother[-1] = 1.0
mean_aucother = metrics.auc(mean_fpr, mean_tprother)
std_aucother = np.std(aucsother)
axi.plot(
    mean_fpr,
    mean_tprother,
    color="g",
    label=r"Mean other backgrd ROC (AUC = %0.2f $\pm$ %0.4f)" % (mean_aucother, std_aucother),
    lw=2,
    alpha=0.8,
)

std_tprother = np.std(tprs, axis=0)
tprs_upperother = np.minimum(mean_tprother + std_tprother, 1)
tprs_lowerother = np.maximum(mean_tprother - std_tprother, 0)
axi.fill_between(
    mean_fpr,
    tprs_lower,
    tprs_upper,
    color="grey",
    alpha=0.2,
    label=r"$\pm$ 1 std. dev.",
)


axi.set(
    xlim=[-0.05, 1.05],
    ylim=[-0.05, 1.05],
    title="Receiver operating characteristic curve")
axi.legend(loc="lower right",)
plt.setp(axi.get_legend().get_texts(), fontsize=25) # for legend text
plt.setp(axi.get_legend().get_title(), fontsize=25) #for legend title
plt.xticks(fontsize= 20)
plt.yticks(fontsize= 20)
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)


figu.savefig("/user/dmarckx/public_html/ML/NN/ROCs/kfoldROCNN_" + str(nr_events) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize) + ".png")
plt.close(figu)


