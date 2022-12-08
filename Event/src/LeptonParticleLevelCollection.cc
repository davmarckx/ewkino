#include "../interface/LeptonParticleLevelCollection.h"


LeptonParticleLevelCollection::LeptonParticleLevelCollection( const TreeReader& treeReader ){

    // add particle level leptons to thecollection
    for( unsigned l = 0; l < treeReader._pl_nL; ++l){
        push_back( LeptonParticleLevel( treeReader, l ) );
    }
}
