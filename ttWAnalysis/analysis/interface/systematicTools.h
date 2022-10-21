#ifndef systematicTools_H
#define systematicTools_H

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of the framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/histogramTools.h"

namespace systematicTools{

    std::string systematicType( const std::string systematic );
    void fillEnvelope(
	std::map< std::string, std::shared_ptr<TH1D> > histMap,
	std::string upName, std::string downName, std::string tag );
    void fillRMS(
	std::map< std::string, std::shared_ptr<TH1D> > histMap,
	std::string upName, std::string downName, std::string tag );
}

#endif
