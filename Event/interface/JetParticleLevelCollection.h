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

        //make jet collection with b-tagged jets 
        JetParticleLevelCollection PLbJetCollection() const;

	// count objects
	size_type numberOfJets() const;
	size_type numberOfBJets() const;

	// sorting
	void sortByPt();

    private:
       //build JetCollection of jets satisfying a certain requirement
       JetParticleLevelCollection buildSubCollection( bool (JetParticleLevel::*passSelection)() const ) const; 
       JetParticleLevelCollection( const std::vector< std::shared_ptr< JetParticleLevel > >& jetVector ) : PhysicsObjectCollection< JetParticleLevel >( jetVector ) {}
};


#endif 
