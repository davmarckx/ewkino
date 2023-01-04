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
n_estimators = int(sys.argv[4])
max_depth = int(sys.argv[3])
lr = float(sys.argv[2])
year = sys.argv[1]

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)


fract = 0.001
if year != 'all':
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_{}_dilep_BDT.pkl'.format(year)).sample(frac=fract, random_state=13)
    alle1["year"] = 1


else:
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2018_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2017_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PostVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDT_2016PreVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)

    alle1["year"] = 1
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = 0

    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)


other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2018_dilep_BDT.pkl')
other2 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2017_dilep_BDT.pkl')
other3 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2016PostVFP_dilep_BDT.pkl')
other4 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2016PreVFP_dilep_BDT.pkl')

other1["year"] = 1
other2["year"] = 1
other3["year"] = 0
other4["year"] = 0

other1 = pd.concat([other1,other2,other3,other4], ignore_index=True)

alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

other1["region"] = "dilep"
other1 = other1[other1["_weight"]>0]
other1 = other1.replace({"class": dictio})
other1 = other1[other1["class"]!='TTW'] #remove signal samples to only inject the test signal samples


# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0

y = alle1['class'] 

#make other validation sets
X_other = other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
X_other.loc[X_other['class'] != 1, 'class'] = 0

weight_other = X_other["_weight"]
X_other = X_other.drop(['_weight', 'class'], axis = 1)

sums = X.groupby('class')["_weight"].sum()
print(sums)

class_imbalance_SF = sums[0]/sums[1]
print(class_imbalance_SF)

X['_weight_balanced'] = X['_weight']
X.loc[X['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X.loc[X['class'] == 0, '_weight_balanced'] = 1
X['_weight_balanced'] = X['_weight_balanced'] * X['_weight']

weight_train = X['_weight']
weight_train_balanced = X['_weight_balanced']

X = X.drop(['_weight', 'class','_weight_balanced'], axis = 1)


if (not y[y.isin(['TTW','TTX'])].empty):
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index)
    

print("do we have signals?")
print(not y[y.isin([1])].empty)

cv = StratifiedKFold(n_splits=5)
classifier = XGBClassifier(n_estimators=n_estimators, max_depth=max_depth, learning_rate=lr, objective='binary:logistic',nthread=-1)

tprs = []
aucs = []
mean_fpr = np.linspace(0, 1, 100)
tprs_other = []
aucs_other = []
mean_fpr_other = np.linspace(0, 1, 100)

fig, ax = plt.subplots(figsize=(20,20))

i = 1
for train_index, test_index in cv.split(X, y):
    X_train2, X_test2 = X.iloc[train_index], X.iloc[test_index]
    y_train2, y_test2 = y.iloc[train_index], y.iloc[test_index] 

    X_other2 = pd.concat([X_other, X_test2[y_test2==1]],ignore_index=True)
    y_other2 = pd.concat([y_other, y_test2[y_test2==1]],ignore_index=True)

    weight_train_balanced2 = weight_train_balanced.iloc[train_index]
    weight_test2 = weight_train.iloc[test_index]
    weight_other2 = pd.concat([weight_other, weight_test2[y_test2==1]],ignore_index=True)

    
    classifier.fit(X_train2, y_train2, sample_weight=weight_train_balanced2)
    
    y_score = classifier.predict_proba(X_test2)[::,1]
    y_scoreother = classifier.predict_proba(X_other2)[::,1]    

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
    
    fpr, tpr, thresholds = roc_curve(y_test2.to_numpy(), y_score, sample_weight = weight_test)
    fpr_other, tpr_other, tresholds = roc_curve(y_other2.to_numpy(), y_scoreother)
    ax.plot(fpr,tpr,lw=1,alpha=0.3, label = "ROC fold {}".format(i))
    ax.plot(fpr_other,tpr_other,lw=1,alpha=0.3, label = "Other backgrounds fold {}".format(i))
    interp_tpr = np.interp(mean_fpr, fpr, tpr)
    interp_tpr[0] = 0.0
    tprs.append(interp_tpr)
    interp_tpr_other = np.interp(mean_fpr_other, fpr_other, tpr_other)
    interp_tpr_other[0] = 0.0
    tprs_other.append(interp_tpr_other)
    aucs.append(roc_auc_score(y_test2, y_score))
    aucs_other.append(roc_auc_score(y_other2, y_scoreother))
    i += 1

ax.plot([0, 1], [0, 1], linestyle="--", lw=1, color="r", label="Chance", alpha=0.8)


#test set
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

#other set
mean_tpr_other = np.mean(tprs_other, axis=0)
mean_tpr_other[-1] = 1.0
mean_auc_other = metrics.auc(mean_fpr_other, mean_tpr_other)
std_auc_other = np.std(aucs_other)
ax.plot(
    mean_fpr_other,
    mean_tpr_other,
    color="y",
    label=r"Mean ROC other (AUC = %0.2f $\pm$ %0.4f)" % (mean_auc, std_auc),
    lw=2,
    alpha=0.8,
)

std_tpr_other = np.std(tprs_other, axis=0)
tprs_upper_other = np.minimum(mean_tpr_other + std_tpr_other, 1)
tprs_lower_other = np.maximum(mean_tpr_other - std_tpr_other, 0)
ax.fill_between(
    mean_fpr_other,
    tprs_lower_other,
    tprs_upper_other,
    color="brown",
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
