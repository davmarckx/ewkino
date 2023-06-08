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

	// count objects
	size_type numberOfLeptons() const;
	size_type numberOfMuons() const;
	size_type numberOfElectrons() const;
	size_type numberOfTaus() const;
	size_type numberOfLightLeptons() const;

	// check pair presence
	bool hasSameFlavorPair();
	bool hasDifferentFlavorPair();
	bool hasSameSignPair();
	bool hasOppositeSignPair();
	bool hasOppositeSignSameFlavorPair();

	// select objects
	void selectGoodLeptons();
	void removeTaus();

	// sorting
	void sortByPt();

	// Z boson reconstruction
	bool hasZTollCandidate(double halfWindow, 
                               bool allowSameSign = false,
                               bool force = false);

    private:

	bool _ZInitialized = false;
	bool _ZValid = false;
	std::pair< size_type, size_type > _bestZBosonCandidateIndices = {99, 99};
        double _bestZBosonCandidateMass = 0;
        void initializeZBosonCandidate(bool allowSameSign = false, bool force = false);

};


#endif
