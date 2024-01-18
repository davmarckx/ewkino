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
from typing import Tuple
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
balance = float(sys.argv[5])
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

#make other validation sets '_deepFlavor_max','_deepFlavor_leading','_deepFlavor_subLeading',
X_other = other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','_leptonMVATOP_min','nominal',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry','_reweight','_leptonAbsEtaLeading', '_leptonAbsEtaSubLeading','_leptonAbsEtaTrailing','_leptonMaxEta','_jetAbsEtaLeading', '_jetAbsEtaSubLeading','_bjetAbsEtaLeading', 'deltaPhiLeadingLeptonPair','deltaRLeadingLeptonPair','mLeadingLeptonPair', '_nJetsNBJetsCat', '_nJetsNZCat'], axis=1)

#X_other.loc[X_other['class'] == 'TTW', 'class'] = 1
#X_other.loc[X_other['class'] != 1, 'class'] = 0

# make training and testing sets '_deepFlavor_max','_deepFlavor_leading','_deepFlavor_subLeading',
X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','_leptonMVATOP_min','nominal',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield', 'region',
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

steps = np.array([200,325,500,800])
HT_train = X_train['_HT']
#HT_train.clip(upper=800)
#.apply(lambda x: steps[np.argmin(np.abs(x-steps))])
HT_test = X_test['_HT']
#HT_test.clip(upper=800)
#.apply(lambda x: steps[np.argmin(np.abs(x-steps))])
HT_other = X_other['_HT']
#HT_other.clip(upper=800)#.apply(lambda x: steps[np.argmin(np.abs(x-steps))])


# last unused features can be dropped
X_train = X_train.drop(['_weight', 'class','_weight_balanced'], axis = 1)
X_other = pd.concat([X_other, X_test[X_test["class"]==1]],ignore_index=True)
X_test = X_test.drop(['_weight', 'class'], axis = 1)

y_other = X_other['class'].astype(int)
weight_other =  X_other['_weight']
X_other = X_other.drop(['_weight', 'class'], axis = 1)


# last safety to modify classes to numerical
if (not y_train[y_train.isin(['TTW','TTX'])].empty):
    print("some feature was not ")
    y_train = pd.Series(np.where(y_train.values == 'TTW', 1, 0),y_train.index).astype(int)
    y_test = pd.Series(np.where(y_test.values == 'TTW', 1, 0),y_test.index).astype(int)
    y = pd.Series(np.where(y.values == 'TTW', 1, 0),y.index).astype(int)
    
# define globally what HT is for signals
HT_train_signal = HT_train[y_train == 1]

# rename the features so we can save the model to TMVA
X.rename(columns=boostfeaturemap,inplace=True)
X_train.rename(columns=boostfeaturemap,inplace=True)
X_test.rename(columns=boostfeaturemap,inplace=True)
X_other.rename(columns=boostfeaturemap,inplace=True)


# safety for when you test with small samples
print("do we have signals?")
print(not y_train[y_train.isin([1])].empty)



# returns the losses of the current bdt output bin and the ones next to it
htbins = [250,400,600]
def getlosses(xi, hti,binvalsmap):
 #get bdt bin index
 index = int(xi/0.2)

 # get ht bin index
 name = ''
 for i in range(3):
  if hti < htbins[i]:
   name = "ht"+str(i+1)
   break
  else:
   name = "ht4"
   
 lijst = binvalsmap[name]

  
 if index == 0:
    return 99, lijst[0], lijst[1]
 elif index == 4:
    return lijst[3], lijst[4],99
 else:
    return lijst[index-1], lijst[index], lijst[index+1]
  


def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def FlatnessLoss(x, HT, binvalsmap,labels):
  # predicted bin, HT, map that has all bin loss values for all HTs

  # start with zeros list
  lijst = np.zeros(len(x))
  for i in range(len(lijst)):

   # no loss needed for backgrounds
   if labels[i] == 0:
    continue

   # look for losses of the bins next to current bin
   loss1,loss2,loss3 = getlosses(x[i], HT[i],binvalsmap)
 

   # if this loss is the lowest, no transport is needed (we want to keep as much as possible)
   if loss2 <= loss1 and loss2 <= loss3:
    continue

   # if loss1 is smaller than loss 3, we need to transport to 1 (NEGATIVE GRADIENT in the end)
   if loss1 < loss3:
    lijst[i] = abs(loss2-loss1)
    continue

   # if loss1 is BIGGER than loss 3, we need to transport to 3 (POSITIVE GRADIENT in the end)
   if loss3 < loss1:
    lijst[i] -= abs(loss3-loss2)
    continue    

   print("WARNING, SOMEHOW THE LIST OFF GRADIENTS WASNT UPDATED BECAUSE THERE IS A LEAK")
   
  # now go over list again and boost backgrounds to lower values to remedy shape collapse
  balancesum = sum(abs(number) for number in lijst) / sum(labels)


  for i in range(len(lijst)):
    if labels[i] == 0:
      lijst[i] = balancesum/2

  return lijst
    

def logloss(predt: np.ndarray, dtrain: xgb.DMatrix) -> Tuple[np.ndarray, np.ndarray]:

    def gradient(p: np.ndarray, dtrain: xgb.DMatrix) -> np.ndarray:
        p = sigmoid(p)
        y = dtrain.get_label()
        return p - y

    def hessian(p: np.ndarray, dtrain: xgb.DMatrix) -> np.ndarray:
        p = sigmoid(p)
        y = dtrain.get_label()
        eps = 1e-16
        hess = p * (1 - p)
        hess[hess < eps] = eps
        return hess

    #print(predt)
    #predt[predt < -1] = -1 + 1e-6
    grad = gradient(predt, dtrain)                #abs(delta HT*delta y)
    '''
    if len(predt) == len(X_train):
      grad += balance*sigmoid(predt)**2 *(dtrain.get_label())*(HT_train-HT_train.mean()) / HT_train.mean()
      #grad += balance*(dtrain.get_label())*(HT_train-HT_train.mean()) / HT_train.mean()
    elif len(predt) == len(X_test):
      grad += balance*sigmoid(predt)**2 *(dtest.get_label()-dtest.get_label().mean())*(HT_test-HT_test.mean())**2 / abs((dtest.get_label()-dtest.get_label().mean())*(HT_test-HT_test.mean())*HT_test.mean())
      #grad += balance*(dtrain.get_label())*(HT_test-HT_test.mean()) / HT_test.mean()
    elif len(predt) == len(X_other):
      #grad += balance*(dother.get_label()-dother.get_label().mean())*(HT_other-HT_other.mean())**2 / abs((dother.get_label()-dother.get_label().mean())*(HT_other-HT_other.mean())*HT_other.mean())
      grad += balance*sigmoid(predt)**2 *(dtrain.get_label())*(HT_other-HT_other.mean()) / HT_other.mean()
    else:
      #grad += balance*(dtrain.get_label()-dtrain.get_label().mean())*(HT_train-HT_train.mean())**2 / abs((dtrain.get_label()-dtrain.get_label().mean())*(HT_train-HT_train.mean())*HT_train.mean())
      grad += balance*sigmoid(predt)**2 *(dtrain.get_label())*(HT_train-HT_train.mean()) / HT_train.mean()
    '''

    #try new thing based on SDE from https://iopscience.iop.org/article/10.1088/1748-0221/10/03/T03002/pdf, we work with the bins used in the fit so we first transform:
    tfm_predicts_all = 1 /( 1 + np.exp(-4*(2*sigmoid(predt)-1)))  
 
    tfm_predicts_all = pd.Series(tfm_predicts_all,index=y_train.index)
    tfm_predicts = tfm_predicts_all[y_train == 1]

    print(tfm_predicts)

    print("___")
    print(len(tfm_predicts_all))
    print(len(tfm_predicts))
    print(len(y_train[y_train == 1]))
    print("___")

    # get predicted values per HT bin (assumed to be: [0.0, 250.0, 400.0, 600.0, 1000.0])
    tfm_predicts_ht1 = tfm_predicts[HT_train_signal < 250]
    tfm_predicts_ht2 = tfm_predicts[(HT_train_signal > 250) & (HT_train_signal < 400)]
    tfm_predicts_ht3 = tfm_predicts[(HT_train_signal > 400) & (HT_train_signal < 600)]
    tfm_predicts_ht4 = tfm_predicts[HT_train_signal > 600]



    # first calculate which bins need more content, start calculating each bincontent
    total = len(tfm_predicts)
    bin1 = (tfm_predicts < 0.2).sum() / total
    bin2 = ((tfm_predicts > 0.2) & (tfm_predicts < 0.4)).sum() / total
    bin3 = ((tfm_predicts > 0.4) & (tfm_predicts < 0.6)).sum() / total
    bin4 = ((tfm_predicts > 0.6) & (tfm_predicts < 0.8)).sum() / total
    bin5 = (tfm_predicts > 0.8).sum() / total

    ht1total = len(tfm_predicts_ht1)
    ht1bin1 = (tfm_predicts_ht1 < 0.2).sum() / ht1total - bin1
    ht1bin2 = ((tfm_predicts_ht1 > 0.2) & (tfm_predicts_ht1 < 0.4)).sum() / ht1total - bin2
    ht1bin3 = ((tfm_predicts_ht1 > 0.4) & (tfm_predicts_ht1 < 0.6)).sum() / ht1total - bin3
    ht1bin4 = ((tfm_predicts_ht1 > 0.6) & (tfm_predicts_ht1 < 0.8)).sum() / ht1total - bin4
    ht1bin5 = (tfm_predicts_ht1 > 0.8).sum() / ht1total - bin5
    

    ht2total = len(tfm_predicts_ht2)
    ht2bin1 = ( tfm_predicts_ht2 < 0.2).sum() / ht2total - bin1
    ht2bin2 = ((tfm_predicts_ht2 > 0.2) & (tfm_predicts_ht2 < 0.4)).sum() / ht2total - bin2
    ht2bin3 = ((tfm_predicts_ht2 > 0.4) & (tfm_predicts_ht2 < 0.6)).sum() / ht2total - bin3
    ht2bin4 = ((tfm_predicts_ht2 > 0.6) & (tfm_predicts_ht2 < 0.8)).sum() / ht2total - bin4
    ht2bin5 = ( tfm_predicts_ht2 > 0.8).sum() / ht2total - bin5

    ht3total = len(tfm_predicts_ht3)
    ht3bin1 = (tfm_predicts_ht3 < 0.2).sum() / ht3total - bin1
    ht3bin2 = ((tfm_predicts_ht3 > 0.2) & (tfm_predicts_ht3 < 0.4)).sum() / ht3total - bin2
    ht3bin3 = ((tfm_predicts_ht3 > 0.4) & (tfm_predicts_ht3 < 0.6)).sum() / ht3total - bin3
    ht3bin4 = ((tfm_predicts_ht3 > 0.6) & (tfm_predicts_ht3 < 0.8)).sum() / ht3total - bin4
    ht3bin5 = (tfm_predicts_ht3 > 0.8).sum() / ht3total - bin5

    ht4total = len(tfm_predicts_ht4)
    ht4bin1 = (tfm_predicts_ht4 < 0.2).sum() / ht4total - bin1
    ht4bin2 = ((tfm_predicts_ht4 > 0.2) & (tfm_predicts_ht4 < 0.4)).sum() / ht4total - bin2
    ht4bin3 = ((tfm_predicts_ht4 > 0.4) & (tfm_predicts_ht4 < 0.6)).sum() / ht4total - bin3
    ht4bin4 = ((tfm_predicts_ht4 > 0.6) & (tfm_predicts_ht4 < 0.8)).sum() / ht4total - bin4
    ht4bin5 = (tfm_predicts_ht4 > 0.8).sum() / ht4total - bin5

    weights = [1.,1.,1.,8.]
    lossdict = {
    "ht1": [x*weights[0] for x in [ht1bin1,ht1bin2,ht1bin3,ht1bin4,ht1bin5]],
    "ht2": [x*weights[1] for x in [ht1bin1,ht2bin2,ht2bin3,ht2bin4,ht2bin5]],
    "ht3": [x*weights[2] for x in [ht3bin1,ht3bin2,ht3bin3,ht3bin4,ht3bin5]],
    "ht4": [x*weights[3] for x in [ht4bin1,ht4bin2,ht4bin3,ht4bin4,ht4bin5]],
    "total": [bin1,bin2,bin3,bin4,bin5],
    "httotal": [ht1total,ht2total,ht3total,ht4total],
    }

    print("current bin status")
    print(ht4bin1)
    print(ht4bin2)
    print(ht4bin3)
    print(ht4bin4)
    print(ht4bin5)
    # now that we calculated which bins need more content, we define the loss gradient
    if len(predt) == len(X_train):
      extraloss = FlatnessLoss(np.array(tfm_predicts_all), np.array(HT_train), lossdict, np.array(y_train))
      extraloss = [balance*x for x in extraloss]
      grad = [sum(x) for x in zip(grad, extraloss)]
    else:
      grad += 0

    hess = hessian(predt, dtrain) #hessian has no extra terms
    return grad, hess

def my_eval(predt,dmat):
    y=dmat.get_label() if isinstance(dmat,xgb.DMatrix) else dmat
    predt=np.clip(predt,10e-7,1-10e-7)
    return 'logloss_myerror', - np.mean(y * np.log(predt) + (1-y) * np.log(1-predt))

#bst = XGBClassifier(n_estimators=n_estimators, max_depth=max_depth, learning_rate= lr, objective=loglikelood,n_jobs = 4)
print("start training")
print(X_train.columns)
print(X_train)
print(X_train.iloc[0])
print(X_test.iloc[0])
print(X_other.iloc[0])

print("==")
print(y_train.astype(int).unique())
print("==")
print(y_test.astype(int).unique())
print("==")
print(y_other.astype(int).unique())
dtrain = xgb.DMatrix(X_train.astype(float).to_numpy(), label=y_train.astype(int).to_numpy(), weight=weight_train.astype(float).to_numpy())
dtest = xgb.DMatrix(X_test.astype(float).to_numpy(), label=y_test.astype(int).to_numpy(), weight=weight_test.astype(float).to_numpy())
dother = xgb.DMatrix(X_other.astype(float).to_numpy(), label=y_other.astype(int).to_numpy(), weight=weight_other.astype(float).to_numpy())
cpu_res = {}
og_res = {}

#pretrained model
bst2 = xgb.train({'max_depth':max_depth, 'eta':lr,'objective':'binary:logistic','seed':13,'eval_metric':'auc'},  # any other tree method is fine.
           dtrain=dtrain,
           num_boost_round=3*int(n_estimators/4),
           evals=[(dtest,'test'), (dtrain,'train'),(dother,'other')], evals_result=og_res)

#second model
bst = xgb.train({'max_depth':max_depth, 'eta':lr,'seed':13,'eval_metric':'auc'},  # any other tree method is fine.
           dtrain=dtrain,
           xgb_model=bst2,
           num_boost_round=int(n_estimators/4),
           obj=logloss, evals=[(dtest,'test'), (dtrain,'train'),(dother,'other')], evals_result=cpu_res)


#bst2 = xgb.train({'max_depth':max_depth, 'eta':lr,'objective':'binary:logistic','seed':13,'eval_metric':'auc','base_score':0.01},  # any other tree method is fine.
#           dtrain=dtrain,
#           num_boost_round=n_estimators,
#           evals=[(dtest,'test'), (dtrain,'train'),(dother,'other')], evals_result=og_res)


print(bst.predict(data=dtest))
print("XGB implementation:")
#print(bst2.predict(data=dtest))
print(bst.predict(data=dtrain))
print("XGB implementation:")
#print(bst2.predict(data=dtrain))
#bst.fit(X_train.astype(float).to_numpy(), y_train.astype(int).to_numpy(), sample_weight=weight_train_balanced.astype(float).to_numpy(),eval_set = [(X_train.astype(float).to_numpy(), y_train.astype(int).to_numpy()), (X_test.astype(float).to_numpy(), y_test.astype(int).to_numpy()), (X_other.astype(float).to_numpy(), y_other.astype(int).to_numpy())])
print("done, plotting")


# calculations for ROC
y_pred_proba_orig = bst.predict(data=dtest)#.predict_proba(X_test)[::,1]
fpr_orig, tpr_orig, _ = metrics.roc_curve(y_test.astype(int), y_pred_proba_orig.astype(float), sample_weight = weight_test.astype(float))
aucval_orig = metrics.roc_auc_score(y_test.astype(int), y_pred_proba_orig.astype(float), sample_weight = weight_test.astype(float))

y_pred_proba = bst.predict(data=dtrain)#_proba(X_train)[::,1]
fpr, tpr, _ = metrics.roc_curve(y_train.astype(int),  y_pred_proba.astype(float), sample_weight = weight_train.astype(float))
aucval = metrics.roc_auc_score(y_train.astype(int), y_pred_proba.astype(float), sample_weight = weight_train.astype(float))

y_pred_proba_other = bst.predict(data=dother)#_proba(X_other)[::,1]
fpr_other, tpr_other, _ = metrics.roc_curve(y_other.astype(int),  y_pred_proba_other.astype(float), sample_weight = weight_other.astype(float))
aucval_other = metrics.roc_auc_score(y_other.astype(int), y_pred_proba_other.astype(float), sample_weight = weight_other.astype(float))

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

plt.plot(fpr_orig,tpr_orig,color="red",label="test auc="+str("{:.2f}".format(aucval_orig)))
plt.plot(fpr,tpr,color="darkorange",label="train auc="+str("{:.2f}".format(aucval)))
plt.plot(fpr_other,tpr_other,color="green",label="other backgrounds vs test signal auc="+str("{:.2f}".format(aucval_other)))
plt.plot([0, 1], [0, 1], color="navy", linestyle="--")
plt.xlabel("False Positive Rate", fontsize=30)
plt.ylabel("True Positive Rate", fontsize=30)
plt.title("Receiver operating characteristic", fontsize=40)
plt.legend(loc="lower right", fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
now = datetime.now()
dt_string = now.strftime("%d-%m-%Y_%H-%M-%S")
plt.savefig("/user/dmarckx/public_html/ML/BDT/ROC_{}_uGBFL_fixed_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + "_" + str(balance) + dt_string +'_' + str(balance) +  ".png")
plt.close()


#make the loss plot. this still doesnt include the sample weights in the loss, so these are wrong
"""results = cpu_res #bst.evals_result()

epochs = len(results['validation_0']['logloss'])
x_axis = range(0, epochs)

fig, ax = plt.subplots()
ax.plot(x_axis, results['validation_0']['logloss'], label='Train')
ax.plot(x_axis, results['validation_1']['logloss'], label='Test')
#ax.plot(x_axis, results['validation_2']['logloss'], label='Other')
ax.legend()
plt.xlabel('\nEpochs',fontsize=14,fontweight='semibold')
plt.ylabel('Error\n',fontsize=14,fontweight='semibold')
plt.title('XGBoost learning curve\n',fontsize=20,fontweight='semibold')
plt.savefig("/user/dmarckx/public_html/ML/BDT/loss_{}_decorHT_".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + dt_string + ".png")
plt.close()

print(list(X_train.columns))
"""
#save the model
file_name = "/user/dmarckx/ewkino/ML/models/XGB_{}_uGBFL_fixed".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) +'_' + str(balance) +  ".pkl"

# save
pickle.dump(bst, open(file_name, "wb"))

xgb_classifier = xgb.XGBClassifier()
xgb_classifier._Booster = bst
xgb_classifier.max_depth=max_depth
xgb_classifier.n_estimators=n_estimators

ROOT.TMVA.Experimental.SaveXGBoost(bst, "XGB", "../models/XGB_{}_uGBFL_fixed".format(year) + str(len(X_train.columns)) + "_" + str(n_estimators) + "_" + str(max_depth) + "_" + str(lr) + '_' + str(balance) + ".root")

