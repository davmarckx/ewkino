#ifndef JetParticleLevel_H
#define JetParticleLevel_H

/*
Definition of particle-level jets
(copied and simplified from Jet.h)
*/

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"

template< typename objectType > class PhysicsObjectCollection;

class JetParticleLevel : public PhysicsObject{
    
    friend class PhysicsObjectCollection< JetParticleLevel >;

    public:
        JetParticleLevel( const TreeReader&, const unsigned );

        JetParticleLevel( const JetParticleLevel& );
        JetParticleLevel( JetParticleLevel&& ) noexcept;

        ~JetParticleLevel();

        JetParticleLevel& operator=( const JetParticleLevel& );
        JetParticleLevel& operator=( JetParticleLevel&& ) noexcept;

        unsigned hadronFlavor() const{ return _hadronFlavor; }
	bool isBJet() const{ return _hadronFlavor==5; }

        virtual std::ostream& print( std::ostream& ) const override;

    private:
        unsigned _hadronFlavor = 0;
        
        JetParticleLevel* clone() const & { return new JetParticleLevel(*this); }
        JetParticleLevel* clone() && { return new JetParticleLevel( std::move( *this ) ); }

        void copyNonPointerAttributes( const JetParticleLevel& );
};

#endif
