/*
Do basic checks on lepton and jet kinematics
*/

// This is basically just to verify the cuts
// that are in the PartcicleLevelProducer in the ntuplizer.
// Takes a ROOT ntuple as an input and produces a ROOT file with histograms,
// corresponding to the provided event selection and variable definitions.
// The histograms in the output file are named as follows:
// <process tag>_<event selection>_<variable>
// (where selection type can be any defined selection on particle level,
//  see eventselection/src/eventSelectionsParticleLevel.cc)


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
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/variableTools.h"

// include analysis tools
#include "../eventselection/interface/eventSelectionsParticleLevel.h"


std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > initHistMap(
				const std::string processName,
				const std::vector<std::string> event_selections ){
    // map of process name to variable to histogram!
    
    // initialize the output histogram map
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap;
    // loop over event selections
    for(std::string es: event_selections){
	// make a set of histograms
	std::map<std::string, std::shared_ptr<TH1D>> hists;
	std::string baseName = processName + "_" + es + "_";
	// electron pt
	std::string var = "electronPt";
	hists[var] = std::make_shared<TH1D>(
	    (baseName+var).c_str(), processName.c_str(),
            50, 0, 250 );
	// electron eta
	var = "electronEta";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            60, -3, 3 );
	// muon pt
	var = "muonPt";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            50, 0, 250 );
	// muon eta
	var = "muonEta";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            60, -3, 3 );
	// jet pt
	var = "jetPt";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            50, 0, 250 );
	// jet eta
	var = "jetEta";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            60, -3, 3 );
	// min delta R between jet and lepton
	var = "jetLeptonMinDR";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            60, 0, 6 );
	// number of leptons
	var = "nLeptons";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            6, -0.5, 5.5 );
	// number of jets
	var = "nJets";
        hists[var] = std::make_shared<TH1D>(
            (baseName+var).c_str(), processName.c_str(),
            9, -0.5, 8.5 );
	// add to map
	histMap[es] = hists;
    }
    return histMap;
}


void fillHistograms(const std::string& inputDirectory,
			const std::string& sampleList,
			int sampleIndex,
			unsigned long nEvents,
			const std::string& outputDirectory,
			const std::vector<std::string> event_selections){
    std::cout << "=== start function fillHistograms ===" << std::endl;
    
    // initialize TreeReader from input file
    std::cout<<"initializing TreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap =
        initHistMap( processName, event_selections );

    // initialize pass nominal counter
    std::map<std::string, long unsigned > passNominalCounter;
    for(std::string event_selection: event_selections){
        passNominalCounter[event_selection] = 0;
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// initialize map of variables
	Event event = treeReader.buildEvent(entry,false,false,false,false,true);
	double weight = event.weight();

	// loop over event selections
	for(std::string es: event_selections){
	   
	    // do event selection 
	    if( !eventSelectionsParticleLevel::passES(event, es) ) continue;
	    passNominalCounter.at(es)++;

	    // fill histograms
	    histMap.at(es).at("nLeptons").get()->Fill(
		event.leptonParticleLevelCollection().numberOfLeptons(), weight );
	    histMap.at(es).at("nJets").get()->Fill(
                event.jetParticleLevelCollection().numberOfJets(), weight );
	    for(std::shared_ptr<LeptonParticleLevel> lep: event.leptonParticleLevelCollection()){
		if(lep->flavor()==0){
		    histMap.at(es).at("electronPt").get()->Fill(lep->pt(), weight);
		    histMap.at(es).at("electronEta").get()->Fill(lep->eta(), weight);
		} else if(lep->flavor()==1){
		    histMap.at(es).at("muonPt").get()->Fill(lep->pt(), weight);
                    histMap.at(es).at("muonEta").get()->Fill(lep->eta(), weight);
		}
            }
	    for(std::shared_ptr<JetParticleLevel> jet: event.jetParticleLevelCollection()){
                histMap.at(es).at("jetPt").get()->Fill(jet->pt(), weight);
                histMap.at(es).at("jetEta").get()->Fill(jet->eta(), weight);
		double mindr = 99.;
		for(std::shared_ptr<LeptonParticleLevel> lep: event.leptonParticleLevelCollection()){
		    double dr = deltaR(*jet, *lep);
		    if(dr < mindr){ mindr = dr; }
		}
		histMap.at(es).at("jetLeptonMinDR").get()->Fill(mindr, weight);
	    }
	} // end loop over event selections
    } // end loop over events

    // print number of events passing nominal selection
    std::cout << "number of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
            std::cout << "  - " << event_selection;
            std::cout << " " << passNominalCounter.at(event_selection) << std::endl;
    }

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over event selections and selection types
    for(std::string es: event_selections){
	for( auto el: histMap[es] ){
	    el.second->Write();
	}
    }
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 7 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " event_selection, nevents" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::vector<std::string> event_selections = stringTools::split(event_selection,"+");
    unsigned long nevents = std::stoul(argvStr[6]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // fill the histograms
    fillHistograms( input_directory, sample_list, sample_index, nevents,
		    output_directory, event_selections );
    std::cerr << "###done###" << std::endl;
    return 0;
}
