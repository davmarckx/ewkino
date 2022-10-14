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
    Event event = treeReader.buildEvent(0, true, true, false, false);
    
    // print grouped triggers
    std::cout << "e: " << event.passTriggers_e() << std::endl;;
    std::cout << "ee: " << event.passTriggers_ee() << std::endl;
    std::cout << "eee: " << event.passTriggers_eee() << std::endl;
    std::cout << "m: " << event.passTriggers_m() << std::endl;
    std::cout << "mm: " << event.passTriggers_mm() << std::endl;
    std::cout << "mmm: " << event.passTriggers_mmm() << std::endl;
    std::cout << "em: " << event.passTriggers_em() << std::endl;
    std::cout << "eem: " << event.passTriggers_eem() << std::endl;
    std::cout << "emm: " << event.passTriggers_emm() << std::endl;
    std::cout << "ref: " << event.passTriggers_ref() << std::endl;
    
    // individual triggers
    event.triggerInfo().printAvailableIndividualTriggers(); 
}
