#include "../interface/EwkinoXSections.h"

//include c++ library classes
#include <vector>
#include <stdexcept>

//include other parts of framework
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../constants/branchingFractions.h"


EwkinoXSections::EwkinoXSections( const std::string& xSectionTablePath, const bool ZToLL, const bool WToLL ){

    std::vector< std::string > linesInTable = systemTools::readLines( xSectionTablePath );
    for( const auto& line : linesInTable ){
        std::vector< std::string > splitLine = stringTools::split( line );
        unsigned mass = static_cast< unsigned >( std::stoi( splitLine[0] ) );
        double xSec = std::stod( splitLine[1] );
        if( ZToLL ){
            xSec *= particle::brZToLL;
        }
        if( WToLL ){
            xSec *= particle::brWToLNu;
        }
        massToXSec[ mass ] = xSec;
    }
}


double EwkinoXSections::xSection( unsigned mass ) const{
    auto it = massToXSec.find( mass );
    if( it == massToXSec.cend() ){
        throw std::invalid_argument( "No cross section is available for NLSP mass of " + std::to_string( mass ) + " GeV." );
    }
    return it->second;
}
