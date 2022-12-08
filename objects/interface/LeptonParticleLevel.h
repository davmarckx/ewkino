#ifndef LeptonParticleLevel_H
#define LeptonParticleLevel_H

/* 
Definition of particle-level leptons
(copied and simplified from Lepton.h)
*/

//include c++ library classes 
#include <utility>
#include <memory>

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"

template< typename ObjectType > class PhysicsObjectCollection;

class LeptonParticleLevel : public PhysicsObject {
    
    friend class PhysicsObjectCollection<LeptonParticleLevel>;
    
    public: 
        LeptonParticleLevel( const TreeReader&, unsigned int ); 

	LeptonParticleLevel( const LeptonParticleLevel& );
        LeptonParticleLevel( LeptonParticleLevel&& ) noexcept;

        ~LeptonParticleLevel();
        
        LeptonParticleLevel& operator=( const LeptonParticleLevel& );
        LeptonParticleLevel& operator=( LeptonParticleLevel&& ) noexcept;

        int charge() const{ return _charge; }
	unsigned int flavor() const{ return _flavor; }
        bool isMuon() const{ return _flavor==1; }
        bool isElectron() const{ return _flavor==0; }
        bool isTau() const{ return _flavor==2; }
        bool isLightLepton(){ return (isMuon() || isElectron()); }

        virtual std::ostream& print( std::ostream& os = std::cout ) const override;

    private:
        int _charge = 0;
        unsigned int _flavor = 0;
    
        void copyNonPointerAttributes( const LeptonParticleLevel& );

        LeptonParticleLevel* clone() const & { return new LeptonParticleLevel(*this); }
        LeptonParticleLevel* clone() && { return new LeptonParticleLevel( std::move(*this)); }

};

bool sameFlavor( const LeptonParticleLevel&, const LeptonParticleLevel& );
bool oppositeSign( const LeptonParticleLevel&, const LeptonParticleLevel& );

#endif
