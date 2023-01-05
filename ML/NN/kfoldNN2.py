import torch
from Networks import *
from sklearn.model_selection import train_test_split, KFold
from sklearn.model_selection import StratifiedKFold
import sklearn
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score

import numpy as np

import pandas as pd
import json

fract = 0.0005

#plotting packages
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

#load dataset######################################
alle1 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN_2018_dilep.pkl').sample(frac=fract)
alle2 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN_2017_dilep.pkl').sample(frac=fract)
alle3 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN_2016PostVFP_dilep.pkl').sample(frac=fract)
alle4 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_NN_2016PreVFP_dilep.pkl').sample(frac=fract)

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

#background = alle1[alle1["class"] != 'TTW']
#signal = alle1[alle1["class"] == 'TTW']

X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventNN',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','_chargeflipweight','_fakeRateFlavour', '_bestZMass'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

y = alle1["class"]


#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20, random_state=13)
if (not y_train[y_train.isin(['TTW','TTX'])].empty):
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index)


sums = X_train.groupby('class')["_weight"].sum()
class_imbalance_SF = sums[0]/sums[1]


X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1


weight_train = X_train['_weight']#contains gen reweighting
weight_test = X_test['_weight']#contains gen reweighting
#weight_train_balanced = X_train['_weight_balanced']#contains class imbalance reweighting, which seems to be more usefull as a np.array in pytorch
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


dropval = 0.20
regdropval = 0.05
learr = 0.0005
beta1 = 0.1
beta2 = 0.1
batchsize = 50
epochs = 100


cv = StratifiedKFold(n_splits=5)

tprs = []
aucs = []
mean_fpr = np.linspace(0, 1, 100)

figu, axi = plt.subplots(figsize=(20,20))

i = 1

print(X_train_norm)
print(y_train)
for train_index, test_index in cv.split(X_train_norm, y_train):
    X_train_norm2, X_test_norm2 = X_train_norm.iloc[train_index], X_train_norm.iloc[test_index]
    weight_train2,weight_test2 = weight_train.iloc[train_index], weight_train.iloc[test_index]
    y_train2, y_test2 = y_train.iloc[train_index], y_train.iloc[test_index]

    sNN = TrainNN2(X_train_norm2, y_train2, X_test_norm2, y_test2,weight_train2,weight_test2, classweight, dropval,learr,beta1, beta2, batchsize,"kfold/kfoldNN2_" + str(i), epochs=epochs, manualNjobs=8)

    with torch.no_grad():
        predictions = sNN(torch.from_numpy(np.array(X_test_norm2)).float())
        #predictions = predictions.to(torch.long)
        y_pred = predictions.numpy().tolist()
    y_score = [item[1] for item in y_pred]

    #viz = RocCurveDisplay.from_estimator(
    #    classifier,
    #    X_test,
    #    y_test,
    #    name="ROC fold {}".format(i),
    #    alpha=0.3,
    #    lw=1,
    #    ax=ax,
    #)

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

    fpr, tpr, thresholds = roc_curve(y_test2.to_numpy(), y_score)
    axi.plot(fpr,tpr,lw=1,alpha=0.3, label = "ROC fold {}".format(i))
    interp_tpr = np.interp(mean_fpr, fpr, tpr)
    interp_tpr[0] = 0.0
    tprs.append(interp_tpr)
    aucs.append(roc_auc_score(y_test2, y_score))
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


figu.savefig("/user/dmarckx/public_html/ML/NN/ROCs/kfoldROCNN2_" + str(fract) + "drop" + str(int(dropval*100)) + "_" + str(epochs) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10)) + ")" + str(batchsize) + ".png")
plt.close(figu) 
