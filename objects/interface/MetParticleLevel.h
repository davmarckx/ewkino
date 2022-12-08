#ifndef MetParticleLevel_H
#define MetParticleLevel_H

// include c++ libraries
#include <math.h>

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"

class MetParticleLevel : public PhysicsObject {

    public:
        MetParticleLevel() = default;
        MetParticleLevel( const TreeReader& );

        virtual std::ostream& print( std::ostream& ) const override;

    private:

};
#endif 
