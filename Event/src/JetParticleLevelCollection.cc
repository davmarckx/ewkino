#include "../interface/JetParticleLevelCollection.h"


JetParticleLevelCollection::JetParticleLevelCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._pl_nJets; ++j ){
        push_back( JetParticleLevel( treeReader, j ) ); 
    }
}
