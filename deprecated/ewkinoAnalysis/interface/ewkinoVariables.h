#ifndef ewkinoVariables_H
#define ewkinoVariables_H

//include c++ library classes
#include <string>
#include <map>

//include other parts of framework
#include "../../Event/interface/Event.h"

namespace ewkino{
    std::map< std::string, double > computeVariables( Event& event, const std::string& unc );
}


#endif
