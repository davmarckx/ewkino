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
from sklearn.metrics import RocCurveDisplay
from sklearn.model_selection import StratifiedKFold
from sklearn import __version__
from datetime import datetime
import xgboost as xgb
from xgboost import XGBClassifier
import sys

print('sklearn version' + str(__version__))

year = sys.argv[1]

#plotting packages
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt



#make ready

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_{}_dilep_BDT.pkl'.format(year)).sample(frac=0.2)
alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

background = alle1[alle1["class"] != 'TTW']
signal = alle1[alle1["class"] == 'TTW']



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

# Create parameter grid
param_grid = {'learning_rate': [1,0.75,0.5, 0.25, .1, 0.05],
                  'n_estimators': [50, 500, 1000, 1500, 2000, 2500, 3000]}

# Create grid search 
grid_search = GridSearchCV(XGBClassifier(max_depth = 3,objective='binary:logistic',nthread = 4) , param_grid, cv = 4,scoring = 'balanced_accuracy', n_jobs = 4)

# Fit grid search
grid_search.fit(X_train, y_train,sample_weight=weight_train_balanced)



results = pd.DataFrame(grid_search.cv_results_)
hm_data = results.pivot(index='param_learning_rate', 
                   columns='param_n_estimators',
                   values='mean_test_score')

# Plot heatmap
fix, ax = plt.subplots(figsize=(9, 6))

ax = sns.heatmap(hm_data,
                 annot=True,
                 cmap='RdBu',
                 linecolor='white')

ax.set_title('Hyperparameter Gridsearch', fontsize=16)
ax.set_ylabel('learning rate', fontsize=12)
ax.set_xlabel('estimators', fontsize=12)


now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig('/user/dmarckx/public_html/ML/BDT/gridsearch/gridsearch_BDT_' + dt_string + '.png')

print("Best parameters: {} \n\n".format(grid_search.best_params_))
print("Best cross-validation score: {:.2f}".format(grid_search.best_score_))
