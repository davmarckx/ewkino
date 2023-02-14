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
import threading

class myThread (threading.Thread):
   def __init__(self, threadID, name, filename):
      threading.Thread.__init__(self)
      self.threadID = threadID
      self.name = name
      self.filename = filename
      self.data = []
   def run(self):
      print("Starting " + self.name)
      df =  pd.read_pickle(self.filename)
      self.data = df
      print("Exiting " + self.name)



nr_events = int(sys.argv[8]) #should e.g. be set to -1 for final training
year = sys.argv[9]
sparse = sys.argv[10]
njobs = int(sys.argv[11])
status = "GATsmalltrainnewfeatures_withgraphnormalization_{}".format(year)


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
    roc_auc = metrics.roc_auc_score(y_true, y_pred, sample_weight = testweight)
    fpr2, tpr2, threshold2 = metrics.roc_curve(y_truetrain, y_predtrain, sample_weight=trainweight)
    roc_auc2 = metrics.roc_auc_score(y_truetrain, y_predtrain, sample_weight = trainweight)
    fpr3, tpr3, threshold3 = metrics.roc_curve(y_trueother, y_predother, sample_weight=otherweight)
    roc_auc3 = metrics.roc_auc_score(y_trueother, y_predother, sample_weight = otherweight)


    plt.title('weighted Receiver Operating Characteristic')
    plt.plot(fpr, tpr, 'b', label = 'test AUC = %0.2f' % roc_auc)
    plt.plot(fpr2, tpr2, 'k',ls='dotted', label = 'train AUC = %0.2f' % roc_auc2)
    plt.plot(fpr3, tpr3, 'g', label = 'other background AUC = %0.2f (vs test signal samples)' % roc_auc3)
    plt.legend(loc = 'lower right')
    plt.plot([0, 1], [0, 1],'r--')
    plt.xlim([0, 1])
    plt.ylim([0, 1])
    plt.ylabel('True Positive Rate')
    plt.xlabel('False Positive Rate')
    plt.savefig("/user/dmarckx/public_html/ML/NN/ROCs/" + status + sparse + str(nr_events) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "ROCNN2" + ".png")


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

print("load datasets")
#load dataset######################################
if year == "all":
    if nr_events > 0:
        traindata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2018_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2018_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2017_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2017_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

    else:
        print("train")
        thread1 = myThread(1, "Thread-1", '../ML_dataframes/trainsets/graphtrainset_2018_{}_smallGNN.pkl'.format(sparse))
        print("test")
        thread2 = myThread(2, "Thread-2", '../ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse))
        print("other")
        thread3 = myThread(3, "Thread-3", '../ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse))
        print("2018 done")
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata = thread1.data
        testdata = thread2.data
        otherdata = thread3.data
        
        thread1 = myThread(1, "Thread-1", '../ML_dataframes/trainsets/graphtrainset_2017_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", '../ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", '../ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data
        print("2017")

        thread1 = myThread(1, "Thread-1", '../ML_dataframes/trainsets/graphtrainset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", '../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", '../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data
        print("2016")

        thread1 = myThread(1, "Thread-1", '../ML_dataframes/trainsets/graphtrainset_2016PreVFP_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", '../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", '../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data

else:
    if nr_events > 0:
        traindata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse)),4*nr_events)
        testdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse)),nr_events)
        otherdata = random.sample(pd.read_pickle('../ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse)),4*nr_events)
    else:
        traindata = pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse))
        testdata = pd.read_pickle('../ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse))
        otherdata = pd.read_pickle('../ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse))

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

GCN = trainGCN(traindata, testdata, classweight, dropval, learr, beta1, beta2, batchsize, status, sparse=sparse, nheads=nheads, self_loops=self_loops, epochs=epochs, manualNjobs=njobs)

torch.save(GCN.state_dict(), '/user/dmarckx/ewkino/ML/models/trained_newfeatures_' + status + sparse + str(nr_events) + 'GCN+_' + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize)  + '.sav')
ROC_plot(GCN,traindata,testdata,otherdata)
