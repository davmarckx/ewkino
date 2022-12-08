#include "../interface/LeptonParticleLevel.h"

//include c++ library classes
#include <utility>
#include <stdexcept>


LeptonParticleLevel::LeptonParticleLevel( 
    const TreeReader& treeReader, const unsigned index ) :
    PhysicsObject( treeReader._pl_lPt[index], treeReader._pl_lEta[index], 
		    treeReader._pl_lPhi[index], treeReader._pl_lE[index], 
		    treeReader.is2016(), 
		    treeReader.is2016PreVFP(),
		    treeReader.is2016PostVFP(),
		    treeReader.is2017(),
		    treeReader.is2018() ),
    _charge( treeReader._pl_lCharge[index] ),
    _flavor( treeReader._pl_lFlavor[index] )
    {}


LeptonParticleLevel::LeptonParticleLevel( const LeptonParticleLevel& rhs ) :
    PhysicsObject( rhs ),
    _charge( rhs._charge ),
    _flavor( rhs._flavor )
    {}


LeptonParticleLevel::LeptonParticleLevel( LeptonParticleLevel&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _charge( rhs._charge ),
    _flavor( rhs._flavor )
    {}


LeptonParticleLevel::~LeptonParticleLevel(){
}


void LeptonParticleLevel::copyNonPointerAttributes( const LeptonParticleLevel& rhs ){
    _charge = rhs._charge;
    _flavor = rhs._flavor;
}


LeptonParticleLevel& LeptonParticleLevel::operator=( const LeptonParticleLevel& rhs ){
    //copy the PhysicsObject part of the Lepton
    PhysicsObject::operator=(rhs); 
    //copy non pointer members
    copyNonPointerAttributes( rhs );
    return *this;
}


LeptonParticleLevel& LeptonParticleLevel::operator=( LeptonParticleLevel&& rhs ) noexcept{
    //move the PhysicsObject part of the lepton
    PhysicsObject::operator=( std::move(rhs) );
    //in case of self assignment the move assignment should do no work
    if( this != &rhs ){
        //copy non pointer members
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


bool sameFlavor( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( 
        ( lhs.isMuon() && rhs.isMuon() ) ||
        ( lhs.isElectron() && rhs.isElectron() ) ||
        ( lhs.isTau() && rhs.isTau() )
    );
}


bool oppositeSign( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( lhs.charge() != rhs.charge() );
}


std::ostream& LeptonParticleLevel::print( std::ostream& os) const{
    PhysicsObject::print( os );
    os << " / charge = " << ( _charge > 0 ? "+" : "-" );
    os << " / flavor = " << _flavor;
    return os;
}
