import uproot
import ROOT
import sys
# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, '/user/dmarckx/ewkino/ML/NN/')

#plotting packages
import seaborn as sns
import matplotlib.pyplot as plt
import math
sns.set_style("darkgrid", {'axes.edgecolor': 'black', 'xtick.bottom': True,'ytick.left': True, 'xtick.direction': 'inout','ytick.direction': 'inout'})

#math and data packages
import pandas as pd
import numpy as np
import scipy
import random

import sklearn
from itertools import cycle
from sklearn import tree
from sklearn.model_selection import train_test_split, KFold, RandomizedSearchCV, RepeatedStratifiedKFold, StratifiedKFold
from sklearn.metrics import mean_squared_error
from sklearn.metrics import r2_score
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler, PolynomialFeatures
from sklearn.pipeline import make_pipeline
from sklearn.metrics import classification_report, confusion_matrix
#from sklearn.metrics import ConfusionMatrixDisplay #plot_confusion_matrix
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
#from sklearn.metrics import RocCurveDisplay
from sklearn.model_selection import StratifiedKFold
from sklearn import __version__

print('sklearn version' + str(__version__))


from sklearn.decomposition import PCA
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis


# explicitly require this experimental feature
#from sklearn.experimental import enable_halving_search_cv # noqa
# now you can import normally from model_selection
#from sklearn.model_selection import HalvingGridSearchCV

fract = 1
boostfeaturemap = {'_abs_eta_recoil':'f1', '_Mjj_max':'f2', '_deepFlavor_max':'f3',
       '_deepFlavor_leading':'f4', '_deepFlavor_subLeading':'f5', '_lT':'f6', '_pTjj_max':'f7',
       '_dRlb_min':'f8', '_dRl1l2':'f9', '_HT':'f10', '_nJets':'f11', '_nBJets':'f12',
       '_dRlWrecoil':'f13', '_dRlWbtagged':'f14', '_M3l':'f15', '_abs_eta_max':'f16', '_MET_pt':'f17',
       #'_nMuons':'f18',
       '_leptonChargeLeading':'f19',
       '_leptonPtLeading':'f20', '_leptonPtSubLeading':'f21', '_leptonAbsEtaLeading':'f22',
       '_leptonAbsEtaSubLeading':'f23',
       '_jetPtLeading':'f24', '_jetPtSubLeading':'f25', '_jetAbsEtaLeading':'f26', '_jetAbsEtaSubLeading':'f27', '_jetMassLeading':'f28',
       '_jetMassSubLeading':'f29','_leptonMVATOP_max':'f30','_leptonMVATOP_min':'f18','year':'f32', 'flavorregion':'f33','_leptonMVAttH_min':'f34'}



alle1 = pd.read_pickle('ML_dataframes/lepMVAstudy/corrstudies_2018_corrstudies.pkl').sample(frac=fract, random_state=13)
alle2 = pd.read_pickle('ML_dataframes/lepMVAstudy/corrstudies_2017_corrstudies.pkl').sample(frac=fract, random_state=13)
alle3 = pd.read_pickle('ML_dataframes/lepMVAstudy/corrstudies_2016PostVFP_corrstudies.pkl').sample(frac=fract, random_state=13)
alle4 = pd.read_pickle('ML_dataframes/lepMVAstudy/corrstudies_2016PreVFP_corrstudies.pkl').sample(frac=fract, random_state=13)

# years are this way because OHE is too sparse for gBDTs and 0123 is an 'unphysical' ordering which didn't outperform in early benchmarks
alle1["year"] = 2
alle2["year"] = 1
alle3["year"] = 0
alle4["year"] = -1

# now concat them together
alle1 = pd.concat([alle1, alle2,alle3,alle4], ignore_index=True)
#alle1.rename(columns=boostfeaturemap,inplace=True)


MC = alle1[alle1["class"]!='Obs']
Data = alle1[alle1["class"]=='Obs']



MCinfo = MC.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','deltaPhiLeadingLeptonPair',
       '_bjetEtaLeading',"_leptonFlavor1","_leptonFlavor2","_dRl1jet","_leptonMVAttH_min","_nMuons",
       'mLeadingLeptonPair', '_nJetsNBJetsCat', '_nJetsNZCat','_leptonEtaLeading','_leptonEtaSubLeading',
       '_bjetPtLeading', '_bjetAbsEtaLeading', 'deltaEtaLeadingLeptonPair','_nLooseBJets','_nTightBJets',
       '_leptonMaxEta', '_jetEtaLeading', '_jetEtaSubLeading','_weight', '_reweight',
       '_leptonEtaTrailing','_leptonAbsEtaTrailing' ,'deltaRLeadingLeptonPair','_leptonMVATOP_lowestlep',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield','_leptonELeading',
       '_leptonESubLeading',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry','_leptonMVATOP_leading','_leptonMVATOP_subleading'], axis=1)

print("\n")
print(MCinfo.corr())



sort = MCinfo.corr().sort_values(by=['_eventBDT_maxonly'], ascending=False, axis=1).columns
tmp = MCinfo.loc[:,sort]
corrmat = MCinfo.corr().round(decimals=2)
f, ax = plt.subplots(figsize=(50,50))
sns.heatmap(corrmat, square=True, annot=True, cmap="RdBu", annot_kws={"size":30});
ax.set_title('Vector correlation of BDT features: MC', fontsize=60,fontweight="bold", y=1.05)
ax.text(0.0, 1.05, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=38,
        fontweight='bold',
        transform=ax.transAxes)
ax.text(0.0, 1.03, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=34,
        transform=ax.transAxes)
plt.xticks(fontsize= 30,rotation = 60, ha="right")
plt.yticks(fontsize= 30)
plt.savefig("corr_MC.png")


Datainfo = Data.drop(['_runNb', '_lumiBlock', '_eventNb', '_normweight','_dPhill_max','_MET_phi', '_nElectrons','_numberOfVertices',"_deepCSV_subLeading","_deepCSV_max","_deepCSV_leading",'_leptonChargeSubLeading',"_l1dxy","_l1dz","_l1sip3d","_l2dxy","_l2dz","_l2sip3d",'_lW_charge','_lW_pt','deltaPhiLeadingLeptonPair',
       '_bjetEtaLeading',"_leptonFlavor1","_leptonFlavor2","_dRl1jet","_leptonMVAttH_min","_nMuons",
       'mLeadingLeptonPair', '_nJetsNBJetsCat', '_nJetsNZCat','_leptonEtaLeading','_leptonEtaSubLeading',
       '_bjetPtLeading', '_bjetAbsEtaLeading', 'deltaEtaLeadingLeptonPair','_nLooseBJets','_nTightBJets',
       '_leptonMaxEta', '_jetEtaLeading', '_jetEtaSubLeading','_weight', '_reweight',
       '_leptonEtaTrailing','_leptonAbsEtaTrailing','deltaRLeadingLeptonPair','_leptonMVATOP_lowestlep',
       '_leptonreweight', '_nonleptonreweight', '_fakerateweight','_MT','_yield','_leptonELeading',
       '_leptonESubLeading',
       '_chargeflipweight','_fakeRateFlavour','_bestZMass', '_Z_pt','_leptonPtTrailing','_leptonEtaTrailing', '_lW_asymmetry','_leptonMVATOP_leading','_leptonMVATOP_subleading'], axis=1)

sort = Datainfo.corr().sort_values(by=['_eventBDT_maxonly'], ascending=False, axis=1).columns
tmp = Datainfo.loc[:,sort]
corrmat = Datainfo.corr().round(decimals=2)
f, ax = plt.subplots(figsize=(50,50))
sns.heatmap(corrmat, square=True, annot=True, cmap="RdBu", annot_kws={"size":40});
ax.set_title('Vector correlation of BDT features: Data', fontsize=60,fontweight="bold", y=1.05)
ax.text(0.0, 1.05, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=38,
        fontweight='bold',
        transform=ax.transAxes)
ax.text(0.0, 1.03, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=34,
        transform=ax.transAxes)
plt.xticks(fontsize= 30,rotation = 60, ha="right")
plt.yticks(fontsize= 30)
plt.savefig("corr_Data.png")






corrmat = (MCinfo.corr().round(decimals=2) / Datainfo.corr().round(decimals=2)).round(decimals=2)
f, ax = plt.subplots(figsize=(50,50))
sns.heatmap(corrmat, square=True, annot=True, cmap="RdBu", annot_kws={"size":40});
ax.set_title('Vector correlation of BDT features: MC Data Ratio', fontsize=60,fontweight="bold", y=1.05)
ax.text(0.0, 1.05, 'CMS',
        horizontalalignment='left',
        verticalalignment='top',
        fontsize=38,
        fontweight='bold',
        transform=ax.transAxes)
ax.text(0.0, 1.03, 'Simulation Internal',
        horizontalalignment='left',
        verticalalignment='top',
        fontstyle = 'italic',
        fontsize=34,
        transform=ax.transAxes)
plt.xticks(fontsize= 30,rotation = 60, ha="right")
plt.yticks(fontsize= 30)
plt.savefig("corr_ratioMonD.png")
