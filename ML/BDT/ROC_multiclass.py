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
       '_jetMassSubLeading':'f29','year':'f30'}

file_name = "/user/dmarckx/ewkino/ML/BDT/boostfeaturemaps/boostfeaturemap.pkl"
# save boostfeaturemap to keep up to date and translate back
pickle.dump(boostfeaturemap, open(file_name, "wb"))


#get the xgboost parameters
n_estimators = int(sys.argv[4])
max_depth = int(sys.argv[3])
lr = float(sys.argv[2])
year = sys.argv[1]


#make ready

# Opening JSON file that contains the renaming of classes 
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)


fract = 0.2
if year != 'all':
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDTnew_{}_dilep_BDT.pkl'.format(year)).sample(frac=fract, random_state=13)
    alle1["year"] = 1


else:
    alle1 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDTnew_2018_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle2 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDTnew_2017_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle3 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDTnew_2016PostVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle4 = pd.read_pickle('../ML_dataframes/trainsets/trainset_smallBDTnew_2016PreVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)

    # years are this way because OHE is too sparse for gBDTs and 0123 is an 'unphysical' ordering which didn't outperform in early benchmarks
    alle1["year"] = 2
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = 0
    
    # now concat them together
    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)

if year != 'all':
    other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_{}_dilep_BDT.pkl'.format(year))
    other1["year"] = 1

else:
    other1 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2018_dilep_BDT.pkl')
    other2 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2017_dilep_BDT.pkl')
    other3 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2016PostVFP_dilep_BDT.pkl')
    other4 = pd.read_pickle('../ML_dataframes/trainsets/otherset_smallBDT_2016PreVFP_dilep_BDT.pkl')

    other1["year"] = 2
    other2["year"] = 1
    other3["year"] = 0
    other4["year"] = 0

    #now concat them together
    other1 = pd.concat([other1,other2,other3,other4], ignore_index=True)



# only keep positive weight training samples
alle1["region"] = "dilep"
alle1 = alle1[alle1["_weight"]>0]
#alle1 = alle1.replace({"class": dictio})

other1["region"] = "dilep"
other1 = other1[other1["_weight"]>0]
other1 = other1[other1["class"] != "TTW"]
#other1 = other1.replace({"class": dictio})

print(other1["class"].unique())
print(alle1["class"].unique())

#make other validation sets
X_other = other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region' ,'_leptonMVATOP_min',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
X_other.loc[X_other['class'] != 1, 'class'] = 0


# make training and testing sets
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region','_leptonMVATOP_min',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
X.loc[X['class'] == 'TTW', 'class'] = 1
X.loc[X['class'] == 'TTZ', 'class'] = 0
X.loc[X['class'] == 'TT', 'class'] = 2
X.loc[X['class'] == 'TTG', 'class'] = 3
X.loc[X['class'] == 'TTH', 'class'] = 4

classes = ["TTZ","TTW","TT","TTG","TTH"]

y_other = X_other['class']
y = X['class']
 
print("unique X_other classes")
print(X_other["class"].unique())
print("unique X classes")
print(X["class"].unique())

print("unique y_other classes")
print(y_other.unique())
print("unique y classes")
print(y.unique())


#split into train test, can use kfolds later
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20, random_state=13)

# calculate class imbalance
sums = list(X_train.groupby('class')["_weight"].sum())
print(sums)
class_imbalance_1 = 1#/(sums[0]/(sums[0]+sums[1]+sums[2]+sums[3]+sums[4]))
class_imbalance_2 = 1/(sums[1]/max([sums[0],sums[1],sums[2],sums[3],sums[4]]))
class_imbalance_3 = 1#/(sums[2]/(sums[0]+sums[1]+sums[2]+sums[3]+sums[4]))
class_imbalance_4 = 1#/(sums[3]/(sums[0]+sums[1]+sums[2]+sums[3]+sums[4]))
class_imbalance_5 = 1#/(sums[4]/(sums[0]+sums[1]+sums[2]+sums[3]+sums[4]))


print(class_imbalance_1)


# rescale training weights
X_train['_weight_balanced'] = X_train['_weight']
X_train.loc[X_train['class'] == 0, '_weight_balanced'] = 1 #class_imbalance_1
X_train.loc[X_train['class'] == 1, '_weight_balanced'] = class_imbalance_2
X_train.loc[X_train['class'] == 2, '_weight_balanced'] = 1 #class_imbalance_3
X_train.loc[X_train['class'] == 3, '_weight_balanced'] = 1 #class_imbalance_4
X_train.loc[X_train['class'] == 4, '_weight_balanced'] = 1 #class_imbalance_5


X_train['_weight_balanced'] = X_train['_weight_balanced'] * X_train['_weight']

weight_train = X_train['_weight']
weight_test = X_test['_weight']
weight_train_balanced = X_train['_weight_balanced']


# last unused features can be dropped
X_train = X_train.drop(['_weight', 'class','_weight_balanced'], axis = 1).astype(float)
#remove signal samples to only inject the test signal samples later on (otherwise bias
X_other = X_other[X_other["class"]!=1]
print("X_test classes")
print(X_test["class"])
X_other = pd.concat([X_other, X_test[X_test["class"]==1]],ignore_index=True)
y_other = pd.concat([y_other, y_test[X_test["class"]==1]],ignore_index=True)
X_test = X_test.drop(['_weight', 'class'], axis = 1)
weight_other =  X_other['_weight']

X_other = X_other.drop(['_weight', 'class'], axis = 1)
print("y_other")
print(y_other)
print("X_other")
print(X_other)

# last safety to modify classes to numerical
if (not y_train[y_train.isin(['TTRR','TTXRR'])].empty):
    print("some class was not converted!")
    print(y_train.unique())
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index)
    y_other = pd.Series(np.where(y_other.values == 'TTW', 1, 0),y_other.index)


    
# rename the features so we can save the model to TMVA
X.rename(columns=boostfeaturemap,inplace=True)
X_train.rename(columns=boostfeaturemap,inplace=True)
X_test.rename(columns=boostfeaturemap,inplace=True)
X_other.rename(columns=boostfeaturemap,inplace=True)


# safety for when you test with small samples
print("do we have signals?")
print(not y_train[y_train.isin([1])].empty)
print(y_other[y_other.isin([0])].empty)
print(y_other[y_other.isin([1])].empty)
print(not y_other[y_other.isin([2])].empty)



bst = XGBClassifier(n_estimators=n_estimators, max_depth=max_depth,num_class=5, learning_rate= lr, objective='objective=multi:softmax',n_jobs = 4)
print("start training")
bst.fit(X_train.to_numpy(), y_train.to_numpy(), sample_weight=weight_train_balanced.to_numpy(), eval_metric=["merror","mlogloss"],eval_set = [(X_train.to_numpy(), y_train.to_numpy()), (X_test.to_numpy(), y_test.to_numpy()), (X_other.to_numpy(), y_other.to_numpy())],sample_weight_eval_set=[weight_train,weight_test,weight_other])
print("done, plotting")


# set plot figure size
fig, ax = plt.subplots(1,1, figsize = (20, 20))
linewidths = [1,3,1,1,1]

def multiclass_roc_auc_score(y_tests, y_predicts,test_weight,colors, dataset,other=False, average="macro"):
    y_testmod = list(y_tests)
    y_testmod2 = list(y_tests)
    target = [0,1,2,3,4]
    if other:
        target = [1]
    print(target)
    for (idx, c_label) in enumerate(target):
        print("START AGAIN")
        print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
        print(idx)
        print(c_label)
        #print(y_testmod)
        #print(y_predicts[:,idx])
        for i in range(len(y_testmod)):
            if y_testmod2[i] == c_label:
                # x = 0
                y_testmod[i] = 1
            else:
                y_testmod[i] = 0

        print(y_testmod)
        #print(idx)
        fpr, tpr, thresholds = roc_curve(pd.Series(y_testmod), y_predicts[:,idx],sample_weight = test_weight)
        ax.plot(fpr, tpr, label = dataset + ' class ' + classes[idx]+ ' (AUC %0.2f)'  % (auc(fpr, tpr)), linewidth=linewidths[idx], color = colors[idx])

    return #roc_auc_score(y_testmod, y_predicts, average=average,sample_weight = test_weight, multi_class = "ovr")

y_pred_proba_test = bst.predict_proba(X_test)
y_pred_proba_train = bst.predict_proba(X_train)
y_pred_proba_other = bst.predict_proba(X_other)

multiclass_roc_auc_score(y_test,y_pred_proba_test,weight_test,["lightskyblue","cornflowerblue","blue","mediumblue","darkblue"] ,"test", other=False)
multiclass_roc_auc_score(y_train,y_pred_proba_train,weight_train, ["palegreen","limegreen","mediumseagreen","seagreen","darkgreen"], "train", other=False)
multiclass_roc_auc_score(y_other,y_pred_proba_other,weight_other, ["darkorange"], "other", other=True)

#make the plot
ax.text(0.05, 0.97, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=32,
        fontweight='bold')
ax.text(0.05, 0.94, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=23)

plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
ax.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/BDT/ROC_{}_splitmulticlass_weightmaxclass_20psmalltrain_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()


#make the loss plot. this still doesnt include the sample weights in the loss, so these are wrong
results = bst.evals_result()

epochs = len(results['validation_0']['merror'])
x_axis = range(0, epochs)

fig, ax = plt.subplots(1,1, figsize = (20, 20))
ax.plot(x_axis, results['validation_0']['merror'], label='Train')
ax.plot(x_axis, results['validation_1']['merror'], label='Test')
#ax.plot(x_axis, results['validation_2']['mlogloss'], label='Other')
ax.legend()
plt.xlabel('\nEpochs',fontsize=14,fontweight='semibold')
plt.ylabel('Error\n',fontsize=14,fontweight='semibold')
plt.title('XGBoost learning curve\n',fontsize=20,fontweight='semibold')
plt.savefig("/user/dmarckx/public_html/ML/BDT/merror_{}_splitmulticlass_weightmaxclass_20psmalltrain_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()


fig, ax = plt.subplots(1,1, figsize = (20, 20))
ax.plot(x_axis, results['validation_0']['mlogloss'], label='Train')
ax.plot(x_axis, results['validation_1']['mlogloss'], label='Test')
#ax.plot(x_axis, results['validation_2']['mlogloss'], label='Other')
ax.legend()
plt.xlabel('\nEpochs',fontsize=14,fontweight='semibold')
plt.ylabel('Log loss\n',fontsize=14,fontweight='semibold')
plt.title('XGBoost learning curve\n',fontsize=20,fontweight='semibold')
plt.savefig("/user/dmarckx/public_html/ML/BDT/loss_{}_splitmulticlass_weightmaxclass_20psmalltrain_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()


#save the model
file_name = "/user/dmarckx/ewkino/ML/models/XGB_{}_splitmulticlass_withbettergridsearchshort".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + ".pkl"

# save
pickle.dump(bst, open(file_name, "wb"))
ROOT.TMVA.Experimental.SaveXGBoost(bst, "XGB", "~/public/models/XGBsplitmulticlass_30features_lepMVA_removed_withbettergridsearchshort_{}.root".format(year))
