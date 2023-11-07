import uproot
import ROOT

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
import xgboost as xgb
from xgboost import XGBClassifier

#other imports
from datetime import datetime
import sys


#plotting packages and set nice seaborn plotting style
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt


print('sklearn version' + str(__version__))


#this featuremap is needed to save the xgboost model in a structure accepted by TMVA (native xgboost featurenames)
boostfeaturemap = {'_abs_eta_recoil':'f1', '_Mjj_max':'f2', '_deepFlavor_max':'f3',
       '_deepFlavor_leading':'f4', '_deepFlavor_subLeading':'f5', '_lT':'f6', '_pTjj_max':'f7',
       '_dRlb_min':'f8', '_dRl1l2':'f9', '_HT':'f10', '_nJets':'f11', '_nBJets':'f12',
       '_dRlWrecoil':'f13', '_dRlWbtagged':'f14', '_M3l':'f15', '_abs_eta_max':'f16', '_MET_pt':'f17',
       '_nMuons':'f18', #'_leptonMVATOP_min':'f19',
       '_leptonChargeLeading':'f19',
       '_leptonPtLeading':'f20', '_leptonPtSubLeading':'f21', '_leptonEtaLeading':'f22',
       '_leptonEtaSubLeading':'f23', '_leptonELeading':'f24', '_leptonESubLeading':'f25',
       '_jetPtLeading':'f26', '_jetPtSubLeading':'f27', '_jetMassLeading':'f28',
       '_jetMassSubLeading':'f29','year':'f30'}#,'_jetEtaLeading':'f31', '_jetEtaSubLeading':'f32',
       #'_dRl2btagged':'f33', '_dRl1btagged':'f34'}

file_name = "/user/dmarckx/ewkino/ML/BDT/boostfeaturemaps/boostfeaturemap.pkl"
# save boostfeaturemap to keep up to date and translate back
pickle.dump(boostfeaturemap, open(file_name, "wb"))


#get the xgboost parameters
BDTloc = "XGB_all_robustnessOFF31_3000_3_0.1.pkl"
robustBDTloc = "XGB_all_robustnessON31_3000_3_0.1.pkl"
year = 'all'
#make ready

# Opening JSON file that contains the renaming of classes 
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)


fract = 0.2
if year != 'all':
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_robustBDT_{}_robust_BDT.pkl'.format(year)).sample(frac=fract, random_state=13)
    alle1["year"] = 1


else:
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_robustBDT_2018_robust_BDT.pkl').sample(frac=fract, random_state=13)
    alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_robustBDT_2017_robust_BDT.pkl').sample(frac=fract, random_state=13)
    alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_robustBDT_2016PostVFP_robust_BDT.pkl').sample(frac=fract, random_state=13)
    alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_robustBDT_2016PreVFP_robust_BDT.pkl').sample(frac=fract, random_state=13)

    # years are this way because OHE is too sparse for gBDTs and 0123 is an 'unphysical' ordering which didn't outperform in early benchmarks
    alle1["year"] = 2
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = -1
    
    # now concat them together
    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)


other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_robustBDT_2018_robust_BDT.pkl')
other2 = pd.read_pickle('../ML_dataframes/trainsets/otherset_robustBDT_2017_robust_BDT.pkl')
other3 = pd.read_pickle('../ML_dataframes/trainsets/otherset_robustBDT_2016PostVFP_robust_BDT.pkl')
other4 = pd.read_pickle('../ML_dataframes/trainsets/otherset_robustBDT_2016PreVFP_robust_BDT.pkl')

other1["year"] = 2
other2["year"] = 1
other3["year"] = 0
other4["year"] = -1

#now concat them together
other1 = pd.concat([other1,other2,other3,other4], ignore_index=True)



# only keep positive weight training samples
alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
alle1 = alle1.replace({"class": dictio})

other1["region"] = "dilep"
other1 = other1[other1["_weight"]>0]
other1 = other1.replace({"class": dictio})
other1 = other1[other1["class"]!='TTW'] #remove signal samples to only inject the test signal samples later on (otherwise bias)

#make other validation sets
X_other = other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','_leptonMVATOP_min',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','deltaEtaLeadingLeptonPair','_bjetEtaLeading','_bjetPtLeading','_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry','_reweight','_leptonAbsEtaLeading', '_leptonAbsEtaSubLeading','_leptonAbsEtaTrailing','_leptonMaxEta','_jetAbsEtaLeading', '_jetAbsEtaSubLeading','_bjetAbsEtaLeading', 'deltaPhiLeadingLeptonPair','deltaRLeadingLeptonPair','mLeadingLeptonPair', '_nJetsNBJetsCat', '_nJetsNZCat'], axis=1)

#X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
#X_other.loc[X_other['class'] != 1, 'class'] = 0

# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','_leptonMVATOP_min',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','deltaEtaLeadingLeptonPair','_bjetEtaLeading','_bjetPtLeading','_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry','_reweight','_leptonAbsEtaLeading', '_leptonAbsEtaSubLeading','_leptonAbsEtaTrailing','_leptonMaxEta','_jetAbsEtaLeading', '_jetAbsEtaSubLeading','_bjetAbsEtaLeading', 'deltaPhiLeadingLeptonPair','deltaRLeadingLeptonPair','mLeadingLeptonPair', '_nJetsNBJetsCat', '_nJetsNZCat'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] != 1, 'class'] = 0
X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
X_other.loc[X_other['class'] != 1, 'class'] = 0
y = X['class'] 
y_other = X_other['class']



#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X.astype(float), y, test_size=0.20, random_state=13)

# calculate class imbalance
sums = X_train.groupby('class')["_weight"].sum()
print(sums)
class_imbalance_SF = sums[0]/sums[1]
print(class_imbalance_SF)


# rescale training weights
X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_SF
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1
X_train['_weight_balanced'] = X_train['_weight_balanced'] * X_train['_weight']

weight_train = X_train['_weight']
weight_test = X_test['_weight']
weight_train_balanced = X_train['_weight_balanced']



# last unused features can be dropped
X_train_nominal = X_train[X_train["nominal"].isnull()]
y_train_nominal = X_train_nominal['class'].astype(int)
weight_train_nominal =  X_train_nominal['_weight']
X_train_nominal = X_train_nominal.drop(['_weight', 'class','nominal', '_weight_balanced'], axis = 1)
X_train_ud = X_train[~X_train["nominal"].isnull()]
y_train_ud = X_train_ud['class'].astype(int)
weight_train_ud =  X_train_ud['_weight']
X_train_ud = X_train_ud.drop(['_weight', 'class','nominal', '_weight_balanced'], axis = 1)
X_train = X_train.drop(['_weight', 'class','_weight_balanced','nominal'], axis = 1)

X_other = pd.concat([X_other, X_test[X_test["class"]==1]],ignore_index=True)

X_test_nominal = X_test[X_test["nominal"].isnull()]
y_test_nominal = X_test_nominal['class'].astype(int)
weight_test_nominal =  X_test_nominal['_weight']
X_test_nominal = X_test_nominal.drop(['_weight', 'class','nominal'], axis = 1)
X_test_ud = X_test[~X_test["nominal"].isnull()]
y_test_ud = X_test_ud['class'].astype(int)
weight_test_ud =  X_test_ud['_weight']
X_test_ud = X_test_ud.drop(['_weight', 'class','nominal'], axis = 1)
X_test = X_test.drop(['_weight', 'class','nominal'], axis = 1)



y_other = X_other['class'].astype(int)
weight_other =  X_other['_weight']
X_other_nominal = X_other[X_other["nominal"].isnull()]
y_other_nominal = X_other_nominal['class'].astype(int)
weight_other_nominal =  X_other_nominal['_weight']
X_other_nominal = X_other_nominal.drop(['_weight', 'class','nominal'], axis = 1)
X_other_ud = X_other[~X_other["nominal"].isnull()]
y_other_ud = X_other_ud['class'].astype(int)
weight_other_ud =  X_other_ud['_weight']
X_other_ud = X_other_ud.drop(['_weight', 'class','nominal'], axis = 1)
X_other = X_other.drop(['_weight', 'class','nominal'], axis = 1)


# last safety to modify classes to numerical
if (not y_train[y_train.isin(['TTW','TTX'])].empty):
    print("some feature was not ")
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index).astype(int)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index).astype(int)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index).astype(int)
    

# rename the features so we can save the model to TMVA
X.rename(columns=boostfeaturemap,inplace=True)
X_train.rename(columns=boostfeaturemap,inplace=True)
X_test.rename(columns=boostfeaturemap,inplace=True)
X_other.rename(columns=boostfeaturemap,inplace=True)


# safety for when you test with small samples
print("do we have signals?")
print(not y_train[y_train.isin([1])].empty)

file = open('../models/'+BDTloc, 'rb')
# dump information to that file
model = pickle.load(file)

file = open('../models/'+robustBDTloc, 'rb')
# dump information to that file
robustmodel = pickle.load(file)

y_pred_proba_111 = robustmodel.predict_proba(X_test_nominal)[::,1]
fpr_111, tpr_111, _ = metrics.roc_curve(y_test_nominal.astype(int), y_pred_proba_111.astype(float), sample_weight = weight_test_nominal.astype(float))
aucval_111 = metrics.roc_auc_score(y_test_nominal.astype(int), y_pred_proba_111.astype(float), sample_weight = weight_test_nominal.astype(float))

y_pred_proba_222 = robustmodel.predict_proba(X_train_nominal)[::,1]
fpr_222, tpr_222, _ = metrics.roc_curve(y_train_nominal.astype(int),  y_pred_proba_222.astype(float), sample_weight = weight_train_nominal.astype(float))
aucval_222 = metrics.roc_auc_score(y_train_nominal.astype(int), y_pred_proba_222.astype(float), sample_weight = weight_train_nominal.astype(float))

y_pred_proba_333 = robustmodel.predict_proba(X_other_nominal)[::,1]
fpr_333, tpr_333, _ = metrics.roc_curve(y_other_nominal.astype(int),  y_pred_proba_333.astype(float), sample_weight = weight_other_nominal.astype(float))
aucval_333 = metrics.roc_auc_score(y_other_nominal.astype(int), y_pred_proba_333.astype(float), sample_weight = weight_other_nominal.astype(float))

y_pred_proba_1111 = robustmodel.predict_proba(X_test_ud)[::,1]
fpr_1111, tpr_1111, _ = metrics.roc_curve(y_test_ud.astype(int), y_pred_proba_1111.astype(float), sample_weight = weight_test_ud.astype(float))
aucval_1111 = metrics.roc_auc_score(y_test_ud.astype(int), y_pred_proba_1111.astype(float), sample_weight = weight_test_ud.astype(float))

y_pred_proba_2222 = robustmodel.predict_proba(X_train_ud)[::,1]
fpr_2222, tpr_2222, _ = metrics.roc_curve(y_train_ud.astype(int),  y_pred_proba_2222.astype(float), sample_weight = weight_train_ud.astype(float))
aucval_2222 = metrics.roc_auc_score(y_train_ud.astype(int), y_pred_proba_2222.astype(float), sample_weight = weight_train_ud.astype(float))

y_pred_proba_3333 = robustmodel.predict_proba(X_other_ud)[::,1]
fpr_3333, tpr_3333, _ = metrics.roc_curve(y_other_ud.astype(int),  y_pred_proba_3333.astype(float), sample_weight = weight_other_ud.astype(float))
aucval_3333 = metrics.roc_auc_score(y_other_ud.astype(int), y_pred_proba_3333.astype(float), sample_weight = weight_other_ud.astype(float))


#========================================================================================

#print(list(X_test_nominal.columns))
#X_test_nominal = X_test_nominal.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)
#X_train_nominal = X_train_nominal.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)
#X_other_nominal = X_other_nominal.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)
#X_test_ud = X_test_ud.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)
#X_train_ud = X_train_ud.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)
#X_other_ud = X_other_ud.drop(["_dRl1jet"], axis = 1)#,'_nLooseBJets','_nTightBJets','_jetEtaLeading','_jetEtaSubLeading','_bjetPtLeading','_bjetEtaLeading','deltaEtaLeadingLeptonPair'], axis = 1)

# calculations for ROC
y_pred_proba_1 = model.predict_proba(X_test_nominal)[::,1]
fpr_1, tpr_1, _ = metrics.roc_curve(y_test_nominal.astype(int), y_pred_proba_1.astype(float), sample_weight = weight_test_nominal.astype(float))
aucval_1 = metrics.roc_auc_score(y_test_nominal.astype(int), y_pred_proba_1.astype(float), sample_weight = weight_test_nominal.astype(float))

y_pred_proba_2 = model.predict_proba(X_train_nominal)[::,1]
fpr_2, tpr_2, _ = metrics.roc_curve(y_train_nominal.astype(int),  y_pred_proba_2.astype(float), sample_weight = weight_train_nominal.astype(float))
aucval_2 = metrics.roc_auc_score(y_train_nominal.astype(int), y_pred_proba_2.astype(float), sample_weight = weight_train_nominal.astype(float))

y_pred_proba_3 = model.predict_proba(X_other_nominal)[::,1]
fpr_3, tpr_3, _ = metrics.roc_curve(y_other_nominal.astype(int),  y_pred_proba_3.astype(float), sample_weight = weight_other_nominal.astype(float))
aucval_3 = metrics.roc_auc_score(y_other_nominal.astype(int), y_pred_proba_3.astype(float), sample_weight = weight_other_nominal.astype(float))

y_pred_proba_11 = model.predict_proba(X_test_ud)[::,1]
fpr_11, tpr_11, _ = metrics.roc_curve(y_test_ud.astype(int), y_pred_proba_11.astype(float), sample_weight = weight_test_ud.astype(float))
aucval_11 = metrics.roc_auc_score(y_test_ud.astype(int), y_pred_proba_11.astype(float), sample_weight = weight_test_ud.astype(float))

y_pred_proba_22 = model.predict_proba(X_train_ud)[::,1]
fpr_22, tpr_22, _ = metrics.roc_curve(y_train_ud.astype(int),  y_pred_proba_22.astype(float), sample_weight = weight_train_ud.astype(float))
aucval_22 = metrics.roc_auc_score(y_train_ud.astype(int), y_pred_proba_22.astype(float), sample_weight = weight_train_ud.astype(float))

y_pred_proba_33 = model.predict_proba(X_other_ud)[::,1]
fpr_33, tpr_33, _ = metrics.roc_curve(y_other_ud.astype(int),  y_pred_proba_33.astype(float), sample_weight = weight_other_ud.astype(float))
aucval_33 = metrics.roc_auc_score(y_other_ud.astype(int), y_pred_proba_33.astype(float), sample_weight = weight_other_ud.astype(float))

#=========================================================

#make the plot
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

#plt.plot(fpr_1,tpr_1,color="red",label="test auc="+str("{:.2f}".format(aucval_1)))
#plt.plot(fpr_11,tpr_11,color="red", linestyle='dashed',label="test variations auc="+str("{:.2f}".format(aucval_11)))

plt.plot(fpr_2,tpr_2,color="darkorange",label="train auc="+str("{:.2f}".format(aucval_2)))
plt.plot(fpr_22,tpr_22,color="darkorange", linestyle='dashed',label="train variations auc="+str("{:.2f}".format(aucval_22)))


plt.plot(fpr_222,tpr_222,color="red",label="retrained train auc="+str("{:.2f}".format(aucval_222)))
plt.plot(fpr_2222,tpr_2222,color="red", linestyle='dashed',label="retrained train variations auc="+str("{:.2f}".format(aucval_2222)))
#plt.plot(fpr_3,tpr_3,color="green",label="other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_3)))
#plt.plot(fpr_33,tpr_33,color="green", linestyle='dashed',label="varied other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_33)))


plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
plt.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/robustness/ROC_robustnesscheck_train.png")
plt.close()

#make the plot
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

plt.plot(fpr_1,tpr_1,color="darkorange",label="test auc="+str("{:.2f}".format(aucval_1)))
plt.plot(fpr_11,tpr_11,color="darkorange", linestyle='dashed',label="test variations auc="+str("{:.2f}".format(aucval_11)))


plt.plot(fpr_111,tpr_111,color="red",label="retrained test auc="+str("{:.2f}".format(aucval_111)))
plt.plot(fpr_1111,tpr_1111,color="red", linestyle='dashed',label="retrained test variations auc="+str("{:.2f}".format(aucval_1111)))
#plt.plot(fpr_2,tpr_2,color="darkorange",label="train auc="+str("{:.2f}".format(aucval)))
#plt.plot(fpr_22,tpr_22,color="darkorange", linestyle='dashed',label="train variations auc="+str("{:.2f}".format(aucval_22)))

#plt.plot(fpr_3,tpr_3,color="green",label="other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_3)))
#plt.plot(fpr_33,tpr_33,color="green", linestyle='dashed',label="varied other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_33)))


plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
plt.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/robustness/ROC_robustnesscheck_test.png")
plt.close()

#make the plot
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

#plt.plot(fpr_1,tpr_1,color="red",label="test auc="+str("{:.2f}".format(aucval_1)))
#plt.plot(fpr_11,tpr_11,color="red", linestyle='dashed',label="test variations auc="+str("{:.2f}".format(aucval_11)))

#plt.plot(fpr_2,tpr_2,color="darkorange",label="train auc="+str("{:.2f}".format(aucval)))
#plt.plot(fpr_22,tpr_22,color="darkorange", linestyle='dashed',label="train variations auc="+str("{:.2f}".format(aucval_22)))

plt.plot(fpr_3,tpr_3,color="darkorange",label="other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_3)))
plt.plot(fpr_33,tpr_33,color="darkorange", linestyle='dashed',label="varied other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_33)))


plt.plot(fpr_333,tpr_333,color="red",label="retrained other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_333)))
plt.plot(fpr_3333,tpr_3333,color="red", linestyle='dashed',label="retrained varied other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_3333)))

plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
plt.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/robustness/ROC_robustnesscheck_other.png")
plt.close()



diff11 = y_pred_proba_11 - y_test_ud
diff1111 = y_pred_proba_1111 - y_test_ud



diff33 = y_pred_proba_33 - y_other_ud
diff3333 = y_pred_proba_3333 - y_other_ud



diff22 = y_pred_proba_22 - y_train_ud
diff2222 = y_pred_proba_2222 - y_train_ud

f, ax = plt.subplots(figsize=(10,10))
line1 =sns.histplot(data=diff11, x= diff11 , bins=50, binrange = (-1,1),fill=True, weights=weight_test_ud,linewidth=2, common_bins=True, stat='density', alpha=0.1, element="step")
line2 =sns.histplot(data=diff1111, x= diff1111 , bins=50, binrange = (-1,1),fill=False, weights=weight_test_ud,linewidth=2, common_bins=True, stat='density', alpha=0.1, element="step")


plt.rc('legend', fontsize=15)
ax.set_title(r'BDT error stability for test sample', fontsize=25,fontweight="bold")
ax.set_xlabel(r'signal output node error', fontsize=25)#r'
ax.set_ylabel('Events A.U.', fontsize=25)

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

for label in ax.get_xticklabels()[::2]:
    label.set_visible(False)

plt.xticks(fontsize= 20)
plt.yticks(fontsize= 20)
#plt.setp(ax.get_xticklabels(), rotation=-30, horizontalalignment='right')
#plt.legend(title=r'$\bf{Event \; Type}$',prop=dict(weight='bold'),title_fontsize='xx-large', loc='upper right', fancybox=True)
#ax.legend(['background before final selection','background after correlated BDT selection','background discrete mass decorrelated',"signal", 'background ANN selection'], loc="center right")

plt.savefig("/user/dmarckx/public_html/ML/robustness/score_error_test.png")
plt.close()

f, ax = plt.subplots(figsize=(10,10))
line1 =sns.histplot(data=diff22, x= diff22 , bins=50, binrange = (-1,1),fill=True, weights=weight_test_ud,linewidth=2, common_bins=True, stat='count', alpha=0.1, element="step")
line2 =sns.histplot(data=diff2222, x= diff2222 , bins=50, binrange = (-1,1),fill=False, weights=weight_test_ud,linewidth=2, common_bins=True, stat='count', alpha=0.1, element="step")


plt.rc('legend', fontsize=15)
ax.set_title(r'BDT error stability for train sample', fontsize=25,fontweight="bold")
ax.set_xlabel(r'signal output node error', fontsize=25)#r'
ax.set_ylabel('Events A.U.', fontsize=25)

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

for label in ax.get_xticklabels()[::2]:
    label.set_visible(False)

plt.xticks(fontsize= 20)
plt.yticks(fontsize= 20)
#plt.setp(ax.get_xticklabels(), rotation=-30, horizontalalignment='right')
#plt.legend(title=r'$\bf{Event \; Type}$',prop=dict(weight='bold'),title_fontsize='xx-large', loc='upper right', fancybox=True)
#ax.legend(['background before final selection','background after correlated BDT selection','background discrete mass decorrelated',"signal", 'background ANN selection'], loc="center right")

plt.savefig("/user/dmarckx/public_html/ML/robustness/score_error_train.png")
plt.close()



f, ax = plt.subplots(figsize=(10,10))
line1 =sns.histplot(data=diff33, x= diff33 , bins=50, binrange = (-1,1),fill=True, weights=weight_test_ud,linewidth=2, common_bins=True, stat='count', alpha=0.1, element="step")
line2 =sns.histplot(data=diff3333, x= diff3333 , bins=50, binrange = (-1,1),fill=False, weights=weight_test_ud,linewidth=2, common_bins=True, stat='count', alpha=0.1, element="step")


plt.rc('legend', fontsize=15)
ax.set_title(r'BDT error stability for other sample', fontsize=25,fontweight="bold")
ax.set_xlabel(r'signal output node error', fontsize=25)#r'
ax.set_ylabel('Events A.U.', fontsize=25)

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

for label in ax.get_xticklabels()[::2]:
    label.set_visible(False)

plt.xticks(fontsize= 20)
plt.yticks(fontsize= 20)
#plt.setp(ax.get_xticklabels(), rotation=-30, horizontalalignment='right')
#plt.legend(title=r'$\bf{Event \; Type}$',prop=dict(weight='bold'),title_fontsize='xx-large', loc='upper right', fancybox=True)
#ax.legend(['background before final selection','background after correlated BDT selection','background discrete mass decorrelated',"signal", 'background ANN selection'], loc="center right")

plt.savefig("/user/dmarckx/public_html/ML/robustness/score_error_other.png")
plt.close()
