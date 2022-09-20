/*
Tools for accessing and reading root files
*/

#ifndef rootFileTools_h
#define rootFileTools_h

// include c++ library classes 
#include <string>
#include <exception>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"

namespace rootFileTools{

    bool nTupleIsReadable( const std::string& );

}

#endif
