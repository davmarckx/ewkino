/*
Compare event variables between detector level and particle level
*/

// Takes a ROOT ntuple as an input and produces a ROOT file with histograms,
// corresponding to the provided event selection and variable definitions.
// The histograms in the output file are named as follows:
// <process tag>_<event selection>_<selection type>_<variable>
// (where selection type can be the usual options for detector level
// and "particlelevel" for particle level selection)


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
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/readFakeRateTools.h"
#include "../../Tools/interface/readChargeFlipTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/variableTools.h"

// include analysis tools
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"


std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > initHistMap(
				const std::vector<HistogramVariable> histvars,
				const std::string processName,
				const std::vector<std::string> event_selections,
				const std::vector<std::string> selection_types ){
    // map of process name to variable to histogram!
    
    // initialize the output histogram map
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap;
    // loop over event selections and selection types
    for(std::string es: event_selections){
        for(std::string st: selection_types){
	    // initialize the name
            std::string instanceName = es+"_"+st;
            std::string thisProcessName = processName;
            if( st=="fakerate" ) thisProcessName = "nonprompt";
            if( st=="chargeflips" ) thisProcessName = "chargeflips";
	    // make a set of thistograms
	    std::map<std::string, std::shared_ptr<TH1D>> hists;
	    hists = variableTools::initializeHistograms( histvars );
	    // loop over variables
	    for(unsigned int i=0; i<histvars.size(); ++i){
		std::string variable = histvars[i].name();
		std::string name = thisProcessName+"_"+instanceName+"_"+variable;
		histMap[instanceName][variable] = hists[variable];
		histMap[instanceName][variable]->SetName(name.c_str());
		histMap[instanceName][variable]->SetTitle(thisProcessName.c_str());
	    }
	}
    }
    return histMap;
}


void fillHistograms(const std::string& inputDirectory,
			const std::string& sampleList,
			int sampleIndex,
			unsigned long nEvents,
			const std::string& outputDirectory,
			const std::vector<std::string> event_selections,
			const std::vector<std::string> selection_types,
			const std::string& variation,
			const std::string& muonfrmap,
			const std::string& electronfrmap,
			const std::string& electroncfmap,
			const std::vector<HistogramVariable> histvars){
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

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    for(std::string st: selection_types){
	if(st=="fakerate"){
	    frmap_muon = readFakeRateTools::readFRMap(muonfrmap, "muon", year);
	    frmap_electron = readFakeRateTools::readFRMap(electronfrmap, "electron", year);
	    std::cout << "read fake rate maps" << std::endl;
	}
    }

    // load charge flip maps if needed
    std::shared_ptr<TH2D> cfmap_electron;
    for(std::string st: selection_types){
        if(st=="chargeflips"){
            cfmap_electron = readChargeFlipTools::readChargeFlipMap(
				electroncfmap, year, "electron");
            std::cout << "read charge flip maps" << std::endl;
        }
    }
    
    // make reweighter
    std::cout << "initializing Reweighter..." << std::endl;
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    reweighterFactory = std::shared_ptr<ReweighterFactory>( 
	//new EmptyReweighterFactory() ); // for testing
	new Run2ULReweighterFactory() ); // for real
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    
    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::vector<std::string> selection_types_ext;
    for( const std::string& el: selection_types ){ selection_types_ext.push_back(el); }
    selection_types_ext.push_back("particlelevel");
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap =
        initHistMap(histvars, processName, event_selections, selection_types_ext );

    // initialize pass nominal counter
    std::map<std::string,     // event selection
        std::map<std::string, // selection type
        long unsigned > > passNominalCounter;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types_ext){
            passNominalCounter[event_selection][selection_type] = 0;
        }
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();
	std::map<std::string,double> varmapPL = eventFlatteningParticleLevel::initVarMap();
	Event event = treeReader.buildEvent(entry,false,false,false,false,true);

	// loop over event selections
	for(std::string es: event_selections){
	    // loop over selection types at detector level
	    for(std::string st: selection_types){
		std::string instanceName = es+"_"+st;
		// do event selection
		if(!passES(event, es, st, variation)) continue;
		passNominalCounter.at(es).at(st)++;
		// fill histograms
		varmap = eventFlattening::eventToEntry(event, reweighter, st, 
					frmap_muon, frmap_electron, cfmap_electron, 
                                        variation);
		double weight = varmap["_normweight"];
		for(HistogramVariable histVar: histvars){
		    std::string variableName = histVar.name();
		    std::string variable = histVar.variable();
		    histogram::fillValue( histMap.at(instanceName).at(variableName).get(),
				          varmap.at(variable), weight);
		}
	    } // end loop over selection types at detector level
	    
	    // do selection at particle level
	    std::string selectionType = "particlelevel";
	    std::string instanceName = es+"_"+selectionType;
	    if( !eventSelectionsParticleLevel::passES(event, es) ) continue;
	    passNominalCounter.at(es).at(selectionType)++;
	    varmapPL = eventFlatteningParticleLevel::eventToEntry(event);
            double weight = event.weight();
            for(HistogramVariable histVar: histvars){
                std::string variableName = histVar.name();
                std::string variable = histVar.variable();
                //histogram::fillValue( histMap.at(instanceName).at(variableName).get(),
                //                      varmapPL.at(variable), weight);
		// alternative: do not use histogram::fillValue but use TH1->Fill directly,
		// in order to put values out of bounds in under- and overflow bins.
		histMap.at(instanceName).at(variableName).get()->Fill(varmapPL.at(variable), weight);
            }
	} // end loop over event selections
    } // end loop over events

    // print number of events passing nominal selection
    std::cout << "number of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types_ext){
            std::cout << "  - " << event_selection << " " << selection_type;
            std::cout << " " << passNominalCounter.at(event_selection).at(selection_type) << std::endl;
        }
    }

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over event selections and selection types
    for(std::string es: event_selections){
	for(std::string st: selection_types_ext){
	    std::string instanceName = es+"_"+st;
	    // loop over variables
	    for(HistogramVariable histVar: histvars){
		std::string variableName = histVar.name();
		// fine binned histograms and fake rate histograms should not be clipped,
		// as negative values are allowed at this stage.
		bool doClip = true;
		if( stringTools::stringContains(variableName,"fineBinned") ) doClip = false;
		if( st == "fakerate") doClip = false;
		// find the histogram for this variable
		std::shared_ptr<TH1D> hist = histMap[instanceName][variableName];
		// if histogram is empty, fill with dummy value (needed for combine);
		if( hist->GetEntries()==0 ){
		    hist->SetBinContent(1,1e-6); 
		}
		// clip and write histogram
		if( doClip ) histogram::clipHistogram( hist.get() );
		hist->Write();
	    }
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 13 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " variable_file, event_selection, selection_type, variation,";
	std::cerr << " muonfrmap, electronfrmap, electroncfmap";
	std::cerr << " nevents" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& variable_file = argvStr[5];
    std::string& event_selection = argvStr[6];
    std::vector<std::string> event_selections = stringTools::split(event_selection,"+");
    std::string& selection_type = argvStr[7];
    std::vector<std::string> selection_types = stringTools::split(selection_type,"+");
    std::string& variation = argvStr[8];
    std::string& muonfrmap = argvStr[9];
    std::string& electronfrmap = argvStr[10];
    std::string& electroncfmap = argvStr[11];
    unsigned long nevents = std::stoul(argvStr[12]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - variable file: " << variable_file << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - selection type: " << selection_type << std::endl;
    std::cout << "  - variation: " << variation << std::endl;
    std::cout << "  - muon FR map: " << muonfrmap << std::endl;
    std::cout << "  - electron FR map: " << electronfrmap << std::endl;
    std::cout << "  - electron CF map: " << electroncfmap << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // read variables
    std::vector<HistogramVariable> histvars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histvars ){
	std::cout << var.toString() << std::endl;	
    }*/

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    std::map<std::string,double> emptymapPL = eventFlatteningParticleLevel::initVarMap();
    for(HistogramVariable histVar: histvars){
	std::string variableName = histVar.name();
        std::string variable = histVar.variable();
        if( emptymap.find(variable)==emptymap.end() ){
	    std::string msg = "ERROR: variable "+variable+" ("+variableName+")";
	    msg.append(" not recognized in detector-level variable map.");
	    throw std::invalid_argument(msg);
	}
	if( emptymapPL.find(variable)==emptymapPL.end() ){
            std::string msg = "ERROR: variable "+variable+" ("+variableName+")";
            msg.append(" not recognized in particle-level variable map.");
            throw std::invalid_argument(msg);
        }
    }

    // fill the histograms
    fillHistograms( input_directory, sample_list, sample_index, nevents,
		    output_directory,
		    event_selections, selection_types, variation,
		    muonfrmap, electronfrmap, electroncfmap,
		    histvars );
    std::cerr << "###done###" << std::endl;
    return 0;
}
