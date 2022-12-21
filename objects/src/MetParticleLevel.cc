#include "../interface/MetParticleLevel.h"


MetParticleLevel::MetParticleLevel( const TreeReader& treeReader ):
    PhysicsObject( treeReader._pl_met, 0., treeReader._pl_metPhi, treeReader._pl_met,
		    treeReader.is2016(), treeReader.is2016PreVFP(), treeReader.is2016PostVFP(),
		    treeReader.is2017(), treeReader.is2018() )
    {}


std::ostream& MetParticleLevel::print( std::ostream& os ) const{
    os << "Met : ";
    os << "(pt = " << pt() << ", phi = " << phi() << ")";
    return os;
}
