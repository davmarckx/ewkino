#include "../interface/Jet.h"

//include c++ library classes 
#include <cmath>
#include <stdexcept>
#include <string>

//include other parts of framework
#include "../interface/JetSelector.h"


Jet::Jet( const TreeReader& treeReader, const unsigned jetIndex,
	    const bool readAllJECVariations, const bool readGroupedJECVariations ):
    PhysicsObject( 
        treeReader._jetSmearedPt[jetIndex], 
        treeReader._jetEta[jetIndex], 
        treeReader._jetPhi[jetIndex], 
        treeReader._jetE[jetIndex] * ( treeReader._jetSmearedPt[jetIndex] / treeReader._jetPt[jetIndex] ),
        treeReader.is2016(),
	treeReader.is2016PreVFP(),
	treeReader.is2016PostVFP(), 
        treeReader.is2017(),
	treeReader.is2018()
    ),
    _deepCSV( treeReader._jetDeepCsv_b[jetIndex] 
		+ treeReader._jetDeepCsv_bb[jetIndex] ),
    _deepFlavor( treeReader._jetDeepFlavor_b[jetIndex] 
		+ treeReader._jetDeepFlavor_bb[jetIndex] 
		+ treeReader._jetDeepFlavor_lepb[jetIndex] ),
    _isTight( treeReader._jetIsTight[jetIndex] ),
    _isTightLeptonVeto( treeReader._jetIsTightLepVeto[jetIndex] ),

    //WARNING : is hadron flavor defined for jets in data?
    _hadronFlavor( treeReader._jetHadronFlavor[jetIndex] ),
    _pt_JECDown( treeReader._jetSmearedPt_JECDown[jetIndex] ),
    _pt_JECUp( treeReader._jetSmearedPt_JECUp[jetIndex] ),
    _pt_JERDown( treeReader._jetSmearedPt_JERDown[jetIndex] ),
    _pt_JERUp( treeReader._jetSmearedPt_JERUp[jetIndex] ),
    selector( new JetSelector( this ) )
{
    if( readAllJECVariations ){
	for( const auto mapEl: treeReader._jetSmearedPt_JECSourcesUp ){
	    std::string key = mapEl.first;
	    key = stringTools::removeOccurencesOf(key,"_jetSmearedPt_");
	    key = stringTools::removeOccurencesOf(key,"_JECSourcesUp");
	    _pt_JECSourcesUp.insert( {key,mapEl.second[jetIndex]} );
	}
	for( const auto mapEl: treeReader._jetSmearedPt_JECSourcesDown ){
	    std::string key = mapEl.first;
	    key = stringTools::removeOccurencesOf(key,"_jetSmearedPt_");
	    key = stringTools::removeOccurencesOf(key,"_JECSourcesDown");
	    _pt_JECSourcesDown.insert( {key,mapEl.second[jetIndex]} );
	}
    }
    if( readGroupedJECVariations ){
	for( const auto mapEl: treeReader._jetSmearedPt_JECGroupedUp ){
	    std::string key = mapEl.first;
	    key = stringTools::removeOccurencesOf(key,"_jetSmearedPt_");
	    key = stringTools::removeOccurencesOf(key,"_JECGroupedUp");
	    _pt_JECGroupedUp.insert( {key,mapEl.second[jetIndex]} );
	}
	for( const auto mapEl: treeReader._jetSmearedPt_JECGroupedDown ){
	    std::string key = mapEl.first;
	    key = stringTools::removeOccurencesOf(key,"_jetSmearedPt_");
	    key = stringTools::removeOccurencesOf(key,"_JECGroupedDown");
	    _pt_JECGroupedDown.insert( {key,mapEl.second[jetIndex]} );
	}
    }

    //catch potential invalid values of deepCSV and deepFlavor
    if( std::isnan( _deepCSV ) ){
        _deepCSV = 0.;

    //set minimum value to 0 to avoid default values
    } else if( _deepCSV < 0 ){
        _deepCSV = 0.;
    }

    if( std::isnan( _deepFlavor ) ){
        _deepFlavor = 0.;
    } else if( _deepFlavor < 0 ){
        _deepFlavor = 0.;
    }

    //check that _hadronFlavor has a known value
    if( ! ( ( _hadronFlavor == 0 ) || ( _hadronFlavor == 4 ) || ( _hadronFlavor == 5 ) ) ){
        throw std::invalid_argument( "jet hadronFlavor is '" + std::to_string( _hadronFlavor ) + "' while it should be 0, 4 or 5." );
    }
}


Jet::Jet( const Jet& rhs ) : 
    PhysicsObject( rhs ),
    _deepCSV( rhs._deepCSV ),
    _deepFlavor( rhs._deepFlavor ),
    _isTight( rhs._isTight ),
    _isTightLeptonVeto( rhs._isTightLeptonVeto ),
    _hadronFlavor( rhs._hadronFlavor ),
    _pt_JECDown( rhs._pt_JECDown ),
    _pt_JECUp( rhs._pt_JECUp ),
    _pt_JERDown( rhs._pt_JERDown ),
    _pt_JERUp( rhs._pt_JERUp ),
    _pt_JECSourcesUp( rhs._pt_JECSourcesUp ),
    _pt_JECSourcesDown( rhs._pt_JECSourcesDown ),
    _pt_JECGroupedUp( rhs._pt_JECGroupedUp ),
    _pt_JECGroupedDown( rhs._pt_JECGroupedDown ),
    selector( new JetSelector( this ) )
    {}


Jet::Jet( Jet&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
	_deepCSV( rhs._deepCSV ),
    _deepFlavor( rhs._deepFlavor ),
    _isTight( rhs._isTight ),
    _isTightLeptonVeto( rhs._isTightLeptonVeto ),
    _hadronFlavor( rhs._hadronFlavor ),
    _pt_JECDown( rhs._pt_JECDown ),
    _pt_JECUp( rhs._pt_JECUp ),
    _pt_JERDown( rhs._pt_JERDown ),
    _pt_JERUp( rhs._pt_JERUp ),
    _pt_JECSourcesUp( rhs._pt_JECSourcesUp ),
    _pt_JECSourcesDown( rhs._pt_JECSourcesDown ),
    _pt_JECGroupedUp( rhs._pt_JECGroupedUp ),
    _pt_JECGroupedDown( rhs._pt_JECGroupedDown ),
    selector( new JetSelector( this ) )
    {}


Jet::~Jet(){
    delete selector;
}


void Jet::copyNonPointerAttributes( const Jet& rhs ){
    _deepCSV = rhs._deepCSV;
    _deepFlavor = rhs._deepFlavor;
    _isTight = rhs._isTight;
    _isTightLeptonVeto = rhs._isTightLeptonVeto;
    _hadronFlavor = rhs._hadronFlavor;
    _pt_JECDown = rhs._pt_JECDown;
    _pt_JECUp = rhs._pt_JECUp;
    _pt_JERDown = rhs._pt_JERDown;
    _pt_JERUp = rhs._pt_JERUp;
    _pt_JECSourcesUp = rhs._pt_JECSourcesUp;
    _pt_JECSourcesDown = rhs._pt_JECSourcesDown;
    _pt_JECGroupedUp = rhs._pt_JECGroupedUp;
    _pt_JECGroupedDown = rhs._pt_JECGroupedDown;
}


Jet& Jet::operator=( const Jet& rhs ){

    PhysicsObject::operator=(rhs);

    //note that the jet selector does not have to be changed, it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


Jet& Jet::operator=( Jet&& rhs ) noexcept {

    PhysicsObject::operator=( std::move(rhs) );

    if( this != &rhs ){
        copyNonPointerAttributes( rhs );

        //current selector can still keep pointing to this object
    }
    return *this;
}


Jet Jet::variedJet(const double newPt) const{
    Jet variedJet( *this );
    variedJet.setLorentzVector( newPt, eta(), phi(), energy()*( newPt / pt() ) );
    return variedJet;
} 


Jet Jet::JetJECDown() const{
    return variedJet( _pt_JECDown );
}


Jet Jet::JetJECUp() const{
    return variedJet( _pt_JECUp );
}


Jet Jet::JetJERDown() const{
    return variedJet( _pt_JERDown );
}


Jet Jet::JetJERUp() const{
    return variedJet( _pt_JERUp );
}


Jet Jet::JetJECDown( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    // first check case of empty string (total combined JEC)
    if( source.size()==0 ){ return this->JetJECDown(); }
    double newpt = 0.;
    bool found = false;
    for( auto mapEl: this->_pt_JECSourcesDown ){
	if(source==mapEl.first){ 
	    newpt = mapEl.second;
	    found = true; 
	}
    }
    for( auto mapEl: this->_pt_JECGroupedDown ){
        if(source==mapEl.first){
	    newpt = mapEl.second;
	    found = true;
	}
    }
    if( !found ){
	std::string msg = "ERROR in Jet.JetJECDown: JEC source " + source;
	msg.append( " not recognized." );
	throw std::runtime_error( msg );
    }
    return variedJet( newpt );
}


Jet Jet::JetJECUp( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    // first check case of empty string (total combined JEC)
    if( source.size()==0 ){ return this->JetJECUp(); }
    double newpt = 0.;
    bool found = false;
    for( auto mapEl: this->_pt_JECSourcesUp ){
        if(source==mapEl.first){
	    newpt = mapEl.second;
	    found = true;
	}
    }
    for( auto mapEl: this->_pt_JECGroupedUp ){
        if(source==mapEl.first){
	    newpt = mapEl.second;
	    found = true;
	}
    }
    if( !found ){
        std::string msg = "ERROR in Jet.JetJECUp: JEC source " + source;
        msg.append( " not recognized." );
        throw std::runtime_error( msg );
    }

    return variedJet( newpt );
}


bool Jet::isGood() const{
    return selector->isGood();
}


bool Jet::isGoodAnyVariation() const{
    return (
        isGood() ||
        JetJECDown().isGood() ||
        JetJECUp().isGood() ||
        JetJERDown().isGood() ||
        JetJERUp().isGood()
    );
}


bool Jet::inBTagAcceptance() const{
    return selector->inBTagAcceptance();
}


bool Jet::isBTaggedLoose() const{
    return selector->isBTaggedLoose();
}


bool Jet::isBTaggedMedium() const{
    return selector->isBTaggedMedium();
}


bool Jet::isBTaggedTight() const{
    return selector->isBTaggedTight();
}


bool Jet::isBTaggedAnyVariation( bool ( Jet::*isBTaggedWP )() const ) const{
    return (
        ( *this.*isBTaggedWP )() ||
        ( JetJECDown().*isBTaggedWP )() ||
        ( JetJECUp().*isBTaggedWP )() ||
        ( JetJERDown().*isBTaggedWP )() ||
        ( JetJERUp().*isBTaggedWP )()
    );
}


bool Jet::isBTaggedLooseAnyVariation() const{
    return isBTaggedAnyVariation( &Jet::isBTaggedLoose );
}


bool Jet::isBTaggedMediumAnyVariation() const{
    return isBTaggedAnyVariation( &Jet::isBTaggedMedium );
}


bool Jet::isBTaggedTightAnyVariation() const{
    return isBTaggedAnyVariation( &Jet::isBTaggedTight );
}


std::ostream& Jet::print( std::ostream& os ) const{
    os << "Jet : ";
    PhysicsObject::print( os );
    os << " / deepCSV = " << _deepCSV << " / deepFlavor = " << _deepFlavor << " / isTight = " << _isTight << " / isTightLeptonVeto = " << _isTightLeptonVeto << " / hadronFlavor = " << _hadronFlavor << " / pt_JECDown = " << _pt_JECDown << " / pt_JECUp = " << _pt_JECUp;
    return os;
}
