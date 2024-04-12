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


bool LeptonParticleLevel::isGood() const{
    // basic selections on particle level leptons.
    // note: contrary to detector-level leptons, this is not implemented in a
    //       dedicated LeptonSelector class, because particle level selection
    //       is supposed to be really simple and non-changing.
    if( isTau() ) return false; // do not consider taus for now.
    if( pt() < 10. ) return false;
    if( isElectron() && std::fabs(eta()) > 2.5 ) return false;
    if( isMuon() && std::fabs(eta()) > 2.4 ) return false; // default, synced with Oviedo
    //if( isMuon() && std::fabs(eta()) > 2.5 ) return false; // synced with LHCHWG note
    return true;
}


bool LeptonParticleLevel::sameFlavor( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( 
        ( lhs.isMuon() && rhs.isMuon() ) ||
        ( lhs.isElectron() && rhs.isElectron() ) ||
        ( lhs.isTau() && rhs.isTau() )
    );
}


bool LeptonParticleLevel::differentFlavor( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return !sameFlavor(lhs, rhs);
}


bool LeptonParticleLevel::sameSign( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( lhs.charge() == rhs.charge() );
}


bool LeptonParticleLevel::oppositeSign( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( lhs.charge() != rhs.charge() );
}


bool LeptonParticleLevel::oppositeSignSameFlavor( const LeptonParticleLevel& lhs, const LeptonParticleLevel& rhs ){
    return ( oppositeSign(lhs, rhs) && sameFlavor(lhs, rhs) );
}


std::ostream& LeptonParticleLevel::print( std::ostream& os) const{
    PhysicsObject::print( os );
    os << " / charge = " << ( _charge > 0 ? "+" : "-" );
    os << " / flavor = " << _flavor;
    return os;
}
