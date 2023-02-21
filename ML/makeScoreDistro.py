import uproot
import ROOT
import sys
# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, '/user/dmarckx/ewkino/ML/NN/')

import torch
from torch_geometric.data import Data
import torch_geometric
import networkx as nx

from Networks import *

import threading


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



#plotting packages and set nice seaborn plotting style
import seaborn as sns
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})
import matplotlib.pyplot as plt


print('sklearn version' + str(__version__))

# load datasets

# Opening JSON file that contains the renaming
with open('/user/dmarckx/ewkino/ttWAnalysis/eventselection/processes/rename_processes.json') as json_file:
    dictio = json.load(json_file)

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


isTorch = False
fract = 1
nr_events = 100
year = "2016PreVFP"

print("load datasets")
#load dataset######################################
if year == "all" and isTorch:
    if nr_events > 0:
        traindata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtrainset_2018_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphotherset_2018_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtrainset_2017_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphotherset_2017_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtrainset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphotherset_2016PostVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

        traindata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtrainset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)
        testdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtestset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),nr_events)
        otherdata += random.sample(pd.read_pickle('ML_dataframes/trainsets/graphotherset_2016PreVFP_{}_smallGNN.pkl'.format(sparse)),4*nr_events)

    else:
        print("train")
        thread1 = myThread(1, "Thread-1", 'ML_dataframes/trainsets/graphtrainset_2018_{}_smallGNN.pkl'.format(sparse))
        #traindata = pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2018_{}_smallGNN.pkl'.format(sparse))
        print("test")
        thread2 = myThread(2, "Thread-2", 'ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse))
        #testdata = pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse))
        print("other")
        thread3 = myThread(3, "Thread-3", 'ML_dataframes/trainsets/graphtestset_2018_{}_smallGNN.pkl'.format(sparse))
        #otherdata = pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2018_{}_smallGNN.pkl'.format(sparse))
        print("2018 done")
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata = thread1.data
        testdata = thread2.data
        otherdata = thread3.data

        thread1 = myThread(1, "Thread-1", 'ML_dataframes/trainsets/graphtrainset_2017_{}_smallGNN.pkl'.format(sparse))
        #traindata += pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2017_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", 'ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse))
        #testdata += pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", 'ML_dataframes/trainsets/graphtestset_2017_{}_smallGNN.pkl'.format(sparse))
        #otherdata += pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2017_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data
        print("2017")

        thread1 = myThread(1, "Thread-1", 'ML_dataframes/trainsets/graphtrainset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        #traindata += pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", 'ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        #testdata += pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", 'ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        #otherdata += pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data
        print("2016")

        thread1 = myThread(1, "Thread-1", 'ML_dataframes/trainsets/graphtrainset_2016PreVFP_{}_smallGNN.pkl'.format(sparse))
        #traindata += pd.read_pickle('../ML_dataframes/trainsets/graphtrainset_2016PreVFP_{}_smallGNN.pkl'.format(sparse))
        thread2 = myThread(2, "Thread-2", 'ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        #testdata += pd.read_pickle('../ML_dataframes/trainsets/graphtestset_2016PreVFP_{}_smallGNN.pkl'.format(sparse))
        thread3 = myThread(3, "Thread-3", 'ML_dataframes/trainsets/graphtestset_2016PostVFP_{}_smallGNN.pkl'.format(sparse))
        #otherdata += pd.read_pickle('../ML_dataframes/trainsets/graphotherset_2016PreVFP_{}_smallGNN.pkl'.format(sparse))
        thread1.start()
        thread2.start()
        thread3.start()
        thread1.join()
        thread2.join()
        thread3.join()
        traindata += thread1.data
        testdata += thread2.data
        otherdata += thread3.data

elif isTorch:
    if nr_events > 0:
        traindata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse)),4*nr_events)
        testdata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse)),nr_events)
        otherdata = random.sample(pd.read_pickle('ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse)),4*nr_events)
    else:
        traindata = pd.read_pickle('ML_dataframes/trainsets/graphtrainset_{}_{}_smallGNN.pkl'.format(year, sparse))
        testdata = pd.read_pickle('ML_dataframes/trainsets/graphtestset_{}_{}_smallGNN.pkl'.format(year, sparse))
        otherdata = pd.read_pickle('ML_dataframes/trainsets/graphotherset_{}_{}_smallGNN.pkl'.format(year, sparse))

elif year != 'all' and not isTorch:
    alle1 = pd.read_pickle('ML_dataframes/trainsets/trainset_smallBDTnew_{}_dilep_BDT.pkl'.format(year)).sample(frac=fract, random_state=13)
    alle1["year"] = 1


elif not isTorch:
    alle1 = pd.read_pickle('ML_dataframes/trainsets/trainset_smallBDTnew_2018_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle2 = pd.read_pickle('ML_dataframes/trainsets/trainset_smallBDTnew_2017_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle3 = pd.read_pickle('ML_dataframes/trainsets/trainset_smallBDTnew_2016PostVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)
    alle4 = pd.read_pickle('ML_dataframes/trainsets/trainset_smallBDTnew_2016PreVFP_dilep_BDT.pkl').sample(frac=fract, random_state=13)

    # years are this way because OHE is too sparse for gBDTs and 0123 is an 'unphysical' ordering which didn't outperform in early benchmarks
    alle1["year"] = 1
    alle2["year"] = 1
    alle3["year"] = 0
    alle4["year"] = 0

    # now concat them together
    alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)

if year != 'all' and not isTorch:
    other1 = pd.read_pickle('ML_dataframes/trainsets/otherset_smallBDTnew_2018_dilep_BDT.pkl')
    other1["year"] = 1

elif not isTorch:
    other1 = pd.read_pickle('ML_dataframes/trainsets/otherset_smallBDTnew_2018_dilep_BDT.pkl')
    other2 = pd.read_pickle('ML_dataframes/trainsets/otherset_smallBDTnew_2017_dilep_BDT.pkl')
    other3 = pd.read_pickle('ML_dataframes/trainsets/otherset_smallBDTnew_2016PostVFP_dilep_BDT.pkl')
    other4 = pd.read_pickle('ML_dataframes/trainsets/otherset_smallBDTnew_2016PreVFP_dilep_BDT.pkl')

    other1["year"] = 1
    other2["year"] = 1
    other3["year"] = 0
    other4["year"] = 0

    #now concat them together
    other1 = pd.concat([other1,other2,other3,other4], ignore_index=True)
    
# load model
if isTorch:
    dropval = 0.25
    nheads = 4
    self_loops = True
    model = GCN(dropval, traindata[0],nheads,self_loops)
    
    model.load_state_dict(torch.load("models/trained_newfeatures_GATsmalltrainnewfeatures_withgraphnormalization_allfullyconnected5GCN+_drop25_20_0.001_(0,1)50.sav"))
else:
    # open a file, where you stored the xgb weights
    file = open('models/XGB_newbackgrd_all_final_withbettergridsearchshort30_4000_3_0.05.pkl', 'rb')

    # dump information to that file
    model = pickle.load(file)
    alle1.rename(columns=boostfeaturemap,inplace=True)
    y = alle1["class"]
    other1.rename(columns=boostfeaturemap,inplace=True)
    y_other = other1["class"]
    X = alle1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",
       '_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt', '_weight', 'class',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield','_leptonMVATOP_min',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)
    X_other = other1.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_eventBDT','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",
       '_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt', '_weight', 'class',
       '_leptonMVAttH_min','_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield','_leptonMVATOP_min',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry'], axis=1)



alle1["y_pred_proba"] = model.predict_proba(X)[::,1]
other1["y_pred_proba"] = model.predict_proba(X_other)[::,1]



alle1 = pd.concat([alle1, other1], ignore_index=True)
#print(y_pred_proba)

hueorder = y.unique()

f, ax = plt.subplots(figsize=(10,10))
line1 =sns.histplot(data=alle1, x= alle1['y_pred_proba'] , bins=50, binrange = (0,1),fill=True, weights=alle1["_weight"],hue_order=hueorder, hue='class', common_norm=True,legend=True,linewidth=2, common_bins=True, stat='density', alpha=0.1, element="step")

plt.rc('legend', fontsize=15)
ax.set_title(r'BDT output', fontsize=25,fontweight="bold")
ax.set_xlabel(r'signal output node', fontsize=25)#r'
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
plt.savefig("testje3.png")
