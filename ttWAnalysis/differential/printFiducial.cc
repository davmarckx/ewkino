// print events that pass the fiducial selection

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <fstream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include other parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"
#include "../analysis/interface/systematicTools.h"


void printFiducialPassing(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
	    unsigned long nEntries){
    // initialize TreeReader from input file
    std::cout << "=== start function printFiducialPassing ===" << std::endl;;
    std::cout << "initializing TreeReader and setting to sample n. " << sampleIndex << "..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(unsigned int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::cout << "initialized" << std::endl;
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    std::cout << year << std::endl;
    std::cout << inputFileName << std::endl;
    std::cout << processName << std::endl;
   

    //std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    std::string outputFilePath = outputDirectory +"/"+ processName + year + "_noJetCleaning.txt";

    std::cout << "filename" << std::endl;

    std::ofstream MyFile(outputFilePath);

    std::cout << "file initialized" << std::endl;

    // get hCounter info
    std::pair<double,int> temp = treeReader.getHCounterInfo();
    double hCounterWeights = temp.first;
    int hCounterEntries = temp.second;
    std::cout << "retrieved follwing info from hCounter:" << std::endl;
    std::cout << "  - sum of weights: " << hCounterWeights << std::endl;
    std::cout << "  - entries: " << hCounterEntries << std::endl;

    MyFile << "=== sample info ==="<< std::endl;
    MyFile << "  - sum of weights: " << hCounterWeights << std::endl;
    MyFile << "  - entries: " << hCounterEntries << std::endl;
    MyFile << "=== =========== ==="<< std::endl;
    MyFile << std::endl;
    MyFile << "|evtnr|lumi|_nJets|_nBJets|_leptonPtLeading|_leptonPtSubLeading|"<< std::endl;

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
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

        // build the event
        Event event = treeReader.buildEvent(
                        entry,
                        false, false,
                        false, false,
                        true, false );

        // initialize map of variables
        std::map<std::string,double> varmap = eventFlatteningParticleLevel::initVarMap();

        // event for unselected jets
        /*Event event2 = treeReader.buildEvent(
                        entry,
                        false, false,
                        false, false,
                        true, false );*/

        // loop over event selections and selection types
        for( std::string event_selection: event_selections ){
        for( std::string selection_type: selection_types ){

        if( selection_type!="particlelevel" ){
            std::string msg = "ERROR: found selection_type " + selection_type;
            msg.append(" while only 'particlelevel' is supported for now.");
        }

        // do nominal selection and calculate particle-level event variables
        //if( event.eventNumber()!=905872 and event.luminosityBlock()!=560 ) continue;
        
        if( !eventSelectionsParticleLevel::passES(event, event_selection) ) continue;
        varmap = eventFlatteningParticleLevel::eventToEntry(event);
        MyFile << "|" + std::to_string(event.eventNumber()) + "|" + std::to_string(event.luminosityBlock()) + "|" + std::to_string(varmap["_nJets"]) + "|" + std::to_string(varmap["_nBJets"]) + "|" + std::to_string(varmap["_leptonPtLeading"]) + "|" + std::to_string(varmap["_leptonPtSubLeading"]) + "|" + std::to_string(varmap["_leptonEtaLeading"]) + "|" + std::to_string(varmap["_leptonEtaSubLeading"]) + "|";
        
        //eventSelectionsParticleLevel::cleanLeptonsAndAllJets(event2);
        JetParticleLevelCollection jetcollection = event.jetParticleLevelCollection();
        std::pair<int,int> njetsnbjets = eventSelectionsParticleLevel::nJetsNBJets(event);
        jetcollection.sortByPt();
        for(int i=0; i<std::min(6,njetsnbjets.first); i++){
            MyFile << std::to_string(jetcollection[i].pt())  + "|";
        }
        for(int i=0; i<std::min(6,njetsnbjets.first); i++){
            MyFile << std::to_string(jetcollection[i].eta())  + "|";
        }
        MyFile <<std::endl;
       
    }}
    }
    // Close the file
    MyFile.close();
    }
int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 7;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: printFiducial.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input_directory" << std::endl;
	std::cerr << "sample_list" << std::endl;
	std::cerr << "sample_index" << std::endl;
	std::cerr << "output_directory" << std::endl;
	std::cerr << "event_selection" << std::endl;
	std::cerr << "selection_type" << std::endl;
	std::cerr << "nevents" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    std::string& selection_type = argvStr[6];
    std::vector<std::string> selection_types = stringTools::split(selection_type,",");
    unsigned long nevents = std::stoul(argvStr[7]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - selection type: " << selection_type << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;


    // fill the passing events
    printFiducialPassing( input_directory, sample_list, sample_index, output_directory,
			       event_selections, selection_types, 
                               nevents );

    std::cerr << "###done###" << std::endl;
    return 0;
}
