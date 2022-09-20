/*
Tools related to fake rate map reading
*/


#ifndef fakeRateTools_h
#define fakeRateTools_h

// include c++ library classes 
#include <string>
#include <exception>

// include ROOT classes 
#include "TFile.h"

// include other parts of the framework
#include "../../Event/interface/Event.h"

namespace readFakeRateTools{

    std::shared_ptr< TH2D > readFRMap( const std::string&,
				       const std::string& );

    std::shared_ptr< TH2D > readFRMap( const std::string&,
				       const std::string&, 
				       const std::string& );

    double fakeRateWeight( const Event&,
			   const std::shared_ptr< TH2D >&,
			   const std::shared_ptr< TH2D >& );

    int fakeRateFlavour( const Event& );
}

#endif
