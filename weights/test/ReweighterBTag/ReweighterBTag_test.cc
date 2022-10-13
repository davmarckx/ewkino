// note: kept for reference but not updated since a long time,
//       since the b-tag shape reweighter instead of fixed WP reweighter must be used.
//       will need (at least) to fix relative imports and change the file paths for this to work again.

#include "../../weights/interface/ReweighterBTag.h"
#include "../../weights/interface/ConcreteReweighterBTag.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../copyMoveTest.h"


int main(){

    //read b-tag efficiencies 
    TFile* bTagEffFile = TFile::Open( "testSF/bTagEff_deepCSV_cleaned_tZq_2016.root" );
    std::shared_ptr< TH2 > bTagEff_udsg( dynamic_cast< TH2* >( bTagEffFile->Get( "bTagEff_tightudsg" ) ) );
    bTagEff_udsg->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEff_c( dynamic_cast< TH2* >( bTagEffFile->Get( "bTagEff_tightcharm" ) ) );
    bTagEff_c->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEff_b( dynamic_cast< TH2* >( bTagEffFile->Get( "bTagEff_tightbeauty" ) ) );
    bTagEff_b->SetDirectory( gROOT );

    //make b-tag reweighter
    ReweighterBTagDeepCSV reweighterBTag( "../../weights/files/DeepCSV_2016LegacySF_WP_V1.csv", "tight", bTagEff_udsg, bTagEff_c, bTagEff_b );
    copyMoveTest( reweighterBTag );

    //loop over samples
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );
            reweighterBTag.weight( event );
            reweighterBTag.weightDown( event );
            reweighterBTag.weightUp( event );
        }
    }

    return 0;
}
