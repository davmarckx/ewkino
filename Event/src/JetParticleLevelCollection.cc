#include "../interface/JetParticleLevelCollection.h"


JetParticleLevelCollection::JetParticleLevelCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._pl_nJets; ++j ){
        push_back( JetParticleLevel( treeReader, j ) ); 
    }
}

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

JetParticleLevelCollection JetParticleLevelCollection::PLbJetCollection() const{
    return buildSubCollection( &JetParticleLevel::isBJet );
}
                            
JetParticleLevelCollection::size_type JetParticleLevelCollection::numberOfJets() const{
    // return number of jets in this collection (i.e. size)
    return size();
}

JetParticleLevelCollection::size_type JetParticleLevelCollection::numberOfBJets() const{
    // return number of b-jets in this collection
    // uses the inherited count method (from PhysicsObjectCollection)
    return count( &JetParticleLevel::isBJet );
}

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
