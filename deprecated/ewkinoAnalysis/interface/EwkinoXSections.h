#ifndef EwkinoXSections_H
#define EwkinoXSections_H

//include c++ library classes
#include <map>
#include <string>

class EwkinoXSections{
    
    public:
        EwkinoXSections( const std::string& xSectionTablePath, const bool ZToLL = false, const bool WToLL = false );

        double xSection( unsigned mass ) const;

    private:
        std::map< unsigned, double > massToXSec;


};
#endif
