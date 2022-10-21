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

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/readFakeRateTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/variableTools.h"

// include analysis tools
#include "interface/eventSelections.h"
#include "interface/eventFlattening.h"


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
    
    // make reweighter
    std::cout << "initializing Reweighter..." << std::endl;
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    reweighterFactory = std::shared_ptr<ReweighterFactory>( 
	new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    
    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap =
        initHistMap(histvars, processName, event_selections, selection_types );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();
	Event event = treeReader.buildEvent(entry,false,false,false,false);

	for(std::string es: event_selections){
	    for(std::string st: selection_types){
		std::string instanceName = es+"_"+st;
	
		// fill histogram
		bool pass = true;
		if(!passES(event, es, st, variation)) pass = false;
		if(pass){
		    varmap = eventFlattening::eventToEntry(event, reweighter, st, 
					frmap_muon, frmap_electron, variation);
	
		    /*std::cout << "----" << std::endl;
		    std::cout << varmap["_normweight"] << std::endl;
		    std::cout << event.weight() << std::endl;
		    std::cout << reweighter.totalWeight(event) << std::endl;
		    std::cout << reweighter["muonID"]->weight(event) << std::endl;
		    std::cout << reweighter["electronID"]->weight(event) << std::endl;
		    std::cout << reweighter["electronReco_pTBelow20"]->weight(event) << std::endl;
		    std::cout << reweighter["electronReco_pTAbove20"]->weight(event) << std::endl;
		    std::cout << reweighter["prefire"]->weight(event) << std::endl;
		    std::cout << reweighter["pileup"]->weight(event) << std::endl;*/
	    
		    double weight = varmap["_normweight"];
		    for(HistogramVariable histVar: histvars){
			std::string variableName = histVar.name();
			std::string variable = histVar.variable();
			histogram::fillValue( histMap.at(instanceName).at(variableName).get(),
				              varmap.at(variable), weight);
		    }
		}
	    } // end loop over selection types
	} // end loop over event selections
    } // end loop over events

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over event selections and selection types
    for(std::string es: event_selections){
	for(std::string st: selection_types){
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

    if( argc < 12 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " variable_file, event_selection, selection_type, variation,";
	std::cerr << " muonfrmap, electronfrmap,";
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
    std::cout << "  - variation: " << variation << std::endl;
    std::cout << "  - muon FR map: " << muonfrmap << std::endl;
    std::cout << "  - electron FR map: " << electronfrmap << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // read variables
    std::vector<HistogramVariable> histvars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histvars ){
	std::cout << var.toString() << std::endl;	
    }*/

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(HistogramVariable histVar: histvars){
	std::string variableName = histVar.name();
        std::string variable = histVar.variable();
        if( emptymap.find(variable)==emptymap.end() ){
	    std::string msg = "ERROR: variable "+variable+" ("+variableName+")";
	    msg.append(" not recognized.");
	    throw std::invalid_argument(msg);
	}
    }

    // fill the histograms
    fillHistograms( input_directory, sample_list, sample_index, nevents,
		    output_directory,
		    event_selections, selection_types, variation,
		    muonfrmap, electronfrmap, 
		    histvars );
    std::cerr << "###done###" << std::endl;
    return 0;
}
