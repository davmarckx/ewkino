/*
Test script for electron reco reweighter.
*/

// include electron reco reweighter
#include "../../interface/ConcreteLeptonReweighter.h"
#include "../../interface/ConcreteReweighterLeptons.h"
#include "../../interface/ConcreteSelection.h"

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
        std::cerr << "ERROR: fillTestReweighterElectronReco.cc requires " << nargs;
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
    HistInfo histInfo = HistInfo( "nElectrons", "nElectrons", 4, -0.5, 3.5 );
    std::map< std::string, std::map< std::string, std::shared_ptr<TH1D> >> histograms;
    std::vector< std::string > snapshots = {"unweightedpreselection",
					    "unweightedpostselection",
					    "weightedpreselection",
					    "weightedpostselection"};
    for( std::string snapshot: snapshots ){
	histograms[snapshot]["nominal"] = histInfo.makeHist( snapshot+"_nominal" );
	histograms[snapshot]["up"] = histInfo.makeHist( snapshot+"_up" );
	histograms[snapshot]["down"] = histInfo.makeHist( snapshot+"_down" );
    }
    
    // make the electron reco reweighter
    // pT below 20 GeV
    TFile* eleRecoSFFile_pTBelow20 = TFile::Open(
        (std::string("../../weightFilesUL/electronRecoSF/")
	+"electronRECO_SF_" + year + "_ptBelow20.root" ).c_str() );
    std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20(
        dynamic_cast< TH2* >( eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
    electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
    eleRecoSFFile_pTBelow20->Close();
    ElectronIDReweighter err_pTBelow20( 
	electronRecoSFHist_pTBelow20,
        new LooseMaxPtSelector< 20 > );
    std::shared_ptr< ReweighterElectronsID> electronRecoReweighter_pTBelow20;
    electronRecoReweighter_pTBelow20 = std::make_shared< ReweighterElectronsID >( err_pTBelow20 );
    // pT above 20 GeV
    TFile* eleRecoSFFile_pTAbove20 = TFile::Open(
        (std::string("../../weightFilesUL/electronRecoSF/")
	+"electronRECO_SF_" + year + "_ptAbove20.root" ).c_str() );
    std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20(
        dynamic_cast< TH2* >( eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
    electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
    eleRecoSFFile_pTAbove20->Close();
    ElectronIDReweighter err_pTAbove20( 
	electronRecoSFHist_pTAbove20,
        new LooseMinPtSelector< 20 > );
    std::shared_ptr< ReweighterElectronsID> electronRecoReweighter_pTAbove20;
    electronRecoReweighter_pTAbove20 = std::make_shared< ReweighterElectronsID >( err_pTAbove20 );

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
            // do basic event
	    event.selectLooseLeptons();
            event.cleanJetsFromFOLeptons();
            event.jetCollection().selectGoodJets();
	    // determine variable to fill
	    double nElectrons = event.numberOfElectrons();
	    // determine event weight
	    double weight = electronRecoReweighter_pTBelow20->weight(event)
			    *electronRecoReweighter_pTAbove20->weight(event);
	    double weightUp = electronRecoReweighter_pTBelow20->weightUp(event)
                              *electronRecoReweighter_pTAbove20->weightUp(event);
	    double weightDown = electronRecoReweighter_pTBelow20->weightDown(event)
                                *electronRecoReweighter_pTAbove20->weightDown(event);
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpreselection"]["nominal"].get(),
				  nElectrons, 1. );
	    // fill histograms with reweighting
	    histogram::fillValue( histograms["weightedpreselection"]["nominal"].get(),
                                      nElectrons, weight );
	    histogram::fillValue( histograms["weightedpreselection"]["up"].get(),
				      nElectrons, weightUp );
	    histogram::fillValue( histograms["weightedpreselection"]["down"].get(),
				      nElectrons, weightDown );
	    // do some dummy event selection
	    if( event.numberOfMediumBTaggedJets() < 2 ){ continue; }
	    if( event.numberOfLightLeptons() < 2 ){ continue; }
	    // fill histograms without reweighting
            histogram::fillValue( histograms["unweightedpostselection"]["nominal"].get(),
                                  nElectrons, 1. );
            // fill histograms with reweighting
            histogram::fillValue( histograms["weightedpostselection"]["nominal"].get(),
                                      nElectrons, weight );
            histogram::fillValue( histograms["weightedpostselection"]["up"].get(),
                                      nElectrons, weightUp );
            histogram::fillValue( histograms["weightedpostselection"]["down"].get(),
                                      nElectrons, weightDown );
	}
    }

    // write histograms to output file
    TFile* filePtr = TFile::Open( outputFileName.c_str(), "recreate" );
    for( std::string snapshot: snapshots ){
	histograms[snapshot]["nominal"]->Write();
        histograms[snapshot]["up"]->Write();
        histograms[snapshot]["down"]->Write();
    }
    filePtr->Close();
}
