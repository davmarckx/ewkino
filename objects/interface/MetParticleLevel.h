#ifndef MetParticleLevel_H
#define MetParticleLevel_H

// include c++ libraries
#include <math.h>

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../TreeReader/interface/NanoGenTreeReader.h"

class MetParticleLevel : public PhysicsObject {

    public:
        MetParticleLevel() = default;
	// constructor from TreeReader
        MetParticleLevel( const TreeReader& );
	// constructor from NanoGenTreeReader
	MetParticleLevel( const NanoGenTreeReader& );

        virtual std::ostream& print( std::ostream& ) const override;

    private:

};
#endif 
