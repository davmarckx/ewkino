/*
Test of particle level collections and objects
*/

// parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"

//include c++ library classes
#include <iostream>

int main(){

    // set test sample
    std::string testFile = "/pnfs/iihe/cms/store/user/llambrec/heavyNeutrinoTTWTest/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/crab_RunIISummer20UL18MiniAOD-106X_upgrade2018_realistic_v11_L1v1-v2_singlelepton_MC_allyears_TTW_ULv6/221207_092937/0000/singlelep_1.root";
    unsigned long maxEntries = 100;

    // make TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( testFile );

    // loop over entries
    unsigned long numberOfEntries = treeReader.numberOfEntries();
    if( maxEntries>0 && maxEntries < numberOfEntries ){
	numberOfEntries = maxEntries;
    }
    for( long unsigned i = 0; i < numberOfEntries; ++i ){
	
	// build event
        Event event = treeReader.buildEvent( i, false, false, false, false, true );
	std::cout << "--- Event ---" << std::endl;	

	// print particle level leptons
	LeptonParticleLevelCollection leptons = event.leptonParticleLevelCollection();
	std::cout << leptons.size() << std::endl;
	for( const std::shared_ptr<LeptonParticleLevel> lepton: leptons ){
	    std::cout << *lepton << std::endl;
	}
    }
}
