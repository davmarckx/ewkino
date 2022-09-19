#include <iostream>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"

void trainMvaMethods(const std::string leptonFlavor, const std::string algorithm){

    bool isMuon = (leptonFlavor == "muon");
    bool isElectron = (leptonFlavor == "electron");
    if(!isMuon && !isElectron){
        std::cerr << "no proper lepton flavor option given!" << std::endl;
        std::cerr << "aborting." << std::endl;
        return;
    }

    //output file
    TFile* outputFile = TFile::Open((const TString&) "trainingOutput_" + leptonFlavor + "_" + algorithm + ".root", "RECREATE" );
    TMVA::Factory *factory = new TMVA::Factory("TMVAClassification" + leptonFlavor + "_" + algorithm, outputFile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset" + leptonFlavor + "_" + algorithm);

    //add variables
    dataloader->AddVariable("pt", 'F');
    dataloader->AddVariable("eta", 'F');
    dataloader->AddVariable("trackMultClosestJet", 'F');
    dataloader->AddVariable("miniIsoCharged", 'F');
    dataloader->AddVariable("miniIsoNeutral", 'F');
    dataloader->AddVariable("pTRel", 'F');
    dataloader->AddVariable("ptRatio", 'F');
    dataloader->AddVariable("csvV2ClosestJet", 'F');
    dataloader->AddVariable("sip3d", 'F');
    dataloader->AddVariable("dxy", 'F');
    dataloader->AddVariable("dz", 'F');
    if(isMuon){
        dataloader->AddVariable("segmentCompatibility", 'F');
    } else if(isElectron){
        dataloader->AddVariable("electronMva", 'F');
    }

    //use correct event weights
    dataloader->SetSignalWeightExpression("eventWeight");
    dataloader->SetBackgroundWeightExpression("eventWeight");

    //get training trees
    TFile* file;
    if(isMuon){
        file =  new TFile("leptonMvaTrainingTree_muon.root");
    } else if(isElectron){
        file =  new TFile("leptonMvaTrainingTree_electron.root");
    }

    TTree* signalTree = (TTree*) (file->Get((const TString&) "signalTree") );
    TTree* backgroundTree = (TTree*) (file->Get((const TString&) "backgroundTree") );

    dataloader->AddSignalTree(signalTree, 1.);
    dataloader->AddBackgroundTree(backgroundTree, 1.);


    TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
    TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

    dataloader->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:NormMode=None:SplitMode=Random:!V" );
    //BDT 
    if(algorithm == "BDT"){
        factory->BookMethod( dataloader, TMVA::Types::kBDT, (const TString&) "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1_" + leptonFlavor, "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedBoost=True:DoBoostMonitor=True");
    }
    else if(algorithm == "MLP"){
        //NN
        factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP_default", "H:!V:NeuronType=tanh:VarTransform=Norm:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );
        factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP_doubleLayer", "H:!V:NeuronType=tanh:VarTransform=Norm:NCycles=600:HiddenLayers=N+5,N+5:TestRate=5:!UseRegulator" );
        factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP_bayesian", "H:!V:NeuronType=tanh:VarTransform=Norm:NCycles=600:HiddenLayers=N+5:TestRate=5:UseRegulator" );
        factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP_sigmoid", "H:!V:NeuronType=sigmoid:VarTransform=Norm:NCycles=600:HiddenLayers=N+5:TestRate=5:UseRegulator" );
    } else if(algorithm == "DNN"){
        // General layout.
        TString layoutString_default("Layout=TANH|128,TANH|128,TANH|128,LINEAR");
        TString layoutString_extraLayers("Layout=TANH|128,TANH|128,TANH|128,TANH|128,TANH|128,LINEAR");
        TString layoutString_double("Layout=TANH|256,TANH|256,TANH|256,LINEAR");
        TString layoutString_sigmoid("Layout=SIGMOID|128,SIGMOID|128,SIGMOID|128,LINEAR");
         
        // Training strategies.
        TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,"
        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
        "WeightDecay=1e-4,Regularization=L2,"
        "DropConfig=0.0+0.5+0.5+0.5, Multithreading=True");
        TString training1("LearningRate=1e-2,Momentum=0.9,Repetitions=1,"
        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
        "WeightDecay=1e-4,Regularization=L2,"
        "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
        TString training2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,"
        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
        "WeightDecay=1e-4,Regularization=L2,"
        "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
        TString trainingStrategyString ("TrainingStrategy=");
        trainingStrategyString += training0 + "|" + training1 + "|" + training2;
        
        // General Options.
        TString dnnOptions_default("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:WeightInitialization=XAVIERUNIFORM");
        TString dnnOptions_sumOfSquares("!H:V:ErrorStrategy=SUMOFSQUARES:VarTransform=N:WeightInitialization=XAVIERUNIFORM");
        TString dnnOptions_gaussianWeights("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:WeightInitialization=XAVIER");
        
        //combine the option strings
        //default
        TString optionsDefault = dnnOptions_default + ":" + layoutString_default + ":" + trainingStrategyString + ":Architecture=CPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_default", optionsDefault);

        //extralayers
        TString optionsExtraLayers = dnnOptions_default + ":" + layoutString_extraLayers + ":" + trainingStrategyString + ":Architecture=CPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_2extraLayers", optionsExtraLayers);

        //double 
        TString optionsDouble = dnnOptions_default + ":" + layoutString_double + ":" + trainingStrategyString + ":Architecture=CPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_doubleNodes", optionsDouble);

        //sigmoid
        TString optionsSigmoid = dnnOptions_default + ":" + layoutString_sigmoid + ":" + trainingStrategyString + ":Architecture=CPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_sigmoid", optionsSigmoid);
        
        // Multi-core CPU implementation.
        //TString cpuOptions = dnnOptions + ":Architecture=CPU";
        //factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN CPU", cpuOptions);
    }
    //train MVAs using the set of training events
    factory->TrainAllMethods();

    //evaluate all MVAs using the set of test events
    factory->TestAllMethods();

    //evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();

    //save the output
    outputFile->Close();

    delete factory;
    delete dataloader;
}
int main(int argc, char* argv[]){
    if(argc > 2) trainMvaMethods(argv[1], argv[2]);
    else trainMvaMethods("muon", "DNN");
    return 0;
}
