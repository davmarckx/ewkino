/*
Quick file reading and printing
*/

//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


void printLeptonVariables(
    const std::string& pathToFile,
    unsigned long nEntries ){

    // initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return;
    }

    // other initializations
    std::string year = treeReader.getYearString();

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>numberOfEntries ) nEntries = numberOfEntries;
    for( long unsigned entry = 0; entry < nEntries; ++entry ){
        // build event
        Event event = treeReader.buildEvent( entry, false, false, false, false );

	std::cout << "-------" << std::endl;
	std::cout << "Event: " << event.eventNumber() << std::endl;
	for( const auto& leptonPtr : event.lightLeptonCollection() ){
	    std::cout << "  " << leptonPtr->isElectron();
	    std::cout << ", " << leptonPtr->pt() << ", " << leptonPtr->eta();
	    std::cout << ", " << leptonPtr->closestJetDeepFlavor();
	    std::cout << ", " << leptonPtr->ptRatio() << std::endl;
	}
    }
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc != 3 ){
        std::cerr << "scanner requires two argument to run : " << std::endl;
	std::cerr << " - input file path" << std::endl;
	std::cerr << " - number of entries" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    unsigned long nentries = std::stoul(argvStr[2]);

    // call main function
    printLeptonVariables( input_file_path, nentries );

    std::cerr << "###done###" << std::endl;
    return 0;
}
