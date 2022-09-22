#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"


/*
loose electron selection
*/


double leptonMVACutElectron(){
    return 0.81;
}


bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 10 ) return false;
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) > 1.4442 
        && std::abs(electronPtr->etaSuperCluster()) < 1.566) return false;
    return true;
}



bool ElectronSelector::isLoose2016() const{ 
    return true;
}


bool ElectronSelector::isLoose2016PreVFP() const{
    return true;
}


bool ElectronSelector::isLoose2016PostVFP() const{
    return true;
}


bool ElectronSelector::isLoose2017() const{
    return true;
}


bool ElectronSelector::isLoose2018() const{
    return true;
}


/*
FO electron selection
*/

bool ElectronSelector::isFOBase() const{
    if( !isLoose() ) return false;
    if( !electronPtr->passConversionVeto() ) return false;
    if( !electronPtr->passChargeConsistency() ) return false;
    return true;
}


bool ElectronSelector::isFO2016() const{
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2016PreVFP() const{
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2016PostVFP() const{
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2017() const{
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
        if( electronPtr->ptRatio() < 0.4 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2018() const{
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
        if( electronPtr->ptRatio() < 0.4 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


/*
tight electron selection
*/

bool ElectronSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( electronPtr->leptonMVATOPUL() <= leptonMVACutElectron() ) return false;
    return true;
}


bool ElectronSelector::isTight2016() const{
    return true;
}


bool ElectronSelector::isTight2016PreVFP() const{
    return true;
}


bool ElectronSelector::isTight2016PostVFP() const{
    return true;
}


bool ElectronSelector::isTight2017() const{
    return true;
}


bool ElectronSelector::isTight2018() const{
    return true;
}


/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.72 / electronPtr->ptRatio() );
}
