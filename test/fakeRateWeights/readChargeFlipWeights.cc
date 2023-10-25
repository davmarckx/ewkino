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
#include "../../Tools/interface/readChargeFlipTools.h"
#include "../../Event/interface/Event.h"


void readChargeFlipWeights(
    const std::string& pathToFile,
    unsigned long nEntries,
    const std::string& electronCFMap ){

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

    // load fake rate maps
    std::shared_ptr<TH2D> cfmap_electron;
    cfmap_electron = readChargeFlipTools::readChargeFlipMap(electronCFMap, year, "electron");

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>numberOfEntries ) nEntries = numberOfEntries;
    for( long unsigned entry = 0; entry < nEntries; ++entry ){
        // build event
        Event event = treeReader.buildEvent( entry, false, false, false, false );
	double cfweight = readChargeFlipTools::chargeFlipWeight(event, cfmap_electron, true);
	std::cout << "-----" << std::endl;
	std::cout << event.eventNumber() << std::endl;
	std::cout << cfweight << std::endl;
    }
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc != 4 ){
        std::cerr << "scanner requires four argument to run : " << std::endl;
	std::cerr << " - input file path" << std::endl;
	std::cerr << " - number of entries" << std::endl;
	std::cerr << " - path to electron charge flip map" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    unsigned long nentries = std::stoul(argvStr[2]);
    std::string& electroncfmap = argvStr[3];

    // call main function
    readChargeFlipWeights( input_file_path, nentries, electroncfmap );

    std::cerr << "###done###" << std::endl;
    return 0;
}
