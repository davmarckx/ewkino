#this is a N dimensional gridsearch, which stores the results in a pickled file.

import uproot

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random
import pickle
import json
import math

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
from sklearn.multiclass import OneVsRestClassifier
from sklearn.preprocessing import label_binarize
from sklearn.metrics import RocCurveDisplay
from sklearn.model_selection import StratifiedKFold
from sklearn import __version__
import xgboost as xgb
from xgboost import XGBClassifier


import sys
from datetime import datetime


print('sklearn version' + str(__version__))

#plotting packages
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt



# the year
year = sys.argv[1]


# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

if year=="all":
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2018_dilep_BDT.pkl').sample(frac=0.2)
    alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2017_dilep_BDT.pkl').sample(frac=0.2)
    alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PostVFP_dilep_BDT.pkl').sample(frac=0.2)
    alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PreVFP_dilep_BDT.pkl').sample(frac=0.2)

    alle1["year"] = 1
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = 0

    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)

else: 
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_{}_dilep_BDT.pkl'.format(year)).sample(frac=fract, random_state=13)
    alle1["year"] = 1


alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})



# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi','_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_yield', 'region','_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing','_numberOfVertices',"_leptonMVAttH_min"], axis=1)
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

# Create parameter grid
param_grid = {'learning_rate': [0.25,0.1,0.05,0.01,0.005],
                  'n_estimators': [1000,2000,3000,4000,5000],
                  'max_depth': [2,3,4,5]}
                

# Create grid search 
grid_search = GridSearchCV(XGBClassifier(objective='binary:logistic',nthread = 3) , param_grid, cv = 5,scoring = 'balanced_accuracy', n_jobs = 5)

# Fit grid search
grid_search.fit(X_train, y_train,sample_weight=weight_train_balanced)



results = pd.DataFrame(grid_search.cv_results_)

results.to_pickle("gridresults/pickled_gridresults_allyears.pkl")


print("Best parameters: {} \n\n".format(grid_search.best_params_))
print("Best cross-validation score: {:.2f}".format(grid_search.best_score_))
