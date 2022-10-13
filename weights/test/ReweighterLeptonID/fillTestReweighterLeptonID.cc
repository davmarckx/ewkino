/*
Test script for lepton ID reweighter.
*/

// import lepton ID reweighter
#include "../../interface/LeptonReweighter.h"
#include "../../interface/ReweighterLeptons.h"
#include "../../interface/ConcreteSelection.h"
#include "../../interface/ConcreteLeptonReweighter.h"
#include "../../interface/ConcreteReweighterLeptons.h"
#include "../../interface/CombinedReweighter.h"

// include other parts of the framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../Tools/interface/Sample.h"
#include "../../../Tools/interface/HistInfo.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/histogramTools.h"


int main( int argc, char* argv[] ){

    int nargs = 4;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: fillTestReweighterLeptonID.cc requires " << nargs;
	std::cerr << " arguments to run:" << std::endl;
	std::cerr << "- directory of input file(s)" << std::endl;
        std::cerr << "- name of input file (.root) OR samplelist (.txt)" << std::endl;
	std::cerr << "- name of output file" << std::endl;
	std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputDirectory = argvStr[1];
    std::string& sampleList = argvStr[2];
    std::string& outputFileName = argvStr[3];
    long unsigned nEvents = std::stoul(argvStr[4]);

    // read the input file
    TreeReader treeReader;
    std::vector<Sample> samples;
    bool modeSampleList = false;
    // case 1: single input file
    if( stringTools::stringEndsWith(sampleList, ".root") ){
	std::string inputFilePath = stringTools::formatDirectoryName(inputDirectory)+sampleList;
	treeReader.initSampleFromFile( inputFilePath );
	samples.push_back( treeReader.currentSample() );
    }
    // case 2: samplelist
    else if( stringTools::stringEndsWith(sampleList, ".txt") ){
	treeReader.readSamples( sampleList, inputDirectory );
	samples = treeReader.sampleVector();
	modeSampleList = true;
    }
    std::cout << "will use the following samples:" << std::endl;
    for( Sample sample: samples ) std::cout << "- " << sample.fileName() << std::endl;

    // initialize year from first sample
    // note: in this simple test no check is done to assure all samples in the list are of same year!
    if( modeSampleList ) treeReader.initSample();
    std::string year = treeReader.getYearString();

    // initialize some histograms
    HistInfo histInfo = HistInfo( "", "Leading lepton pT", 50, 0., 250. );
    std::map< std::string, std::map< std::string, std::shared_ptr<TH1D> >> histograms;
    std::vector< std::string > snapshots = {"unweightedpreselection",
					    "unweightedpostselection",
					    "weightedpreselection",
					    "weightedpostselection"};
    std::vector<std::string> variations = { "muonsyst", "muonstat", "electronsyst", "electronstat" };
    for( std::string snapshot: snapshots ){
	histograms[snapshot]["nominal"] = histInfo.makeHist( snapshot + "_nominal" );
	for( std::string var: variations ){
	    histograms[snapshot][var+"Up"] = histInfo.makeHist( snapshot+"_"+var+"Up" );
	    histograms[snapshot][var+"Down"] = histInfo.makeHist( snapshot+"_"+var+"Down" );
	}
    }
    
    // make the lepton ID reweighter (copy from with some naming modifications from ConcreteReweighterFactory)
    CombinedReweighter combinedReweighter;
    // make muon ID reweighter
    std::string muonSFFileName = std::string("../../weightFilesUL/leptonSF/")
				    +"leptonMVAUL_SF_muons_Medium_"+year+".root";
    TFile* muonSFFile = TFile::Open( (muonSFFileName).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one and errors relative
    // (note: the histogram _combined_syst contains the absolute uncertainties as bin contents!),
    // load the statistical errors and set the bin contents to one and the errors relative
    // (note: the histogram _stat contains the absolute uncertainties as bin errors!).
    std::shared_ptr< TH2 > muonSFHist_nom( dynamic_cast< TH2* >(
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt" ) ) );
    muonSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_syst( dynamic_cast< TH2* >(
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt_combined_syst" ) ) );
    muonSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_stat( dynamic_cast< TH2* >(
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt_stat" ) ) );
    muonSFHist_stat->SetDirectory( gROOT );
    muonSFFile->Close();
    for(int i = 0; i <= muonSFHist_nom->GetNbinsX()+1; ++i){
        for(int j = 0; j <= muonSFHist_nom->GetNbinsY()+1; ++j){
            // process values
            muonSFHist_nom->SetBinError(i,j,0.);
            muonSFHist_syst->SetBinError(i,j,
                muonSFHist_syst->GetBinContent(i,j)/muonSFHist_nom->GetBinContent(i,j));
            muonSFHist_syst->SetBinContent(i,j,1.);
            muonSFHist_stat->SetBinError(i,j,
                muonSFHist_stat->GetBinError(i,j)/muonSFHist_nom->GetBinContent(i,j));
            muonSFHist_stat->SetBinContent(i,j,1.);
        }
    } 
    MuonReweighter muonReweighter_nom( muonSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter("muon",
        std::make_shared<ReweighterMuons>(muonReweighter_nom));
    MuonReweighter muonReweighter_syst( muonSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter("muonsyst",
        std::make_shared<ReweighterMuons>(muonReweighter_syst));
    MuonReweighter muonReweighter_stat( muonSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter("muonstat",
        std::make_shared<ReweighterMuons>(muonReweighter_stat));

    // make electron ID reweighter
    std::string eleSFFileName = std::string("../../weightFilesUL/leptonSF/")
				    +"leptonMVAUL_SF_electrons_Tight_"+year+".root";
    TFile* eleSFFile = TFile::Open( (eleSFFileName).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one and errors relative
    // (note: the histogram syst contains the absolute uncertainties as bin contents!),
    // load the statistical errors and set the bin contents to one and errors relative
    // (note: the histogram stat contains the absolute uncertainties as bin contents!).
    std::shared_ptr< TH2 > electronSFHist_nom( dynamic_cast< TH2* >
    ( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_syst( dynamic_cast< TH2* >
        ( eleSFFile->Get( "sys" ) ) );
    electronSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_stat( dynamic_cast< TH2* >
        ( eleSFFile->Get( "stat" ) ) );
    electronSFHist_stat->SetDirectory( gROOT );
    eleSFFile->Close();
    for(int i = 0; i <= electronSFHist_nom->GetNbinsX()+1; ++i){
        for(int j = 0; j <= electronSFHist_nom->GetNbinsY()+1; ++j){
            electronSFHist_nom->SetBinError(i,j,0.);
            electronSFHist_syst->SetBinError(i,j,
                electronSFHist_syst->GetBinContent(i,j)/electronSFHist_nom->GetBinContent(i,j));
            electronSFHist_syst->SetBinContent(i,j,1.);
            electronSFHist_stat->SetBinError(i,j,
                electronSFHist_stat->GetBinContent(i,j)/electronSFHist_nom->GetBinContent(i,j));
            electronSFHist_stat->SetBinContent(i,j,1.);
        }
    }
    ElectronIDReweighter electronIDReweighter_nom( electronSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter( "electron",
        std::make_shared<ReweighterElectronsID>(electronIDReweighter_nom) );
    ElectronIDReweighter electronIDReweighter_syst( electronSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter( "electronsyst",
        std::make_shared<ReweighterElectronsID>(electronIDReweighter_syst) );
    ElectronIDReweighter electronIDReweighter_stat( electronSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter( "electronstat",
        std::make_shared<ReweighterElectronsID>(electronIDReweighter_stat) );

    // do event loop
    unsigned numberOfSamples = samples.size();
    for( unsigned i = 0; i < numberOfSamples; ++i ){
        std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        if(modeSampleList ) treeReader.initSample( samples[i] );
        long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEvents==0 ) nEvents = numberOfEntries;
        else nEvents = std::min(nEvents, numberOfEntries);
        std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
        for( long unsigned entry = 0; entry < nEvents; ++entry ){
            Event event = treeReader.buildEvent( entry );
            // do basic event cleaning
	    event.selectLooseLeptons();
	    event.removeTaus();
            event.cleanJetsFromFOLeptons();
            event.jetCollection().selectGoodJets();
	    // determine variable to fill
	    double leppt = 0;
	    if( event.numberOfLeptons()>0 ){
		event.sortLeptonsByPt();
		leppt = event.leptonCollection()[0].pt();
	    }
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpreselection"]["nominal"].get(),
				  leppt, 1. );
	    // fill histograms with reweighting
	    double weight = combinedReweighter.totalWeight(event);
	    histogram::fillValue( histograms["weightedpreselection"]["nominal"].get(),
				  leppt, weight );
	    for( std::string var: variations ){
		double weightUp = combinedReweighter.weightUp( event, var );
		double weightDown = combinedReweighter.weightDown( event, var );
		histogram::fillValue( histograms["weightedpreselection"][var+"Up"].get(),
				      leppt, weightUp );
		histogram::fillValue( histograms["weightedpreselection"][var+"Down"].get(),
                                      leppt, weightDown );
	    }
	    // do some dummy event selection
            if( event.numberOfMediumBTaggedJets() < 2 ){ continue; }
            if( event.numberOfLightLeptons() < 2 ){ continue; }
            // fill histograms without reweighting
            histogram::fillValue( histograms["unweightedpostselection"]["nominal"].get(),
                                  leppt, 1. );
            // fill histograms with reweighting
	    histogram::fillValue( histograms["weightedpostselection"]["nominal"].get(),
                                  leppt, weight );
            for( std::string var: variations ){
                double weightUp = combinedReweighter.weightUp( event, var );
                double weightDown = combinedReweighter.weightDown( event, var );
                histogram::fillValue( histograms["weightedpostselection"][var+"Up"].get(),
                                      leppt, weightUp );
                histogram::fillValue( histograms["weightedpostselection"][var+"Down"].get(),
                                      leppt, weightDown );
            }
        }
    }

    // write histograms to output file
    TFile* filePtr = TFile::Open( outputFileName.c_str(), "recreate" );
    for( std::string snapshot: snapshots ){
        histograms[snapshot]["nominal"]->Write();
        for( std::string var: variations ){
            histograms[snapshot][var+"Up"]->Write();
            histograms[snapshot][var+"Down"]->Write();
        }
    }  
    filePtr->Close();
}
