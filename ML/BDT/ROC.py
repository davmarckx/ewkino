import uproot
#import ROOT

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
from itertools import cycle
from sklearn import tree
from sklearn.model_selection import train_test_split, KFold
from sklearn.metrics import mean_squared_error
from sklearn.metrics import r2_score
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler, PolynomialFeatures
from sklearn.pipeline import make_pipeline
from sklearn.metrics import classification_report, confusion_matrix
from sklearn.metrics import plot_confusion_matrix
from sklearn.model_selection import GridSearchCV
from sklearn.ensemble import GradientBoostingClassifier
from sklearn.tree import DecisionTreeClassifier
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
from datetime import datetime
import xgboost as xgb
from xgboost import XGBClassifier
import sys
import ROOT

print('sklearn version' + str(__version__))


boostfeaturemap = {'_abs_eta_recoil':'f1', '_Mjj_max':'f2', '_deepFlavor_max':'f3',
       '_deepFlavor_leading':'f4', '_deepFlavor_subLeading':'f5', '_lT':'f6', '_pTjj_max':'f7',
       '_dRlb_min':'f8', '_dRl1l2':'f9', '_HT':'f10', '_nJets':'f11', '_nBJets':'f12',
       '_dRlWrecoil':'f13', '_dRlWbtagged':'f14', '_M3l':'f15', '_abs_eta_max':'f16', '_MET_pt':'f17',
       '_nMuons':'f18', '_leptonMVATOP_min':'f19',
       '_leptonChargeLeading':'f20',
       '_leptonPtLeading':'f21', '_leptonPtSubLeading':'f22', '_leptonEtaLeading':'f23',
       '_leptonEtaSubLeading':'f24', '_leptonELeading':'f25', '_leptonESubLeading':'f26',
       '_jetPtLeading':'f27', '_jetPtSubLeading':'f28', '_jetMassLeading':'f29',
       '_jetMassSubLeading':'f30','year':'f31'}

file_name = "/user/dmarckx/ewkino/ML/BDT/boostfeaturemaps/boostfeaturemap.pkl"
# save boostfeaturemap to keep up to date
pickle.dump(boostfeaturemap, open(file_name, "wb"))

n_estimators = 2#int(sys.argv[4])
max_depth = 2#int(sys.argv[3])
lr = 0.1#float(sys.argv[2])
year = '2018'#sys.argv[1]


#plotting packages
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt



#make ready

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

if year != 'all':
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_{}_dilep_BDT.pkl'.format(year)).sample(frac=0.2)
    alle1["year"] = 1


else:
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2018_dilep_BDT.pkl').sample(frac=0.2)
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2017_dilep_BDT.pkl').sample(frac=0.2)
    alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PostVFP_dilep_BDT.pkl').sample(frac=0.2)
    alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PreVFP_dilep_BDT.pkl').sample(frac=0.2)

    alle1["year"] = 1
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = 0

    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True).sample(frac = 0.25)



alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

y = alle1['class'] 

print(X.keys())
#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X.astype(float), y, test_size=0.20, random_state=13)

sums = X_train.groupby('class')["_weight"].sum()
print(sums)

class_imbalance_SF = sums[0]/sums[1]
print(class_imbalance_SF)

X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1
X_train['_weight_balanced'] = X_train['_weight_balanced'] * X_train['_weight']

#pd.Series(np.where(X_train["class"] == 1, X_train['_weight']*class_imbalance_SF, X_train['_weight']),X_train.index)

weight_train = X_train['_weight']
weight_test = X_test['_weight']
weight_train_balanced = X_train['_weight_balanced']

X_train = X_train.drop(['_weight', 'class','_weight_balanced'], axis = 1)
X_test = X_test.drop(['_weight', 'class'], axis = 1)


if (not y_train[y_train.isin(['TTW','TTX'])].empty):
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index)
    

X.rename(columns=boostfeaturemap,inplace=True)
X_train.rename(columns=boostfeaturemap,inplace=True)
X_test.rename(columns=boostfeaturemap,inplace=True)

print("do we have signals?")
print(not y_train[y_train.isin([1])].empty)


bst = XGBClassifier(n_estimators=n_estimators, max_depth=max_depth, learning_rate= lr, objective='binary:logistic',n_jobs = 4)
print("start training")
bst.fit(X_train.to_numpy(), y_train.to_numpy(), sample_weight=weight_train_balanced.to_numpy(),eval_set = [(X_train.to_numpy(), y_train.to_numpy()), (X_test.to_numpy(), y_test.to_numpy())])
print("done, plotting")

#y_pred_proba_orig = clf.predict_proba(X_test)[::,1]
y_pred_proba_orig = bst.predict_proba(X_test)[::,1]
fpr_orig, tpr_orig, _ = metrics.roc_curve(y_test,  y_pred_proba_orig, sample_weight = weight_test)
aucval_orig = metrics.roc_auc_score(y_test, y_pred_proba_orig, sample_weight = weight_test)

y_pred_proba = bst.predict_proba(X_train)[::,1]
fpr, tpr, _ = metrics.roc_curve(y_train,  y_pred_proba)
aucval = metrics.roc_auc_score(y_train, y_pred_proba, sample_weight = weight_train)

ax = plt.figure(figsize=(20,20))

ax.text(0.15, 0.87, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=32,
        fontweight='bold')
ax.text(0.15, 0.84, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=23)

plt.plot(fpr_orig,tpr_orig,color="red",label="test auc="+str(aucval_orig))
plt.plot(fpr,tpr,color="darkorange",label="train auc="+str(aucval))
plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
plt.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/BDT/ROC_{}_final_20psmalltrain_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()

results = bst.evals_result()

epochs = len(results['validation_0']['logloss'])
x_axis = range(0, epochs)

fig, ax = plt.subplots()
ax.plot(x_axis, results['validation_0']['logloss'], label='Train')
ax.plot(x_axis, results['validation_1']['logloss'], label='Test')
ax.legend()
plt.xlabel('\nEpochs',fontsize=14,fontweight='semibold')
plt.ylabel('Error\n',fontsize=14,fontweight='semibold')
plt.title('XGBoost learning curve\n',fontsize=20,fontweight='semibold')
plt.savefig("/user/dmarckx/public_html/ML/BDT/loss_{}_final_20psmalltrain_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()

file_name = "/user/dmarckx/ewkino/ML/models/XGB_{}_nativetest".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + ".pkl"

# save
pickle.dump(bst, open(file_name, "wb"))
#print(bst.get_booster().feature_names)
ROOT.TMVA.Experimental.SaveXGBoost(bst, "XGB", "../models/XGBtest_{}.root".format(year), num_inputs=len(X_train.columns))
