/*
Test script for muon reco reweighter.
*/

// include muon reco reweighter
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
        std::cerr << "ERROR: fillTestReweighterMuonReco.cc requires " << nargs;
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
    HistInfo histInfo = HistInfo( "nMuons", "nMuons", 4, -0.5, 3.5 );
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
    
    // make the muon reco reweighter
    TFile* muonRecoSFFile = TFile::Open(
        (std::string("../../weightFilesUL/muonRecoSF/")
	+"muonRECO_SF_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > muonRecoSFHist(
        dynamic_cast< TH2* >( muonRecoSFFile->Get( "nominal" ) ) );
    muonRecoSFHist->SetDirectory( gROOT );
    muonRecoSFFile->Close();
    MuonReweighter mrr( muonRecoSFHist, new LooseSelector );
    std::shared_ptr< ReweighterMuons > muonRecoReweighter;
    muonRecoReweighter = std::make_shared< ReweighterMuons >( mrr );

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
	    double nMuons = event.numberOfMuons();
	    // determine event weight
	    double weight = muonRecoReweighter->weight(event);
	    double weightUp = muonRecoReweighter->weightUp(event);
	    double weightDown = muonRecoReweighter->weightDown(event);
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpreselection"]["nominal"].get(),
				  nMuons, 1. );
	    // fill histograms with reweighting
	    histogram::fillValue( histograms["weightedpreselection"]["nominal"].get(),
                                      nMuons, weight );
	    histogram::fillValue( histograms["weightedpreselection"]["up"].get(),
				      nMuons, weightUp );
	    histogram::fillValue( histograms["weightedpreselection"]["down"].get(),
				      nMuons, weightDown );
	    // do some dummy event selection
	    if( event.numberOfMediumBTaggedJets() < 2 ){ continue; }
	    if( event.numberOfLightLeptons() < 2 ){ continue; }
	    // fill histograms without reweighting
            histogram::fillValue( histograms["unweightedpostselection"]["nominal"].get(),
                                  nMuons, 1. );
            // fill histograms with reweighting
            histogram::fillValue( histograms["weightedpostselection"]["nominal"].get(),
                                      nMuons, weight );
            histogram::fillValue( histograms["weightedpostselection"]["up"].get(),
                                      nMuons, weightUp );
            histogram::fillValue( histograms["weightedpostselection"]["down"].get(),
                                      nMuons, weightDown );
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
