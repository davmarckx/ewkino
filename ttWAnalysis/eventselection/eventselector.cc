// This is the main C++ class used to perform event selection.
// It is supposed to run on the output file of a skimming procecure
// and produce an even more reduced version of that file targeting specific events.

// include c++ library classes 
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
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "interface/eventSelections.h"

void eventloopES(const std::string& inputDirectory, 
		    const std::string& sampleList,
		    int sampleIndex,
		    unsigned long nEvents,
		    const std::string& outputDirectory,
		    const std::string& event_selection, 
		    const std::string& selection_type, 
		    const std::string& variation){

    // initialize TreeReader
    std::cout << "initialize TreeReader for sample at index " << sampleIndex << "." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){ treeReader.initSample(); }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();

    // make output file
    std::string outputdir = "blackJackAndHookers";
    std::string treename = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );

    // copy histograms from input file to output file
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }

    // make output Tree
    std::shared_ptr< TTree > treePtr( std::make_shared< TTree >(
                                            treename.c_str(), treename.c_str() ) );
    treeReader.setOutputTree( treePtr.get() );
    
    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	Event event = treeReader.buildEvent(entry);
	// warning: without additional arguments, buildEvent does not read individual 
	// triggers, met filters or split jec variations! 
	if(!passES(event, event_selection, selection_type, variation)) continue;
	treePtr->Fill();
    }
    treePtr->Write("", BIT(2) );
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 9  ){
        std::cerr << "ERROR: event selection requires different number of arguments:";
        std::cerr << " input_directory, sample_list, sample_index,";
        std::cerr << " output_directory,";
        std::cerr << " event_selection, selection_type, variation";
	std::cerr << " nevents" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    // necessary arguments:
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    // type of selection and variation to consider:
    std::string& selection_type = argvStr[6];
    std::string& variation = argvStr[7];
    unsigned long nevents = std::stoul(argvStr[8]);

    // call functions
    eventloopES( input_directory, sample_list, sample_index, nevents,
		 output_directory,
                 event_selection, selection_type, variation );
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
