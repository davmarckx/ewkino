#ifndef eventFlatteningParticleLevel_H
#define eventFlatteningParticleLevel_H

// include other parts of framework
#include "../../../constants/particleMasses.h"
#include "../../../Event/interface/Event.h"

// function declarations

namespace eventFlatteningParticleLevel{
    std::map< std::string, double > initVarMap();
    std::map< std::string, double > eventToEntry(Event& event);
}

#endif
