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
#include "../../Tools/interface/readFakeRateTools.h"
#include "../../Event/interface/Event.h"


void readFakeRateWeights(
    const std::string& pathToFile,
    unsigned long nEntries,
    const std::string& electronFRMap,
    const std::string& muonFRMap ){

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
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    frmap_muon = readFakeRateTools::readFRMap(muonFRMap, "muon", year);
    frmap_electron = readFakeRateTools::readFRMap(electronFRMap, "electron", year);

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>numberOfEntries ) nEntries = numberOfEntries;
    for( long unsigned entry = 0; entry < nEntries; ++entry ){
        // build event
        Event event = treeReader.buildEvent( entry, false, false, false, false );
	double frweight = readFakeRateTools::fakeRateWeight(event, frmap_muon, frmap_electron);
	std::cout << frweight << std::endl;
    }
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc != 5 ){
        std::cerr << "scanner requires four argument to run : " << std::endl;
	std::cerr << " - input file path" << std::endl;
	std::cerr << " - number of entries" << std::endl;
	std::cerr << " - path to electron fake rate map" << std::endl;
	std::cerr << " - path to muon fake rate map" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    unsigned long nentries = std::stoul(argvStr[2]);
    std::string& electronfrmap = argvStr[3];
    std::string& muonfrmap = argvStr[4];

    // call main function
    readFakeRateWeights( input_file_path, nentries, electronfrmap, muonfrmap );

    std::cerr << "###done###" << std::endl;
    return 0;
}
