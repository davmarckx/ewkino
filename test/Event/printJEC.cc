/*
Print available JEC variations
*/

// include other parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"


int main( int argc, char* argv[] ){

    int nargs = 1;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: printJEC.cc requires " << nargs;
	std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- path to input file (.root)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputFilePath = argvStr[1];

    // read the input file
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    // initialize the first event
    Event event = treeReader.buildEvent(0, false, false, true, true);
    
    // print JEC variations
    event.jetInfo().printAllJECVariations();
    event.jetInfo().printGroupedJECVariations();
}
