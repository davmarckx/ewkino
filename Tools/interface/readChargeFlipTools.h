/*
Tools related to charge flip map reading
*/


#ifndef chargeFlipTools_h
#define chargeFlipTools_h

// include c++ library classes 
#include <string>
#include <exception>

// include ROOT classes 
#include "TFile.h"

// include other parts of the framework
#include "../../Event/interface/Event.h"

namespace readChargeFlipTools{

    double chargeFlipWeight(
        const Event& event,
        const std::shared_ptr< TH2 >& chargeFlipMap,
	bool doCorrectionFactor );

    std::shared_ptr< TH2D > readChargeFlipMap(
        const std::string& filePath,
        const std::string& year,
        const std::string& flavour );

    std::shared_ptr< TH2D > readChargeFlipMap(
        const std::string& directory,
        const std::string& year,
        const std::string& flavour,
        const std::string& process,
        const std::string& binning );
            
}

#endif
