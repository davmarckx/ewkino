/*
Test script for trigger reweighter.
*/

// include trigger reweighter
#include "../../interface/ReweighterTrigger.h"

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
        std::cerr << "ERROR: fillTestReweighterTrigger.cc requires " << nargs;
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
    HistInfo histInfo = HistInfo( "nVertices", "nVertices", 60, -0.5, 59.5 );
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
    
    // make the trigger reweighter
    std::string triggerWeightPath = "../../weightFilesUL/triggerSF/scalefactors_allYears.root";
    std::shared_ptr<ReweighterTrigger> reweighterTrigger = std::make_shared< ReweighterTrigger >( triggerWeightPath, year);

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
            // do basic jet cleaning
            event.cleanJetsFromFOLeptons();
            event.jetCollection().selectGoodJets();
	    // determine variable to fill
	    double nVertices = event.numberOfVertices();
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpreselection"]["nominal"].get(),
				  nVertices, 1. );
	    // fill histograms with reweighting
	    histogram::fillValue( histograms["weightedpreselection"]["nominal"].get(),
                                      nVertices, reweighterTrigger->weight(event) );
	    histogram::fillValue( histograms["weightedpreselection"]["up"].get(),
				      nVertices, reweighterTrigger->weightUp(event) );
	    histogram::fillValue( histograms["weightedpreselection"]["down"].get(),
				      nVertices, reweighterTrigger->weightDown(event) );
	    // do some dummy event selection
	    if( event.numberOfMediumBTaggedJets() < 2 ){ continue; }
	    // fill histograms without reweighting
            histogram::fillValue( histograms["unweightedpostselection"]["nominal"].get(),
                                  nVertices, 1. );
            // fill histograms with reweighting
            histogram::fillValue( histograms["weightedpostselection"]["nominal"].get(),
                                      nVertices, reweighterTrigger->weight(event) );
            histogram::fillValue( histograms["weightedpostselection"]["up"].get(),
                                      nVertices, reweighterTrigger->weightUp(event) );
            histogram::fillValue( histograms["weightedpostselection"]["down"].get(),
                                      nVertices, reweighterTrigger->weightDown(event) );
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
