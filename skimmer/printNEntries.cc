/*
Small utility to quickly check number of events in an ntuple
*/

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"


long unsigned getNEntries( const std::string& pathToFile ){

    // initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return 0;
    }

    long unsigned nentries = treeReader.numberOfEntries();
    return nentries;
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc < 2 ){
        std::cerr << "ERROR: need at least one file as command line arg. " << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::vector< std::string > files;
    for( unsigned int i=1; i<argvStr.size(); i++ ){ files.push_back(argvStr[i]); }

    // loop over files
    int nfiles = files.size();
    std::cout << "checking " << nfiles << " files..." << std::endl;
    for( std::string f: files ){
        long unsigned nentries = getNEntries(f);
        std::cout << "file: " << f << ": ";
        std::cout << nentries << " entries." << std::endl;
    }

    std::cerr << "###done###" << std::endl;
    return 0;
}
