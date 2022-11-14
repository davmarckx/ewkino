#include "../interface/Lepton.h"

//include c++ library classes
#include <utility>
#include <stdexcept>


Lepton::Lepton( const TreeReader& treeReader, const unsigned leptonIndex, LeptonSelector* leptonSelector ) :
    PhysicsObject( treeReader._lPt[leptonIndex], treeReader._lEta[leptonIndex], 
		    treeReader._lPhi[leptonIndex], treeReader._lE[leptonIndex], 
		    treeReader.is2016(), 
		    treeReader.is2016PreVFP(),
		    treeReader.is2016PostVFP(),
		    treeReader.is2017(),
		    treeReader.is2018() ),
    _charge( treeReader._lCharge[leptonIndex] ),
    _dxy( treeReader._dxy[leptonIndex] ),
    _dz( treeReader._dz[leptonIndex] ), 
    _sip3d( treeReader._3dIPSig[leptonIndex] ),
    generatorInfo( treeReader.isMC() ? new LeptonGeneratorInfo( treeReader, leptonIndex ) : nullptr ),
    selector( leptonSelector ),
    _uncorrectedPt( pt() ),
    _uncorrectedE( energy() )
    {}


//Lepton has value-like behavior 
Lepton::Lepton( const Lepton& rhs, LeptonSelector* leptonSelector ) : 
    PhysicsObject( rhs ),
    _charge( rhs._charge ),
    _dxy( rhs._dxy ),
    _dz( rhs._dz ),
    _sip3d( rhs._sip3d ),
    generatorInfo( new LeptonGeneratorInfo( *(rhs.generatorInfo) ) ),

    //WARNING: selector remains uninitialized, and has to be dynamically allocated in derived classes. Final derived copy constructur MUST CREATE A NEW SELECTOR
    selector( leptonSelector ),

    //make sure to copy "isConeCorrected" so a cone-correction can not be re-applied even after copying a lepton
    isConeCorrected( rhs.isConeCorrected ),
    _uncorrectedPt( rhs._uncorrectedPt ),
    _uncorrectedE( rhs._uncorrectedE )
    {}


Lepton::Lepton( Lepton&& rhs, LeptonSelector* leptonSelector ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _charge( rhs._charge ),
    _dxy( rhs._dxy ),
    _dz( rhs._dz ),
    _sip3d( rhs._sip3d ),
    generatorInfo( rhs.generatorInfo ),

    //WARNING: selector remains uninitialized, and has to be dynamically allocated in derived classes. Final derived copy constructur MUST CREATE A NEW SELECTOR
    selector( leptonSelector ),

    //make sure to copy "isConeCorrected" so a cone-correction can not be re-applied even after copying a lepton
    isConeCorrected( rhs.isConeCorrected ),
    _uncorrectedPt( rhs._uncorrectedPt ),
    _uncorrectedE( rhs._uncorrectedE )
{
    rhs.generatorInfo = nullptr;
}


Lepton::~Lepton(){
    delete selector;
    if( hasGeneratorInfo() ){
        delete generatorInfo;
    }
}


void Lepton::copyNonPointerAttributes( const Lepton& rhs ){
    _charge = rhs._charge;
    _dxy = rhs._dxy;
    _dz = rhs._dz;
    _sip3d = rhs._sip3d;

    //make sure to copy "isConeCorrected" so a cone-correction can not be re-applied even after copying a lepton
    isConeCorrected = rhs.isConeCorrected;
    _uncorrectedPt = rhs._uncorrectedPt;
    _uncorrectedE = rhs._uncorrectedE;
}


Lepton& Lepton::operator=( const Lepton& rhs ){

    //copy the PhysicsObject part of the Lepton
    PhysicsObject::operator=(rhs); 

    //copy non pointer members
    copyNonPointerAttributes( rhs );

    //selector can keep pointing to the current lepton and does not need to be copied!
    
    //need to create new LeptonGeneratorInfo object to ensure self assignment works 
    if( hasGeneratorInfo() ){
        LeptonGeneratorInfo* oldInfo = generatorInfo;
        generatorInfo = new LeptonGeneratorInfo( *rhs.generatorInfo );
        delete oldInfo;
    }
    return *this;
}


Lepton& Lepton::operator=( Lepton&& rhs ) noexcept{

    //move the PhysicsObject part of the lepton
    PhysicsObject::operator=( std::move(rhs) );

    //in case of self assignment the move assignment should do no work
    if( this != &rhs ){

        //copy non pointer members
        copyNonPointerAttributes( rhs );

        //selector can keep pointing to the current lepton and does not need to be moved!

        //move pointer to generator information and make sure to free old memory
        delete generatorInfo; 
        generatorInfo = rhs.generatorInfo;
        rhs.generatorInfo = nullptr;
        
    }
    return *this;
}


bool Lepton::checkGeneratorInfo() const{
    if( !hasGeneratorInfo() ){
        throw std::domain_error( "Trying to access generator information for a lepton that has no generator info!" );
    }
    return true;
}


bool Lepton::isPrompt() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->isPrompt();
    } else {
        return true;
    }
}


int Lepton::momPdgId() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->momPdgId(); 
    } else {
        return 0;
    }
}


int Lepton::matchPdgId() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->matchPdgId();
    } else {
        return 0;
    }
}


int Lepton::matchCharge() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->matchCharge();
    } else {
        return 0;
    }
}


unsigned Lepton::provenance() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenance();
    } else {
        return 0;
    }
}


unsigned Lepton::provenanceCompressed() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenanceCompressed();
    } else {
        return 0;
    }
}


unsigned Lepton::provenanceConversion() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenanceConversion();
    } else {
        return 0;
    }
}

bool Lepton::isChargeFlip() const{
    // veto leptons from photon conversions (not counted as charge flips)
    if( matchPdgId()==22 ) return false;
    // check if measured charge differs from generator charge
    return (charge() != matchCharge());
}


void Lepton::applyConeCorrection(){

    //make sure cone correction can only be applied once 
    if( isConeCorrected ) return;
    isConeCorrected = true;

    //only apply cone correction only to leptons that are FO and not tight
    //loose leptons that do not pass FO are not corrected to make sure the object-level pT thresholds in isFO() are respected, and because they are usually used for purposes not requiring a cone correction
    //Tight leptons should never receive a cone correction!
    if( isFO() && !isTight() ){
        double correctionFactor = selector->coneCorrection();
        setLorentzVector( pt()*correctionFactor, eta(), phi(), energy()*correctionFactor );
    }
}


bool sameFlavor( const Lepton& lhs, const Lepton& rhs ){
    return ( 
        ( lhs.isMuon() && rhs.isMuon() ) ||
        ( lhs.isElectron() && rhs.isElectron() ) ||
        ( lhs.isTau() && rhs.isTau() )
    );
}

bool oppositeFlavor( const Lepton& lhs, const Lepton& rhs ){
    return !sameFlavor(lhs, rhs);
}

bool sameSign( const Lepton& lhs, const Lepton& rhs ){
    return ( lhs.charge() == rhs.charge() );
}

bool oppositeSign( const Lepton& lhs, const Lepton& rhs ){
    return !sameSign(lhs, rhs);
}

bool oppositeSignSameFlavor( const Lepton& lhs, const Lepton& rhs ){
    return ( sameFlavor( lhs, rhs ) && ( lhs.charge() != rhs.charge() ) );
}


std::ostream& Lepton::print( std::ostream& os) const{
    PhysicsObject::print( os );
    os << " / charge = " << ( _charge > 0 ? "+" : "-" ) << " / dxy = " << _dxy << " / dz = " << _dz << " / sip3d = " << _sip3d << " / uncorrectedPt = " << _uncorrectedPt;
    return os;
}
