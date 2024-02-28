#include "../interface/LeptonParticleLevelCollection.h"


LeptonParticleLevelCollection::LeptonParticleLevelCollection( const TreeReader& treeReader ){
    // constructor of a collection of LeptonParticleLevel objects from a TreeReader entry

    // loop over indices up to _pl_nL (number of particle level leptons)
    for( unsigned l = 0; l < treeReader._pl_nL; ++l){
	// create a LeptonParticleLevel object and add to the collection
        push_back( LeptonParticleLevel( treeReader, l ) );
    }
}

// cleaning
void LeptonParticleLevelCollection::cleanLeptonsFromJets(
    const JetParticleLevelCollection& jetCollection,
    const double coneSize ){
    // loop over leptons
    for( const_iterator lIt = cbegin(); lIt != cend(); ){
        LeptonParticleLevel& lepton = **lIt;
        bool isDeleted = false;
        // loop over jets
        for( JetParticleLevelCollection::const_iterator jetIt = jetCollection.cbegin();
             jetIt != jetCollection.cend(); ++jetIt ){
            JetParticleLevel& jet = **jetIt;
            // remove lepton if it overlaps with a jet
            if( deltaR( jet, lepton ) < coneSize ){
                lIt = erase( lIt );
                isDeleted = true;
                break;
            }
        }
        if( !isDeleted ){ ++lIt; }
    }
}

LeptonParticleLevelCollection::size_type LeptonParticleLevelCollection::numberOfLeptons() const{
    // return number of leptons (i.e. size of the collection)
    return size();
}

LeptonParticleLevelCollection::size_type LeptonParticleLevelCollection::numberOfMuons() const{
    // return number of muons in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &LeptonParticleLevel::isMuon );
}

LeptonParticleLevelCollection::size_type LeptonParticleLevelCollection::numberOfElectrons() const{
    // return number of electrons in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &LeptonParticleLevel::isElectron );
}

LeptonParticleLevelCollection::size_type LeptonParticleLevelCollection::numberOfTaus() const{
    // return number of taus in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &LeptonParticleLevel::isTau );
}

LeptonParticleLevelCollection::size_type LeptonParticleLevelCollection::numberOfLightLeptons() const{
    // return number of light leptons in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &LeptonParticleLevel::isLightLepton );
}

bool LeptonParticleLevelCollection::hasSameFlavorPair(){
    // return whether there is a same flavor pair in this collection
    // uses the inerited hasPairWithRequirement method 
    // (from PhysicsObjectCollection)
    return hasPairWithRequirement( &LeptonParticleLevel::sameFlavor );
}

bool LeptonParticleLevelCollection::hasDifferentFlavorPair(){
    // return whether there is a different flavor pair in this collection
    // uses the inerited hasPairWithRequirement method 
    // (from PhysicsObjectCollection)
    return hasPairWithRequirement( &LeptonParticleLevel::differentFlavor );
}

bool LeptonParticleLevelCollection::hasSameSignPair(){
    // return whether there is a same sign pair in this collection
    // uses the inerited hasPairWithRequirement method 
    // (from PhysicsObjectCollection)
    return hasPairWithRequirement( &LeptonParticleLevel::sameSign );
}

bool LeptonParticleLevelCollection::hasOppositeSignPair(){
    // return whether there is an opposite sign pair in this collection
    // uses the inerited hasPairWithRequirement method 
    // (from PhysicsObjectCollection)
    return hasPairWithRequirement( &LeptonParticleLevel::oppositeSign );
}

bool LeptonParticleLevelCollection::hasOppositeSignSameFlavorPair(){
    // return whether there is an OSSF pair in this collection
    // uses the inerited hasPairWithRequirement method 
    // (from PhysicsObjectCollection)
    return hasPairWithRequirement( &LeptonParticleLevel::oppositeSignSameFlavor );
}

void LeptonParticleLevelCollection::removeTaus(){
    // remove taus from the collection
    // uses the inherited selectObjects method 
    // (from PhysicsObjectCollection)
    selectObjects( &LeptonParticleLevel::isLightLepton );
}

void LeptonParticleLevelCollection::selectGoodLeptons(){
    // remove bad leptons from the collection
    // uses the inherited selectObjects method 
    // (from PhysicsObjectCollection)
    selectObjects( &LeptonParticleLevel::isGood );
}

void LeptonParticleLevelCollection::sortByPt(){
    // sort leptons in this collection by pt
    // uses the inherited sortByAttribute method
    // (from PhysicsObjectCollection)
    return sortByAttribute( 
	[](const std::shared_ptr< LeptonParticleLevel >& lhs,
	   const std::shared_ptr< LeptonParticleLevel >& rhs){
	return lhs->pt() > rhs->pt(); } 
    );
}

bool LeptonParticleLevelCollection::hasZTollCandidate(
    double halfWindow, bool allowSameSign, bool force){
    // check if this collection has a leptonic Z boson candidate
    initializeZBosonCandidate(allowSameSign, force);
    if( !_ZValid ) return false;
    if( std::abs( _bestZBosonCandidateMass - particle::mZ ) < halfWindow ) return true;
    return false;
}

void LeptonParticleLevelCollection::initializeZBosonCandidate(
    bool allowSameSign, bool force){
    // reconstruct a potential leptonic Z boson in this collection

    // if a Z boson was already initialized, skip further processing
    // (unless explicitly overridden using the force argument)
    if( _ZInitialized && !force ) return;
    // sort leptons to have consistent indices in later steps
    // (e.g. for determining leading lepton that does not come from Z-decay)
    sortByPt();
    // initializations
    double minDiff = std::numeric_limits< double >::max();
    double bestMass = 0;
    std::pair< size_type, size_type > indicesZCandidate = {99, 99};
    // loop over first lepton
    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        LeptonParticleLevel& l1 = **l1It;
        // loop over second lepton
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            LeptonParticleLevel& l2 = **l2It;
            // skip lepton pairs with different flavor
            if( !LeptonParticleLevel::sameFlavor( l1, l2 ) ) continue;
            // skip lepton pairs with same sign (if requested)
            if( LeptonParticleLevel::sameSign( l1, l2 ) && !allowSameSign ) continue;
            // determine the mass and update best combination
            double mass = ( l1 + l2 ).mass();
            double massDifference = std::abs( mass - particle::mZ );
            if( massDifference < minDiff ){
                minDiff = massDifference;
                bestMass = mass;
                indicesZCandidate = { l1It - cbegin(), l2It - cbegin() };
		// update ZValid flag
		_ZValid = true;
            }
        }
    }
    // update the best indices and mass
    _bestZBosonCandidateIndices = indicesZCandidate;
    _bestZBosonCandidateMass = bestMass;
    // update the ZIsInitialized flag
    _ZInitialized = true;
}
