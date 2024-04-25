/*
Do basic checks on particle level lepton and jet kinematics,
read from a NanoGen file using NanoGenTreeReader.
*/

// This is to verify the correct functioning of NanoGenTreeReader.
// The input file can be a NanoGen file (or a regular NanoAOD file with gen info).
// The histograms in the output file are named as follows:
// <variable>


// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/NanoGenTreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"


std::map<std::string, std::shared_ptr<TH1D>> initHistMap(){
    // map of variable name to histogram
    
    // initialize the output histogram map
    std::map< std::string, std::shared_ptr<TH1D>> histMap;
    // make a set of histograms
    // electron pt
    std::string var = "electronPt";
    histMap[var] = std::make_shared<TH1D>(
	    var.c_str(), var.c_str(),
	    50, 0, 250 );
    // electron eta
    var = "electronEta";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            60, -3, 3 );
    // muon pt
    var = "muonPt";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            50, 0, 250 );
    // muon eta
    var = "muonEta";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            60, -3, 3 );
    // jet pt
    var = "jetPt";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            50, 0, 250 );
    // jet eta
    var = "jetEta";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            60, -3, 3 );
    // min delta R between jet and lepton
    var = "jetLeptonMinDR";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            60, 0, 6 );
    // number of leptons
    var = "nLeptons";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            6, -0.5, 5.5 );
    // number of jets
    var = "nJets";
    histMap[var] = std::make_shared<TH1D>(
            var.c_str(), var.c_str(),
            9, -0.5, 8.5 );
    return histMap;
}


void fillHistograms(const std::string& inputDirectory,
			const std::string& sampleList,
			int sampleIndex,
			unsigned long nEvents,
			const std::string& outputDirectory){
    std::cout << "=== start function fillHistograms ===" << std::endl;
    
    // initialize TreeReader from input file
    std::cout<<"initializing NanoGenTreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    NanoGenTreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample(sampleIndex, true);
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::map< std::string, std::shared_ptr<TH1D>> histMap = initHistMap();

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    double nEntriesReweight = 1;
    if( nEvents!=0 && nEvents<numberOfEntries ){
        std::cout << "limiting number of entries to " << nEvents << std::endl;
        nEntriesReweight = (double)numberOfEntries/nEvents;
        std::cout << "(with corresponding reweighting factor " << nEntriesReweight << ")" << std::endl;
        numberOfEntries = nEvents;
    }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// initialize map of variables
	Event event = treeReader.buildEvent(entry);
	double weight = event.weight() * nEntriesReweight;

	// fill histograms
	histMap.at("nLeptons").get()->Fill(
		event.leptonParticleLevelCollection().numberOfLeptons(), weight );
	histMap.at("nJets").get()->Fill(
                event.jetParticleLevelCollection().numberOfJets(), weight );
	for(std::shared_ptr<LeptonParticleLevel> lep: event.leptonParticleLevelCollection()){
		if(lep->flavor()==0){
		    histMap.at("electronPt").get()->Fill(lep->pt(), weight);
		    histMap.at("electronEta").get()->Fill(lep->eta(), weight);
		} else if(lep->flavor()==1){
		    histMap.at("muonPt").get()->Fill(lep->pt(), weight);
                    histMap.at("muonEta").get()->Fill(lep->eta(), weight);
		}
        }
	for(std::shared_ptr<JetParticleLevel> jet: event.jetParticleLevelCollection()){
                histMap.at("jetPt").get()->Fill(jet->pt(), weight);
                histMap.at("jetEta").get()->Fill(jet->eta(), weight);
		double mindr = 99.;
		for(std::shared_ptr<LeptonParticleLevel> lep: event.leptonParticleLevelCollection()){
		    double dr = deltaR(*jet, *lep);
		    if(dr < mindr){ mindr = dr; }
		}
		histMap.at("jetLeptonMinDR").get()->Fill(mindr, weight);
	}
    } // end loop over events

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    systemTools::makeDirectory(outputFilePath);
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // write histograms
    for( auto el: histMap ){ el.second->Write(); }
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 6 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory, nevents" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    unsigned long nevents = std::stoul(argvStr[5]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // fill the histograms
    fillHistograms( input_directory, sample_list, sample_index, nevents, output_directory );
    std::cerr << "###done###" << std::endl;
    return 0;
}
