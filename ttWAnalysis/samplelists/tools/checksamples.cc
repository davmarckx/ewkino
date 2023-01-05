/*
Sample checking
*/

// Simple script that tries to initialize a TreeReader on a sample list.
// Use case: due to failing T2 server, some samples can be unavailable for reading,
// resulting in no error messages but just stuck jobs...
// Run this script to see which samples are problematic so they can be fixed or commented out.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/Sample.h"


void checkSamples( const std::string& inputDirectory,
		   const std::string& sampleList ){
    
    // initialize TreeReader from input file
    std::cout << "initializing TreeReader..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    unsigned int nSamples = treeReader.numberOfSamples();

    for(unsigned int idx=0; idx < nSamples; ++idx){
	std::cout << "attempting to initialize sample " << idx << std::endl;
        treeReader.initSample();
	std::cout << "succes!" << std::endl;
	std::cout << "initialized sample ";
	std::cout << "(" <<  treeReader.currentSample().fileName() << ")" << std::endl;
    }
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc != 3 ){
        std::cerr << "ERROR: use following command line args:" << std::endl;
        std::cerr << "  - input directory" << std::endl;
	std::cerr << "  - sample list" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;

    // check the samples
    checkSamples( input_directory, sample_list );
    std::cerr << "###done###" << std::endl;
    return 0;
}
