#include "../interface/Event.h"


//include other pars of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../constants/particleMasses.h"


Event::Event( const TreeReader& treeReader, 
		const bool readIndividualTriggers , const bool readIndividualMetFilters,
		const bool readAllJECVariations, const bool readGroupedJECVariations,
                const bool readParticleLevel ) :
    // make collections of physics objects
    _leptonCollectionPtr( new LeptonCollection( treeReader ) ),
    _jetCollectionPtr( new JetCollection( treeReader,
			readAllJECVariations, readGroupedJECVariations ) ),
    _metPtr( new Met( treeReader,
			readAllJECVariations, readGroupedJECVariations ) ),
    // make additional information structures
    _triggerInfoPtr( new TriggerInfo( treeReader, 
			readIndividualTriggers, readIndividualMetFilters ) ),
    _jetInfoPtr( new JetInfo( treeReader, 
			readAllJECVariations, readGroupedJECVariations ) ),
    _eventTagsPtr( new EventTags( treeReader ) ),
    _generatorInfoPtr( treeReader.isMC() ? new GeneratorInfo( treeReader ) : nullptr ),
    _susyMassInfoPtr( treeReader.isSusy() ? new SusyMassInfo( treeReader ) : nullptr ),
    // make collections of particle level physics objects
    _leptonParticleLevelCollectionPtr( (readParticleLevel && treeReader.containsParticleLevel()) ?
                                    new LeptonParticleLevelCollection( treeReader ) :
                                    nullptr ),
    _jetParticleLevelCollectionPtr( (readParticleLevel && treeReader.containsParticleLevel()) ?
                                    new JetParticleLevelCollection( treeReader ) :
                                    nullptr ),
    _metParticleLevelPtr( (readParticleLevel && treeReader.containsParticleLevel()) ?
                                    new MetParticleLevel( treeReader ) : nullptr ),
    // make additional information structures
    _numberOfVertices( treeReader._nVertex ),
    // WARNING : use treeReader::_scaledWeight instead of treeReader::_weight 
    // since the former already includes cross-section and lumiosity scaling
    _weight( treeReader._scaledWeight ),
    _samplePtr( treeReader.currentSamplePtr() )
    {}


Event::~Event(){
    delete _leptonCollectionPtr;
    delete _jetCollectionPtr;
    delete _metPtr;
    delete _triggerInfoPtr;
    delete _jetInfoPtr;
    delete _eventTagsPtr;
    if( hasGeneratorInfo() ){
        delete _generatorInfoPtr;
    }
    if( hasSusyMassInfo() ){
        delete _susyMassInfoPtr;
    }
    if( hasParticleLevel() ){
	delete _leptonParticleLevelCollectionPtr;
	delete _jetParticleLevelCollectionPtr;
	delete _metParticleLevelPtr;
    }
}


Event::Event( const Event& rhs ) :
    _leptonCollectionPtr( new LeptonCollection( *rhs._leptonCollectionPtr ) ),
    _jetCollectionPtr( new JetCollection( *rhs._jetCollectionPtr ) ),
    _metPtr( new Met( *rhs._metPtr ) ),
    _triggerInfoPtr( new TriggerInfo( *rhs._triggerInfoPtr ) ),
    _jetInfoPtr( new JetInfo( *rhs._jetInfoPtr ) ),
    _eventTagsPtr( new EventTags( *rhs._eventTagsPtr ) ),
    _generatorInfoPtr( rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr ),
    _susyMassInfoPtr( rhs.hasSusyMassInfo() ? new SusyMassInfo( *rhs._susyMassInfoPtr ) : nullptr ),
    _leptonParticleLevelCollectionPtr( rhs.hasParticleLevel() ?
        new LeptonParticleLevelCollection( *rhs._leptonParticleLevelCollectionPtr ) : nullptr ),
    _jetParticleLevelCollectionPtr( rhs.hasParticleLevel() ?
        new JetParticleLevelCollection( *rhs._jetParticleLevelCollectionPtr ) : nullptr ),
    _metParticleLevelPtr( rhs.hasParticleLevel() ?
        new MetParticleLevel( *rhs._metParticleLevelPtr ) : nullptr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
    {}


Event::Event( Event&& rhs ) noexcept :
    _leptonCollectionPtr( rhs._leptonCollectionPtr ),
    _jetCollectionPtr( rhs._jetCollectionPtr ),
    _metPtr( rhs._metPtr ),
    _triggerInfoPtr( rhs._triggerInfoPtr ),
    _jetInfoPtr( rhs._jetInfoPtr ),
    _eventTagsPtr( rhs._eventTagsPtr ),
    _generatorInfoPtr( rhs._generatorInfoPtr ),
    _susyMassInfoPtr( rhs._susyMassInfoPtr ),
    _leptonParticleLevelCollectionPtr( rhs._leptonParticleLevelCollectionPtr ),
    _jetParticleLevelCollectionPtr( rhs._jetParticleLevelCollectionPtr ),
    _metParticleLevelPtr( rhs._metParticleLevelPtr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
{
    rhs._leptonCollectionPtr = nullptr;
    rhs._jetCollectionPtr = nullptr;
    rhs._metPtr = nullptr;
    rhs._triggerInfoPtr = nullptr;
    rhs._jetInfoPtr = nullptr;
    rhs._eventTagsPtr = nullptr;
    rhs._generatorInfoPtr = nullptr;
    rhs._susyMassInfoPtr = nullptr;
    rhs._leptonParticleLevelCollectionPtr = nullptr;
    rhs._jetParticleLevelCollectionPtr = nullptr;
    rhs._metParticleLevelPtr = nullptr;
    rhs._samplePtr = nullptr;
}
    

Event& Event::operator=( const Event& rhs ){
    if( this != &rhs ){
        delete _leptonCollectionPtr;
        delete _jetCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
	delete _jetInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
        }
        if( hasSusyMassInfo() ){
            delete _susyMassInfoPtr;
        }
	if( hasParticleLevel() ){
	    delete _leptonParticleLevelCollectionPtr;
	    delete _jetParticleLevelCollectionPtr;
	    delete _metParticleLevelPtr;
	}

        _leptonCollectionPtr = new LeptonCollection( *rhs._leptonCollectionPtr );
        _jetCollectionPtr = new JetCollection( *rhs._jetCollectionPtr );
        _metPtr = new Met( *rhs._metPtr );
        _triggerInfoPtr = new TriggerInfo( *rhs._triggerInfoPtr );
	_jetInfoPtr = new JetInfo( *rhs._jetInfoPtr );
        _eventTagsPtr = new EventTags( *rhs._eventTagsPtr );
        _generatorInfoPtr = rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr;
        _susyMassInfoPtr = rhs.hasSusyMassInfo() ? new SusyMassInfo( *rhs._susyMassInfoPtr ) : nullptr;
	_leptonParticleLevelCollectionPtr = rhs.hasParticleLevel() ? 
	    new LeptonParticleLevelCollection( *rhs._leptonParticleLevelCollectionPtr ) : nullptr;
	_jetParticleLevelCollectionPtr = rhs.hasParticleLevel() ?
            new JetParticleLevelCollection( *rhs._jetParticleLevelCollectionPtr ) : nullptr;
	_metParticleLevelPtr = rhs.hasParticleLevel() ?
            new MetParticleLevel( *rhs._metParticleLevelPtr ) : nullptr;
        _numberOfVertices = rhs._numberOfVertices;
        _weight = rhs._weight;
        _samplePtr = rhs._samplePtr;
    }
    return *this;
}


Event& Event::operator=( Event&& rhs ) noexcept{
    if( this != &rhs ){
        delete _leptonCollectionPtr;
        delete _jetCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
	delete _jetInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
        }
        if( hasSusyMassInfo() ){
            delete _susyMassInfoPtr;
        }
	if( hasParticleLevel() ){
            delete _leptonParticleLevelCollectionPtr;
            delete _jetParticleLevelCollectionPtr;
            delete _metParticleLevelPtr;
        }

        _leptonCollectionPtr = rhs._leptonCollectionPtr;
        rhs._leptonCollectionPtr = nullptr;
        _jetCollectionPtr = rhs._jetCollectionPtr;
        rhs._jetCollectionPtr = nullptr;
        _metPtr = rhs._metPtr;
        rhs._metPtr = nullptr;
        _triggerInfoPtr = rhs._triggerInfoPtr;
        rhs._triggerInfoPtr = nullptr;
	_jetInfoPtr = rhs._jetInfoPtr;
	rhs._jetInfoPtr = nullptr;
        _eventTagsPtr = rhs._eventTagsPtr;
        rhs._eventTagsPtr = nullptr;
        _generatorInfoPtr = rhs._generatorInfoPtr;
        rhs._generatorInfoPtr = nullptr;
        _susyMassInfoPtr = rhs._susyMassInfoPtr;
        rhs._susyMassInfoPtr = nullptr;
	_leptonParticleLevelCollectionPtr = rhs._leptonParticleLevelCollectionPtr;
	rhs._leptonParticleLevelCollectionPtr = nullptr;
	_jetParticleLevelCollectionPtr = rhs._jetParticleLevelCollectionPtr;
        rhs._jetParticleLevelCollectionPtr = nullptr;
	_metParticleLevelPtr = rhs._metParticleLevelPtr;
        rhs._metParticleLevelPtr = nullptr;
        _numberOfVertices = rhs._numberOfVertices;
        _weight = rhs._weight;
        _samplePtr = rhs._samplePtr;
    }
    return *this;
}


void Event::checkGeneratorInfo() const{
    if( !hasGeneratorInfo() ){
        throw std::domain_error( "Trying to access generator information for a data event!" );
    }
}


GeneratorInfo& Event::generatorInfo() const{
    checkGeneratorInfo();
    return *_generatorInfoPtr;
}


void Event::checkSusyMassInfo() const{
    if( !hasSusyMassInfo() ){
        throw std::domain_error( "Trying to access SUSY mass info for a non-SUSY event!" );
    }
}


SusyMassInfo& Event::susyMassInfo() const{
    checkSusyMassInfo();
    return *_susyMassInfoPtr;
}

void Event::checkParticleLevel() const{
    if( !hasParticleLevel() ){
	throw std::domain_error( "Trying to access particle level information which is not there." );
    }
}

LeptonParticleLevelCollection& Event::leptonParticleLevelCollection() const{
    checkParticleLevel();
    return *_leptonParticleLevelCollectionPtr;
}

JetParticleLevelCollection& Event::jetParticleLevelCollection() const{
    checkParticleLevel();
    return *_jetParticleLevelCollectionPtr;
}

MetParticleLevel& Event::metParticleLevel() const{
    checkParticleLevel();
    return *_metParticleLevelPtr;
}

void Event::initializeZBosonCandidate(bool allowSameSign){
    // reconstruct the best Z boson
    
    // first check if initialization was already done
    // to avoid unnecessary duplicate calculations
    if( !ZIsInitialized ){
        // sort leptons to have consistent indices in later steps
	// (e.g. for determining leading lepton that does not come from Z-decay)
        sortLeptonsByPt();
        // reconstruct the best Z boson using function in LeptonCollection
        std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > ZBosonCandidateIndicesAndMass = _leptonCollectionPtr->bestZBosonCandidateIndicesAndMass(allowSameSign);
        _bestZBosonCandidateIndices = ZBosonCandidateIndicesAndMass.first;
        _bestZBosonCandidateMass = ZBosonCandidateIndicesAndMass.second;
	// get the leading other lepton
        // (note that this can also be a tau in this case!)
        if( numberOfLeptons() >= 3 ){
            for( LeptonCollection::size_type leptonIndex = 0; leptonIndex < numberOfLeptons(); ++leptonIndex ){
                if( !( leptonIndex == _bestZBosonCandidateIndices.first || leptonIndex == _bestZBosonCandidateIndices.second ) ){
                    _WLeptonIndex = leptonIndex;
                    break;
                }
            }
        }
	// update the ZIsInitialized flag
        ZIsInitialized = true;
    }
}


std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > Event::bestZBosonCandidateIndicesAndMass(bool allowSameSign){
    initializeZBosonCandidate(allowSameSign);
    return { _bestZBosonCandidateIndices, _bestZBosonCandidateMass };
}


std::pair< LeptonCollection::size_type, LeptonCollection::size_type > Event::bestZBosonCandidateIndices(bool allowSameSign){
    initializeZBosonCandidate(allowSameSign);
    return _bestZBosonCandidateIndices;
}


double Event::bestZBosonCandidateMass(bool allowSameSign){
    initializeZBosonCandidate(allowSameSign);
    return _bestZBosonCandidateMass;
}


bool Event::hasZTollCandidate( const double oneSidedMassWindow, bool allowSameSign ){
    initializeZBosonCandidate(allowSameSign);
    return ( fabs( bestZBosonCandidateMass(allowSameSign) - particle::mZ ) < oneSidedMassWindow );
}


LeptonCollection::size_type Event::WLeptonIndex(){
    initializeZBosonCandidate();
    return _WLeptonIndex;
}


double Event::mtW(){
    initializeZBosonCandidate();
    return mt( WLepton(), met() );
}

// copy event with modified energy/momentum scales
// experimental stage! need to check what attributes are copied or modified in-place!

void Event::setLeptonCollection( const LeptonCollection& lepCollection ){
    _leptonCollectionPtr = new LeptonCollection( lepCollection);
}

Event Event::variedLeptonCollectionEvent(
		    LeptonCollection (LeptonCollection::*variedCollection)() const ) const{
    Event newevt = Event( *this );
    LeptonCollection lepCollection = (this->leptonCollection().*variedCollection)();
    //newevt._leptonCollectionPtr = new LeptonCollection( lepCollection );
    // (the above does not work since _leptonCollectionPtr is private,
    //  try to solve by defining a public function doing the same thing, but not optimal.)
    newevt.setLeptonCollection( lepCollection );  
    return newevt;
}

Event Event::electronScaleUpEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronScaleUpCollection );
}

Event Event::electronScaleDownEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronScaleDownCollection );
}

Event Event::electronResUpEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronResUpCollection );
}

Event Event::electronResDownEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronResDownCollection );
}

