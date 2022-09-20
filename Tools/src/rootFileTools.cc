/*
Tools for accessing and reading root files
*/

// note: at some point should consider moving these to master branch (?)

// include header
#include "../interface/rootFileTools.h"

bool rootFileTools::nTupleIsReadable( const std::string& pathToFile ){
    // check if a file containing an ntuple is readable 
    // (i.e. a TreeReader can be created from it)
    TreeReader treeReader;
    try{ treeReader.initSampleFromFile( pathToFile ); }
    catch( std::domain_error& ){
        std::cerr << "WARNING: cannot read file " << pathToFile << std::endl;
        return false;
    }
    return true;
}
