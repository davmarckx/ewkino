from Networks import *

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

print('sklearn version: ' + str(__version__))

# imports from pytorch
import torch
import json
import sys

fract = sys.argv[9]
status = "bilin"

def ROC_plot(sNN, X_train_norm,y_train,weight_train,X_test_norm, y_test,weight_test):
    with torch.no_grad():
        predictions = sNN(torch.from_numpy(np.array(X_test_norm)).float())
        y_pred = predictions.numpy().tolist()
    y_pred = [item[1] for item in y_pred]


    with torch.no_grad():#
        predictions = sNN(torch.from_numpy(np.array(X_train_norm)).float())
        y_predtrain = predictions.numpy().tolist()
    y_predtrain = [item[1] for item in y_predtrain]


    fpr, tpr, threshold = metrics.roc_curve(list(y_test), y_pred, sample_weight=weight_test)
    roc_auc = metrics.auc(fpr, tpr)
    fpr2, tpr2, threshold2 = metrics.roc_curve(list(y_train), y_predtrain, sample_weight=weight_train)
    roc_auc2 = metrics.auc(fpr2, tpr2)



    plt.title('weighted Receiver Operating Characteristic')
    plt.plot(fpr, tpr, 'b', label = 'test AUC = %0.2f' % roc_auc)
    plt.plot(fpr2, tpr2, 'g', label = 'train AUC = %0.2f' % roc_auc2)
    plt.legend(loc = 'lower right')
    plt.plot([0, 1], [0, 1],'r--')
    plt.xlim([0, 1])
    plt.ylim([0, 1])
    plt.ylabel('True Positive Rate')
    plt.xlabel('False Positive Rate')
    plt.savefig("/user/dmarckx/public_html/ML/NN/ROCs/" + status + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "ROCNN2" + ".png")


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

#load dataset######################################
alle1 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN2_2018_dilep_sNN.pkl').sample(frac=fract)
alle2 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN2_2017_dilep_sNN.pkl').sample(frac=fract)
alle3 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN2_2016PostVFP_dilep_sNN.pkl').sample(frac=fract)
alle4 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN2_2016PreVFP_dilep_sNN.pkl').sample(frac=fract)

alle1["year"] = "2018"
alle2["year"] = "2017"
alle3["year"] = "2016Post"
alle4["year"] = "2016Pre"

alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)

# Get one hot encoding of columns B
one_hot = pd.get_dummies(alle1['year'])
# Join the encoded df
alle1 = alle1.join(one_hot).drop('year',axis = 1)

alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventNN',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','_chargeflipweight','_fakeRateFlavour', '_bestZMass'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

y = X['class'] 


###################################################


#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20, random_state=13)


sums = X_train.groupby('class')["_weight"].sum()
class_imbalance_SF = sums[0]/sums[1]


X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1


weight_train = X_train['_weight']#contains gen reweighting
weight_test = X_test['_weight']#contains gen reweighting
classweight = np.array([1, class_imbalance_SF])


X_train = X_train.drop(['_weight', 'class','_weight_balanced'], axis = 1)
X_test = X_test.drop(['_weight', 'class'], axis = 1)


#for when decor would be needed
#y_reg_train = X_train['NJets']
#y_reg_test = X_test['NJets']


#normalize dataset based on training set info
X_mean = X_train.mean()
X_std = X_train.std()
X_train_norm = (X_train - X_mean)/X_std
X_test_norm = (X_test - X_mean)/X_std
X_norm = (X - X_mean)/X_std
for i in X_test.keys():
    if(X_std[i] < 0.00001):
        X_train_norm.loc[:, i] = 0
        X_test_norm.loc[:, i] = 0
        X_norm.loc[:, i] = 0

print('start training')

dropval = sys.argv[1]
regdropval = sys.argv[2]
learr = sys.argv[3]
beta1 = sys.argv[4]
beta2 = sys.argv[5]
batchsize = sys.argv[6]
epochs = sys.argv[7]
Njobs = sys.argv[8]

#volgorde: TrainNN2(X_train, y_train, X_test, y_test, weights_train, weights_test, classweight, dropval, learr, beta1, beta2, batchsize, epochs=15):
sNN = TrainNN(X_train_norm, y_train, X_test_norm, y_test,weight_train,weight_test, classweight, dropval,learr,beta1, beta2, batchsize,status, epochs=epochs, manualNjobs = Njobs)
#sNN = SmallNetwork2(dropval)
#sNN.load_state_dict(torch.load('/user/dmarckx/ewkino/ML/models/trained_sNN+_drop20_50_5e-05_(0,1)100.sav'))

torch.save(sNN.state_dict(), '/user/dmarckx/ewkino/ML/models/trained_bNN10p_' + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize)  + '.sav')
ROC_plot(sNN, X_train_norm,y_train,weight_train,X_test_norm, y_test,weight_test)
