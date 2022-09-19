#include "../interface/ewkinoCategorization.h"


ewkino::EwkinoCategory ewkino::ewkinoCategory( const Event& event ){

    if( event.numberOfLeptons() == 2 ){
        if( event.numberOfLightLeptons() != 2 ){
            return unknown;
        } else if( event.lepton(0).charge() == event.lepton(1).charge() ){
            return dilepSS;
        } else {
            return unknown;
        }
    } else if( event.numberOfLeptons() == 3 ){
        if( event.numberOfTaus() == 0 ){
            if( event.hasOSSFLeptonPair() ){
                return trilepLightOSSF;
            } else {
                return trilepLightNoOSSF;
            }
        } else if( event.numberOfTaus() == 1 ){
            if( event.hasOSSFLeptonPair() ){
                return trilepSingleTauOSSF;
            } else if( event.hasOSSFLightLeptonPair() ){
                return trilepSingleTauOS;
            } else{
                return trilepSingleTauSS;
            }
        } else if( event.numberOfTaus() == 2 ){
            return trilepDiTau;
        } else {
            return unknown;
        }

    //be careful to only consider the four leading leptons here
    } else if( event.numberOfLeptons() >= 4 ){
        LeptonCollection leadingLeptons = event.leptonCollection().leadingLeptonCollection( 4 );

        if( leadingLeptons.numberOfTaus() == 4 ){
            if( leadingLeptons.numberOfUniqueOSSFPairs() == 2 ){
                return fourlepLightTwoOSSF;
            } else {
                return fourlepLightOneZeroOSSF;
            } 
        } else if( leadingLeptons.numberOfTaus() == 1 ){
            return fourlepSingleTau;
        } else if( leadingLeptons.numberOfTaus() == 2 ){
            if( leadingLeptons.numberOfUniqueOSSFPairs() == 2 ){
                return fourlepDiTauTwoOSSF;
            } else {
                return fourlepDiTauOneZeroOSSF;
            }
        } else {
            return unknown;
        }
    } else {
        return unknown;
    }
}
