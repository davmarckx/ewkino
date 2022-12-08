#ifndef LeptonParticleLevelCollection_H
#define LeptonParticleLevelCollection_H

//include c++ library classes 
#include <set>

//include other parts of code 
#include "../../constants/particleMasses.h"
#include "../../objects/interface/LeptonParticleLevel.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"


class LeptonParticleLevelCollection : public PhysicsObjectCollection< LeptonParticleLevel > {
    
    public:
        LeptonParticleLevelCollection( const TreeReader& );

    private:

};


#endif
