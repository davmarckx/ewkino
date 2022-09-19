#!/usr/bin/env python

from ROOT import TMVA, TFile, TTree, TCut
from subprocess import call
from os.path import isfile

from keras.models import Sequential
from keras.layers import Dense, Activation, Conv1D
from keras.regularizers import l2
from keras.regularizers import l1
from keras.optimizers import SGD

##EDIT##
from keras import optimizers
from keras.layers.advanced_activations import LeakyReLU, PReLU, ELU
from keras.layers import Dropout

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()

output = TFile.Open('TMVA.root', 'RECREATE')
factory = TMVA.Factory('TMVAClassification', output,
                       '!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

data = TFile.Open('leptonMvaTrainingTree_muon.root')
signal = data.Get('signalTree')
background = data.Get('backgroundTree')

dataloader = TMVA.DataLoader('dataset')
dataloader.AddVariable("pt")
dataloader.AddVariable("eta")
dataloader.AddVariable("trackMultClosestJet", 'F');
dataloader.AddVariable("miniIsoCharged", 'F');
dataloader.AddVariable("miniIsoNeutral", 'F');
dataloader.AddVariable("pTRel", 'F');
dataloader.AddVariable("ptRatio", 'F');
dataloader.AddVariable("relIso0p4", 'F');
dataloader.AddVariable("csvV2ClosestJet", 'F');
#dataloader.AddVariable("deepCsvClosestJet", 'F');
dataloader.AddVariable("sip3d", 'F');
dataloader.AddVariable("dxy", 'F');
dataloader.AddVariable("dz", 'F');
dataloader.AddVariable("segmentCompatibility", 'F');


dataloader.AddSignalTree(signal, 1.0)
dataloader.AddBackgroundTree(background, 1.0)

dataloader.SetSignalWeightExpression("eventWeight");
dataloader.SetBackgroundWeightExpression("eventWeight");

dataloader.PrepareTrainingAndTestTree(TCut('eventWeight>0'),
                            'nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V')

# Generate model

##EDIT##
#test leaky relu
#leakyRelu = layers.LeakyReLU(alpha=0.3)
########

# Define model
model = Sequential()
#model.add(Dense(128, activation='relu', W_regularizer=l2(1e-5), input_dim=12))
model.add(Dense(256, activation='relu', input_dim=12)) #12
#model.add(PReLU())
#model.add(Dropout(0.05))
model.add(Dense(256, activation='relu'))
#model.add(PReLU())
#model.add(Dropout(0.05))
model.add(Dense(256, activation='relu'))
#model.add(Dropout(0.05))
#model.add(PReLU())

#model.add(Dense(256, activation='relu', W_regularizer=l2(1e-5), input_dim=12))
#model.add(LeakyReLU(alpha=0.3),)
#model.add(Dense(256, activation='linear'))
#model.add(LeakyReLU(alpha=0.3))
#model.add(Dense(256, activation='linear'))
#model.add(LeakyReLU(alpha=0.3))
##EDIT##
#model.add(Dense(256, activation='relu'))
#model.add(Dense(256, activation='relu'))
########

model.add(Dense(2, activation='softmax'))
#model.add(Dense(1, activation='linear'))
#model.add(Dense(2, activation='sigmoid'))

# Set loss and optimizer
#model.compile(loss='categorical_crossentropy',

#test optimizer
sgd = optimizers.SGD(lr=0.01, decay=1e-6, momentum=0.9, nesterov=True)
#adam = optimizers.Adam(lr=0.01, beta_1=0.9, beta_2=0.999, epsilon=None, decay=0.0)
adam = optimizers.Adam()
#sgd = optimizers.SGD(lr=0.01, decay=1e-6, momentum=0.9, nesterov=False)
#sgd = optimizers.SGD(lr=0.003, decay=1e-6, momentum=0.5, nesterov=True)
###

model.compile(
              loss='binary_crossentropy',
              #loss='cosine_proximity',
#              optimizer=SGD(lr=0.01), metrics=['accuracy', ])
#              optimizer=sgd, metrics=['accuracy', ])
#              optimizer=adam, metrics=['accuracy', ])
#              optimizer=optimizers.RMSprop(), metrics=['accuracy', ])
#              optimizer=optimizers.Adagrad(), metrics=['accuracy', ])
#              optimizer=optimizers.Adadelta(), metrics=['accuracy', ])
#              optimizer=optimizers.Adamax(), metrics=['accuracy', ])
              optimizer=optimizers.Nadam(), metrics=['accuracy', ])

# Store model to file
model.save('model_classification.h5')
model.summary()

# Book methods
factory.BookMethod(dataloader, TMVA.Types.kFisher, 'Fisher',
                   '!H:!V:Fisher')
factory.BookMethod(dataloader, TMVA.Types.kPyKeras, 'kerasDNN',
        'H:!V:VarTransform=D,G:FilenameModel=model_classification.h5:NumEpochs=20:BatchSize=64:') #default NumEpochs = 20  D,G VarTransform=D,G

#factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDTG_m1Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1',
#        '!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True');

# Run training, test and evaluation
factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()
