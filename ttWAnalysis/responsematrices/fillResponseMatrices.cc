/*
Fill response matrices
*/

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TH2D.h"
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


std::map< std::string, std::map< std::string, std::shared_ptr<TH2D> > > initResponseMap(
    const std::vector<HistogramVariable> histvars,
    const std::vector<std::string> instanceNames ){
    // initialize the output histogram map
    std::map< std::string, std::map< std::string, std::shared_ptr<TH2D> > > histMap;
    // loop over instances (could be e.g. event selection regions)
    for(std::string instanceName: instanceNames){
        // make a set of thistograms
        std::map<std::string, std::shared_ptr<TH2D>> hists;
        hists = variableTools::initializeHistograms2D( histvars );
        // loop over variables
        for(unsigned int i=0; i<histvars.size(); ++i){
            std::string variable = histvars[i].name();
            std::string name = instanceName+"_"+variable;
            histMap[instanceName][variable] = hists[variable];
            histMap[instanceName][variable]->SetName(name.c_str());
        }
    }
    return histMap;
}


void fillHistograms(
    const std::string& inputDirectory,
    const std::string& sampleList,
    int sampleIndex,
    unsigned long nEvents,
    const std::string& outputDirectory,
    const std::vector<std::string>& eventSelections,
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
    std::map< std::string,std::map< std::string,std::shared_ptr<TH2D>> > histMap =
        initResponseMap( histvars, eventSelections );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// loop over event selections
	for(std::string eventSelection: eventSelections){

	    // initialize map of variables
	    bool passDL = false;
	    bool passPL = false;
	    std::map<std::string,double> varmapDL = eventFlattening::initVarMap();
	    std::map<std::string,double> varmapPL = eventFlatteningParticleLevel::initVarMap();
	    Event event = treeReader.buildEvent(entry,false,false,false,false,true);
	    double weight = event.weight();

	    // do event selection at detector level
	    if( passES(event, eventSelection, "tight", "nominal") ){
		passDL = true;
		varmapDL = eventFlattening::eventToEntry(event,
		    reweighter, "tight", nullptr, nullptr, nullptr, "nominal");
	    }

	    // do selection at particle level
	    if( eventSelectionsParticleLevel::passES(event, eventSelection) ){
		passPL = true;
		varmapPL = eventFlatteningParticleLevel::eventToEntry(event);
	    }

	    // if both selections failed, skip event
	    if( !passDL && !passPL ) continue;

	    // loop over variables
	    for(HistogramVariable histVar: histvars){
		std::string variableName = histVar.name();
		std::string variable = histVar.variable();
		// if both selection passed, fill the response matrix in the usual way
		if( passDL && passPL ){
		    histogram::fillValues( histMap.at(eventSelection).at(variableName).get(),
					varmapPL.at(variable), varmapDL.at(variable), weight );
		}
		// if one of both selections passed and the other failed,
		// fill with underflow bins
		if( passDL && !passPL ){
		    std::shared_ptr<TH2D> hist = histMap.at(eventSelection).at(variableName);
		    double xvalue = hist->GetXaxis()->GetBinLowEdge(1)-0.1;
		    double yvalue = histogram::boundedYValue(hist.get(), varmapDL.at(variable));
		    hist->Fill(xvalue, yvalue, weight);
		}
		if( !passDL && passPL ){
		    std::shared_ptr<TH2D> hist = histMap.at(eventSelection).at(variableName);
		    double xvalue = histogram::boundedXValue(hist.get(), varmapPL.at(variable));
		    double yvalue = hist->GetYaxis()->GetBinLowEdge(1)-0.1;
		    hist->Fill(xvalue, yvalue, weight);
		}
	    } // end loop over variables
	} // end loop over event selections
    } // end loop over events

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over event selections
    for(std::string eventSelection: eventSelections){
	// loop over variables
	for(HistogramVariable histVar: histvars){
	    std::string variableName = histVar.name();
	    std::shared_ptr<TH2D> hist = histMap[eventSelection][variableName];
	    hist->Write();
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 8 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " variable_file, event_selection,";
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
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    unsigned long nevents = std::stoul(argvStr[7]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - variable file: " << variable_file << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // read variables
    std::vector<HistogramVariable> histvars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histvars ){
	std::cout << var.toString() << std::endl;	
    }*/

    // check variables
    std::map<std::string,double> emptymapDL = eventFlattening::initVarMap();
    std::map<std::string,double> emptymapPL = eventFlatteningParticleLevel::initVarMap();
    for(HistogramVariable histVar: histvars){
	std::string variableName = histVar.name();
        std::string variable = histVar.variable();
        if( emptymapDL.find(variable)==emptymapDL.end() ){
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
    fillHistograms( input_directory, 
		    sample_list, 
		    sample_index, 
		    nevents,
		    output_directory,
		    event_selections,
		    histvars );
    std::cerr << "###done###" << std::endl;
    return 0;
}
