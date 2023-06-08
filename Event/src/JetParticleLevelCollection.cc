#include "../interface/JetParticleLevelCollection.h"

// constructor
JetParticleLevelCollection::JetParticleLevelCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._pl_nJets; ++j ){
        push_back( JetParticleLevel( treeReader, j ) ); 
    }
}

// selection
void JetParticleLevelCollection::selectGoodJets(){
    selectObjects( &JetParticleLevel::isGood );
}

// cleaning
void JetParticleLevelCollection::cleanJetsFromLeptons( 
    const LeptonParticleLevelCollection& leptonCollection,
    const double coneSize ){
    // loop over jets
    for( const_iterator jetIt = cbegin(); jetIt != cend(); ){
        JetParticleLevel& jet = **jetIt;
        bool isDeleted = false;
	// loop over leptons
        for( LeptonParticleLevelCollection::const_iterator lIt = leptonCollection.cbegin();
	     lIt != leptonCollection.cend(); ++lIt ){
            LeptonParticleLevel& lepton = **lIt;
            // remove jet if it overlaps with a lepton
            if( deltaR( jet, lepton ) < coneSize ){
                jetIt = erase( jetIt );
                isDeleted = true;
                break;
            }
        }
        if( !isDeleted ){ ++jetIt; }
    }
}

// build sub-collections
JetParticleLevelCollection JetParticleLevelCollection::buildSubCollection( bool (JetParticleLevel::*passSelection)() const ) const{
    std::vector< std::shared_ptr< JetParticleLevel > > jetVector;
    for( const auto& jetPtr : *this ){
        if( ( *jetPtr.*passSelection )() ){

            //jets are shared between collections!
            jetVector.push_back( jetPtr );
            }
        }
        return JetParticleLevelCollection( jetVector );
}

// build b-jet collection
JetParticleLevelCollection JetParticleLevelCollection::PLbJetCollection() const{
    return buildSubCollection( &JetParticleLevel::isBJet );
}

// number of jets                           
JetParticleLevelCollection::size_type JetParticleLevelCollection::numberOfJets() const{
    // return number of jets in this collection (i.e. size)
    return size();
}

// number of b-jets
JetParticleLevelCollection::size_type JetParticleLevelCollection::numberOfBJets() const{
    // return number of b-jets in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &JetParticleLevel::isBJet );
}

// sorting
void JetParticleLevelCollection::sortByPt(){
    // sort jets in this collection by pt
    // uses the inherited sortByAttribute method
    // (from PhysicsObjectCollection)
    return sortByAttribute(
        [](const std::shared_ptr< JetParticleLevel >& lhs,
           const std::shared_ptr< JetParticleLevel >& rhs){
        return lhs->pt() > rhs->pt(); }
    );
}
