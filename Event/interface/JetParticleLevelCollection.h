#ifndef JetParticleLevelCollection_H
#define JetParticleLevelCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>

//include other parts of framework
#include "../../objects/interface/JetParticleLevel.h"
#include "../../objects/interface/LeptonParticleLevel.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../interface/LeptonParticleLevelCollection.h"
#include "PhysicsObjectCollection.h"


class LeptonParticleLevelCollection;

class JetParticleLevelCollection : public PhysicsObjectCollection< JetParticleLevel > {

    public:
        JetParticleLevelCollection( const TreeReader& );

    private:
        
};


#endif 
