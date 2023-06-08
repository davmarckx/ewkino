#include "../interface/JetParticleLevel.h"

//include c++ library classes 
#include <cmath>
#include <stdexcept>
#include <string>


JetParticleLevel::JetParticleLevel( const TreeReader& treeReader, const unsigned index ):
    PhysicsObject( 
        treeReader._pl_jetPt[index], 
        treeReader._pl_jetEta[index], 
        treeReader._pl_jetPhi[index], 
        treeReader._pl_jetE[index],
        treeReader.is2016(),
	treeReader.is2016PreVFP(),
	treeReader.is2016PostVFP(), 
        treeReader.is2017(),
	treeReader.is2018()
    ),
    _hadronFlavor( treeReader._pl_jetHadronFlavor[index] )
{
    // check that _hadronFlavor has a known value
    if( ! ( ( _hadronFlavor == 0 ) || ( _hadronFlavor == 4 ) || ( _hadronFlavor == 5 ) ) ){
	std::string msg = "ERROR in JetParticleLevel constructor:";
        msg += " jet hadronFlavor is '" + std::to_string( _hadronFlavor ) + "' while it should be 0, 4 or 5.";
        throw std::invalid_argument( msg );
    }
}


JetParticleLevel::JetParticleLevel( const JetParticleLevel& rhs ) : 
    PhysicsObject( rhs ),
    _hadronFlavor( rhs._hadronFlavor )
    {}


JetParticleLevel::JetParticleLevel( JetParticleLevel&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _hadronFlavor( rhs._hadronFlavor )
    {}


JetParticleLevel::~JetParticleLevel(){
}


void JetParticleLevel::copyNonPointerAttributes( const JetParticleLevel& rhs ){
    _hadronFlavor = rhs._hadronFlavor;
}


JetParticleLevel& JetParticleLevel::operator=( const JetParticleLevel& rhs ){
    PhysicsObject::operator=(rhs);
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


JetParticleLevel& JetParticleLevel::operator=( JetParticleLevel&& rhs ) noexcept {
    PhysicsObject::operator=( std::move(rhs) );
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


bool JetParticleLevel::isGood() const{
    // basic selections on particle level jets.
    // note: contrary to detector-level jets, this is not implemented in a
    //       dedicated JetSelector class, because particle level selection
    //       is supposed to be really simple and non-changing.
    // note: for jets, all relevant cuts should already be in the ntuplizer,
    //       but repeat here for safety if that should change.
    if( pt() < 25. ) return false;
    if( std::fabs(eta()) > 2.4 ) return false;
    return true;
}


std::ostream& JetParticleLevel::print( std::ostream& os ) const{
    os << "Jet : ";
    PhysicsObject::print( os );
    os << " / hadronFlavor = " << _hadronFlavor;
    return os;
}
