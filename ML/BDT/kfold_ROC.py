import uproot

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random
import pickle
import json

#sklearn imports
import sklearn
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split, KFold
from sklearn.metrics import classification_report, confusion_matrix
from sklearn.metrics import plot_confusion_matrix
from sklearn.model_selection import GridSearchCV
from sklearn.utils.class_weight import compute_class_weight
from sklearn.utils.class_weight import compute_sample_weight

from sklearn import metrics
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score
from sklearn.model_selection import StratifiedKFold
from sklearn import __version__
from datetime import datetime
import xgboost as xgb
from xgboost import XGBClassifier
import sys

print('sklearn version' + str(__version__))


#plotting packages
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt



#make ready
n_estimators=2000
max_depth = 3
lr = 0.5
year = sys.argv[1]

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

alle1 = pd.read_pickle('/pnfs/iihe/cms/store/user/dmarckx/ML_dataframes/trainsets/trainset_all_{}_dilep.pkl'.format(year)).sample(frac=0.2)
alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

y = alle1['class'] 


#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20, random_state=13)

sums = X_train.groupby('class')["_weight"].sum()
print(sums)

class_imbalance_SF = sums[0]/sums[1]
print(class_imbalance_SF)

X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1
X_train['_weight_balanced'] = X_train['_weight_balanced'] * X_train['_weight']

weight_train = X_train['_weight']
weight_test = X_test['_weight']
weight_train_balanced = X_train['_weight_balanced']

X_train = X_train.drop(['_weight', 'class','_weight_balanced'], axis = 1)
X_test = X_test.drop(['_weight', 'class'], axis = 1)


if (not y_train[y_train.isin(['TTW','TTX'])].empty):
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index)
    

print("do we have signals?")
print(not y_train[y_train.isin([1])].empty)

cv = StratifiedKFold(n_splits=10)
classifier = XGBClassifier(n_estimators=n_estimators, max_depth=max_depth, learning_rate=lr, objective='binary:logistic',nthread=-1)

tprs = []
aucs = []
mean_fpr = np.linspace(0, 1, 100)

fig, ax = plt.subplots(figsize=(20,20))

i = 1
for train_index, test_index in cv.split(X, y):
    X_train2, X_test2 = X_train.iloc[train_index], X_train.iloc[test_index]
    y_train2, y_test2 = y_train.iloc[train_index], y_train.iloc[test_index] 
    weight_train_balanced2 = weight_train_balanced.iloc[train_index]
    
    classifier.fit(X_train2, y_train2, sample_weight=weight_train_balanced2)
    
    y_score = classifier.predict_proba(X_test2)[::,1]
    
    ax.text(0.05, 0.98, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=28,
        fontweight='bold',
        transform=ax.transAxes)
    ax.text(0.05, 0.92, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=20,
        transform=ax.transAxes)
    
    fpr, tpr, thresholds = roc_curve(y_test2.to_numpy(), y_score)
    ax.plot(fpr,tpr,lw=1,alpha=0.3, label = "ROC fold {}".format(i))
    interp_tpr = np.interp(mean_fpr, fpr, tpr)
    interp_tpr[0] = 0.0
    tprs.append(interp_tpr)
    aucs.append(roc_auc_score(y_test2, y_score))
    i += 1

ax.plot([0, 1], [0, 1], linestyle="--", lw=1, color="r", label="Chance", alpha=0.8)

mean_tpr = np.mean(tprs, axis=0)
mean_tpr[-1] = 1.0
mean_auc = metrics.auc(mean_fpr, mean_tpr)
std_auc = np.std(aucs)
ax.plot(
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
ax.fill_between(
    mean_fpr,
    tprs_lower,
    tprs_upper,
    color="grey",
    alpha=0.2,
    label=r"$\pm$ 1 std. dev.",
)

ax.set(
    xlim=[-0.05, 1.05],
    ylim=[-0.05, 1.05],
    title="Receiver operating characteristic curve")
ax.legend(loc="lower right",)
plt.setp(ax.get_legend().get_texts(), fontsize=25) # for legend text
plt.setp(ax.get_legend().get_title(), fontsize=25) #for legend title
plt.xticks(fontsize= 20)
plt.yticks(fontsize= 20)
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)


now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/BDT/kfoldROC_{}_".format(year) + "_" + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
