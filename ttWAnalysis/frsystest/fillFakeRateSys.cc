// This is the main C++ class used to run the analysis,
// i.e. produce distributions + systematic variations as input for combine.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include other parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/CombinedReweighter.h"
#include "../../weights/interface/ReweighterFakeRate.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/histogramTools.h"

// include analysis tools
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"


std::map< std::string,     // process
    std::map< std::string, // event selection
    std::map< std::string, // selection type
    std::map< std::string, // variable
    std::map< std::string, // systematic
    std::shared_ptr<TH1D> > > > > > initHistMap(
    const std::vector<std::string>& processNames,
    const std::vector<std::string>& eventSelections,
    const std::vector<std::string>& selectionTypes,
    const std::vector<HistogramVariable>& histVars,
    const std::vector<std::string>& systematics){
    // make map of histograms
    // the resulting map has five levels: 
    // map[process name][event selection][selection type][variable name][systematic]
    // notes:
    // - processNames should be just one name (per file) in most cases,
    //   but can be multiple if a sample is split in sub-categories.
    // - isData should be true for data samples and false for MC samples;
    //   systematics are skipped for data except for fake rate selection type.
    
    // initialize the output histogram map
    std::map< std::string, 
	std::map< std::string,
	std::map< std::string, 
	std::map< std::string, 
	std::map< std::string, 
	std::shared_ptr<TH1D> > > > > > histMap;
    // loop over process names
    for(std::string processName: processNames){
    // loop over event selections
    for(std::string eventSelection: eventSelections){
    // loop over selection types
    for(std::string selectionType: selectionTypes){
    // loop over variables
    for(HistogramVariable histVar: histVars){
	std::string variableName = histVar.name();
	// form the histogram name
	std::string baseName = processName+"_"+eventSelection+"_"+selectionType+"_"+variableName;
	// add nominal histogram
	histMap[processName][eventSelection][selectionType][variableName]["nominal"] = histVar.initializeHistogram( baseName+"_nominal" );
	histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at("nominal")->SetTitle(processName.c_str());
	// loop over systematics
	for(std::string systematic : systematics){
	    std::string temp = systematic + "Up";
	    histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
	    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
	    temp = systematic + "Down";
	    histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
	    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
	} // systematics
    } // variables
    } // selection types
    } // event selections
    } // process names
    return histMap;
}


std::shared_ptr<TH1D> findHistogramToFill(
    const std::map< std::string, // process
            std::map< std::string, // event selection
            std::map< std::string, // selection type
            std::map< std::string, // variable
            std::map< std::string, // systematic
            std::shared_ptr<TH1D> > > > > >& histMap,
    const std::string& processName,
    const std::string& eventSelection,
    const std::string& selectionType,
    const std::string& systematic,
    const HistogramVariable& variable){
    // helper function to find the correct histogram to fill in the map.
    std::string variableName = variable.name();
    // find the histogram in the map
    try{
        return histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(systematic);
    }
    catch(...){
        std::string msg = "ERROR in findHistogramToFill: histogram not found in the map!";
        msg.append(" Error occurred for following key:\n");
        msg.append("  - process name: "+processName+"\n");
        msg.append("  - event selection: "+eventSelection+"\n");
        msg.append("  - selection type: "+selectionType+"\n");
        msg.append("  - variable name: "+variableName+"\n");
        msg.append("  - systematic: "+systematic+"\n");
        throw std::runtime_error(msg);
    }
}


void fillHistogram(
    const std::map< std::string, // process
            std::map< std::string, // event selection
            std::map< std::string, // selection type
            std::map< std::string, // variable
            std::map< std::string, // systematic
            std::shared_ptr<TH1D> > > > > >& histMap,
    const std::string& processName,
    const std::string& eventSelection,
    const std::string& selectionType,
    const std::string& systematic,
    const HistogramVariable& variable,
    double value, double weight){
    // small helper function to find correct histogram in the map and fill it.
    std::shared_ptr<TH1D> histogramToFill = findHistogramToFill(
        histMap, processName, eventSelection, selectionType, systematic,
        variable);
    histogram::fillValue(histogramToFill.get(), value, weight);
}


void fillSystematicsHistograms(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<HistogramVariable>& histVars, 
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
            const std::string& muonFRMap, 
	    const std::string& electronFRMap,
            const std::string& electronCFMap,
	    unsigned long nEntries ){

    // initialize TreeReader from input file
    std::cout << "=== start function fillSystematicsHistograms ===" << std::endl;;
    std::cout << "initializing TreeReader and setting to sample n. " << sampleIndex << "..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(unsigned int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"fakerate")!=selection_types.end()
	|| std::find(selection_types.begin(),selection_types.end(),"efakerate")!=selection_types.end()
	|| std::find(selection_types.begin(),selection_types.end(),"mfakerate")!=selection_types.end()){
	std::cout << "reading fake rate maps..." << std::endl;
        frmap_muon = readFakeRateTools::readFRMap(muonFRMap, "muon", year);
        frmap_electron = readFakeRateTools::readFRMap(electronFRMap, "electron", year);
    }

    // load charge flip maps if needed
    std::shared_ptr<TH2D> cfmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"chargeflips")!=selection_types.end()){
        std::cout << "reading charge flip maps..." << std::endl;
	cfmap_electron = readChargeFlipTools::readChargeFlipMap(
			    electronCFMap, year, "electron");
    }

    // make reweighter and initialize systematics
    std::cout << "initializing Reweighter..." << std::endl;
    CombinedReweighter reweighter; 
    reweighter.addReweighter( "efakeratenorm", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "electron", year, "norm") );
    reweighter.addReweighter( "efakeratept", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "electron", year, "pt") );
    reweighter.addReweighter( "efakerateeta", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "electron", year, "eta") );
    reweighter.addReweighter( "mfakeratenorm", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "muon", year, "norm") );
    reweighter.addReweighter( "mfakeratept", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "muon", year, "pt") );
    reweighter.addReweighter( "mfakerateeta", std::make_shared<ReweighterFakeRate>(
        electronFRMap, muonFRMap, "muon", year, "eta") );
    std::vector<std::string> systematics;
    systematics.push_back("efakeratenorm");
    systematics.push_back("efakeratept");
    systematics.push_back("efakerateeta");
    systematics.push_back("mfakeratenorm");
    systematics.push_back("mfakeratept");
    systematics.push_back("mfakerateeta");
    
    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::vector<std::string> processNames = {processName};
    std::map< std::string,     // process
	std::map< std::string, // event selection
	std::map< std::string, // selection type
	std::map< std::string, // variable
	std::map< std::string, // systematic
        std::shared_ptr<TH1D> > > > > > histMap = initHistMap(
	processNames, event_selections, selection_types, histVars, systematics);

    // initialize pass nominal counter
    std::map<std::string,     // event selection
	std::map<std::string, // selection type
	long unsigned > > passNominalCounter;
    for(std::string event_selection: event_selections){
	for(std::string selection_type: selection_types){
	    passNominalCounter[event_selection][selection_type] = 0;
	}
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    double nEntriesReweight = 1;
    if( nEntries!=0 && nEntries<numberOfEntries ){
	// loop over a smaller number of entries if samples are impractically large
	std::cout << "limiting number of entries to " << nEntries << std::endl;
	nEntriesReweight = (double)numberOfEntries/nEntries;
	std::cout << "(with corresponding reweighting factor " << nEntriesReweight << ")" << std::endl;
	numberOfEntries = nEntries;
    }
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
	// printouts for progress checking
        if(entry%1000 == 0){
            if(entry < 10000 || (entry%10000 == 0)){
                std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl; }
        }

	// build the event
	Event event = treeReader.buildEvent(
                        entry,
                        false,false,
                        false, false, false );

	// initialize map of variables
        std::map<std::string,double> varmap = eventFlattening::initVarMap();

	// loop over event selections
	for( std::string event_selection: event_selections ){

	// loop over selection types
	for( std::string selection_type: selection_types ){

	// fill nominal histograms
	bool passnominal = true;
	double nominalWeight = 0;
        if(!passES(event, event_selection, selection_type, "nominal")) passnominal = false;
	if(passnominal){
	    varmap = eventFlattening::eventToEntry(event, 
			reweighter, selection_type, 
			frmap_muon, frmap_electron, cfmap_electron, "nominal",
                        nullptr, year);
	    nominalWeight = varmap.at("_normweight")*nEntriesReweight;
	    passNominalCounter.at(event_selection).at(selection_type)++;
	    for(HistogramVariable histVar: histVars){
		std::string variableName = histVar.name();
		std::string variable = histVar.variable();
		fillHistogram( histMap, processName, 
		    event_selection, selection_type, "nominal",
		    histVar, varmap.at(variable), nominalWeight );
	    }
	}

	// fill data systematics histograms
	if(passnominal){
	    // loop over systematics
	    for(std::string systematic : systematics){
		std::string upvar = systematic+"Up";
		std::string downvar = systematic+"Down";
		double upWeight = nominalWeight * reweighter.singleWeightUp(event, systematic);
		double downWeight = nominalWeight * reweighter.singleWeightDown(event, systematic);
		for(HistogramVariable histVar: histVars){
			std::string variableName = histVar.name();
			std::string variable = histVar.variable();
			fillHistogram( histMap, processName,
                            event_selection, selection_type, upvar,
                            histVar, varmap.at(variable), upWeight );
			fillHistogram( histMap, processName,
                            event_selection, selection_type, downvar,
                            histVar, varmap.at(variable), downWeight );
		}
	    }
	}

	} } // end loop over event selections and selection types
    } // end loop over events

    // print number of events passing nominal selection
    std::cout << "number of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types){
            std::cout << "  - " << event_selection << " " << selection_type;
	    std::cout << " " << passNominalCounter.at(event_selection).at(selection_type) << std::endl;;
        }
    }

    // save histograms to the output file
    for(std::string event_selection: event_selections){
    for(std::string selection_type: selection_types){
	// make output ROOT file
	std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
	outputFilePath += event_selection + "/" + selection_type + "/";
        systemTools::makeDirectory(outputFilePath);
	outputFilePath += inputFileName;
	TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
	// loop over variables
	for(HistogramVariable histVar: histVars){
	    std::string variableName = histVar.name();
	    // loop over histograms
	    for(auto mapelement : histMap.at(processName).at(event_selection).at(selection_type).at(variableName)){
		std::shared_ptr<TH1D> hist = mapelement.second;
		hist->Write();
	    } // end loop over histograms
	} // end loop over variables for writing histograms
	outputFilePtr->Close();
    } } // end loop over event selections and selection types for writing histograms
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 11;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: runanalysis.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input_directory" << std::endl;
	std::cerr << "sample_list" << std::endl;
	std::cerr << "sample_index" << std::endl;
	std::cerr << "output_directory" << std::endl;
	std::cerr << "variable_file" << std::endl;
	std::cerr << "event_selection" << std::endl;
	std::cerr << "selection_type" << std::endl;
	std::cerr << "muonfrmap" << std::endl;
	std::cerr << "electronfrmap" << std::endl;
        std::cerr << "electroncfmap" << std::endl;
	std::cerr << "nevents" << std::endl;
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
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    std::string& selection_type = argvStr[7];
    std::vector<std::string> selection_types = stringTools::split(selection_type,",");
    std::string& muonfrmap = argvStr[8];
    std::string& electronfrmap = argvStr[9];
    std::string& electroncfmap = argvStr[10];
    unsigned long nevents = std::stoul(argvStr[11]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - variable file: " << variable_file << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - selection type: " << selection_type << std::endl;
    std::cout << "  - muon FR map: " << muonfrmap << std::endl;
    std::cout << "  - electron FR map: " << electronfrmap << std::endl;
    std::cout << "  - electron CF map: " << electroncfmap << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // read variables
    std::vector<HistogramVariable> histVars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histVars ){
        std::cout << var.toString() << std::endl;       
    }*/

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(HistogramVariable histVar: histVars){
        std::string variable = histVar.variable();
        if( emptymap.find(variable)==emptymap.end() ){
            std::string msg = "ERROR: variable " + variable + " not found";
	    msg.append( " in the per-event variable map!" );
            throw std::invalid_argument(msg);
        }
    }

    // fill the histograms
    fillSystematicsHistograms( input_directory, sample_list, sample_index, output_directory,
			       histVars, event_selections, selection_types, 
			       muonfrmap, electronfrmap, electroncfmap, 
                               nevents );

    std::cerr << "###done###" << std::endl;
    return 0;
}
