
//include c++ library classes
#include <memory>

//include ROOT classes
#include "TH2D.h"

//include other parts of framework
#include "../../Event/interface/Event.h"

namespace chargeFlips{
    double chargeFlipWeight( double l1Pt, double l1AbsEta, unsigned int l1Flavor,
	double l2Pt, double l2AbsEta, unsigned int l2Flavor,
	const std::shared_ptr< TH2 >& chargeFlipMap, unsigned int flavor );
    double chargeFlipWeight( const Event&, const std::shared_ptr< TH2 >& );
    std::shared_ptr< TH2D > readChargeFlipMap( const std::string& filePath,
                                           const std::string& year,
                                           const std::string flavour );
    std::shared_ptr< TH2D > readChargeFlipMap( const std::string& directory,
                                           const std::string& year,
                                           const std::string& flavour,
                                           const std::string& process,
                                           const std::string& binning );
}
