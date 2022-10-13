/*
Test script for b-tag shape reweighter.
More specifically: test if the normalization factors are initialized and applied properly,
both for nominal and for systematically varied b-tag scale factors.
*/

// include b-tag shape reweighter
#include "../../interface/ReweighterBTagShape.h"

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
        std::cerr << "ERROR: testNormalization.cc requires " << nargs;
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
    HistInfo histInfo = HistInfo( "", "Number of jets", 10, -0.5, 9.5 );
    std::map< std::string, std::map< std::string, std::shared_ptr<TH1D> >> histograms;
    std::vector< std::string > snapshots = {"unweightedpreselection",
					    "unweightedpostselection",
					    "weightedprenorm","weightedpostnorm",
					    "weightedpostselection"};
    std::vector<std::string> variations = { "hf","lf","hfstats1","hfstats2",
					    "lfstats1","lfstats2","cferr1","cferr2" };
    for( std::string snapshot: snapshots ){
	histograms[snapshot]["nominal"] = histInfo.makeHist( snapshot + "_nominal" );
	for( std::string var: variations ){
	    histograms[snapshot][var+"Up"] = histInfo.makeHist( snapshot+"_"+var+"Up" );
	    histograms[snapshot][var+"Down"] = histInfo.makeHist( snapshot+"_"+var+"Down" );
	}
    }
    
    // make the b-tag shape reweighter
    // step 1: set correct csv file
    std::string bTagSFFileName = "bTagReshaping_unc_"+year+".csv";
    std::string sfFilePath = "weightFilesUL/bTagSF/"+bTagSFFileName;
    std::string weightDirectory = "../../../weights";
    // step 2: set other parameters
    std::string flavor = "all";
    std::string bTagAlgo = "deepFlavor";
    // step 3: make the reweighter
    std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
	    weightDirectory, sfFilePath, flavor, bTagAlgo, variations, samples );

    // first event loop
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
	    event.removeTaus();
            event.cleanJetsFromLooseLeptons();
	    // determine variable to fill
	    int njets = event.jetCollection().goodJetCollection().size();
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpreselection"]["nominal"].get(),
				  njets, 1. );
	    // fill histograms with reweighting but no normalization
	    histogram::fillValue( histograms["weightedprenorm"]["nominal"].get(),
				  njets, reweighterBTagShape->weight( event ) );
	    for( std::string var: variations ){
		histogram::fillValue( histograms["weightedprenorm"][var+"Up"].get(),
				      njets, reweighterBTagShape->weightUp( event,var ) );
		histogram::fillValue( histograms["weightedprenorm"][var+"Down"].get(),
                                      njets, reweighterBTagShape->weightDown( event,var ) );
	    }
        }
    }

    // second event loop (for normalization)
    for( unsigned i = 0; i < numberOfSamples; ++i ){
	std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
	std::cout << "norm factors of b-tag reweighter before initializatino: " << std::endl;
	reweighterBTagShape->printNormFactors();
	std::map<std::string, std::map<int,double>> averageOfWeights;
	averageOfWeights = reweighterBTagShape->calcAverageOfWeights( samples[i], nEvents );
	reweighterBTagShape->setNormFactors( samples[i], averageOfWeights );
	std::cout << "norm factors of b-tag reweighter after initialization: " << std::endl;
	reweighterBTagShape->printNormFactors();
    }

    // do extra event loop (for checking)
    for( unsigned i = 0; i < numberOfSamples; ++i ){
        std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        std::map<std::string, std::map<int,double>> averageOfWeights;
        averageOfWeights = reweighterBTagShape->calcAverageOfWeights( samples[i], nEvents );
        std::cout << "averages of weights after rerunning: " << std::endl;
        for( auto el: averageOfWeights ){
            std::cout << "    systematic: " << el.first << std::endl;
            for( auto el2: el.second ){
                std::cout << "      - " << el2.first << " -> " << el2.second << std::endl;
            }
        }
    }

    // third event loop
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
	    event.removeTaus();
	    event.cleanJetsFromLooseLeptons();
	    // determine variable to fill
            int njets = event.jetCollection().goodJetCollection().size();
            // fill histograms with reweighting and with normalization
            histogram::fillValue( histograms["weightedpostnorm"]["nominal"].get(),
                                  njets, reweighterBTagShape->weight( event ) );
            for( std::string var: variations ){
                histogram::fillValue( histograms["weightedpostnorm"][var+"Up"].get(),
                                      njets, reweighterBTagShape->weightUp( event,var ) );
                histogram::fillValue( histograms["weightedpostnorm"][var+"Down"].get(),
                                      njets, reweighterBTagShape->weightDown( event,var ) );
            }
	    // do some dummy event selection
	    if( event.numberOfMediumBTaggedJets() < 2 ){ continue; }
	    // fill histograms without reweighting
	    histogram::fillValue( histograms["unweightedpostselection"]["nominal"].get(),
                                  njets, 1. );
	    // fill histograms with reweighting and with normalization
            histogram::fillValue( histograms["weightedpostselection"]["nominal"].get(),
                                  njets, reweighterBTagShape->weight( event ) );
            for( std::string var: variations ){
                histogram::fillValue( histograms["weightedpostselection"][var+"Up"].get(),
                                      njets, reweighterBTagShape->weightUp( event,var ) );
                histogram::fillValue( histograms["weightedpostselection"][var+"Down"].get(),
                                      njets, reweighterBTagShape->weightDown( event,var ) );
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
