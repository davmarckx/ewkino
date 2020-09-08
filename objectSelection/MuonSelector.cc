#include "../objects/interface/MuonSelector.h"

//b-tagging working points
#include "bTagWP.h"

// define here what mva threshold to use in tZq ID's listed below
double muonMVACut(){
    if(LeptonSelector::leptonID()=="tth") return 0.85;
    else if(LeptonSelector::leptonID()=="oldtzq") return 0.8;
    else if(LeptonSelector::leptonID()=="tzqtight") return 0.9;
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return 0.4; 
    else return 1.;
}

// define here what mva value to use in tZq ID's listed below
double muonMVAValue(const Muon* muonPtr){
    if(LeptonSelector::leptonID()=="tth") return muonPtr->leptonMVAttH();
    else if(LeptonSelector::leptonID()=="oldtzq") return muonPtr->leptonMVAtZq();
    else if(LeptonSelector::leptonID()=="tzqtight") return muonPtr->leptonMVATOP();
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return muonPtr->leptonMVATOP();
    else return 0;
}

// define here what b-tagger threshold to use in all tZq ID's listed below
// warning: deepFlavor thresholds are hard-coded in ttH ID!
double muonJetBTagCut(const std::string wp, const std::string year){
    //return bTagWP::getWP("DeepCSV", wp, year);
    return bTagWP::getWP("DeepFlavor", wp, year);    
}

// define here what b-tagger to use in all tZq ID's listed below
// warning: deepFlavor is hard-coded in ttH ID!
double muonJetBTagValue(const Muon* muonPtr){
    //return muonPtr->closestJetDeepCSV();
    return muonPtr->closestJetDeepFlavor();
}

double muonConeCorrectionFactor(){
    if(LeptonSelector::leptonID()=="tth") return 0.75;
    else if(LeptonSelector::leptonID()=="oldtzq") return 0.95;
    else if(LeptonSelector::leptonID()=="tzqtight") return 0.8;
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return 0.67;
    else return 0;
}

/*
-------------------------------------------------------------------
loose muon selection (common to tZq, ttH and old tZq ID)
-------------------------------------------------------------------
*/

bool MuonSelector::isLooseBase() const{
    if( muonPtr->uncorrectedPt() <= 5 ) return false;
    if( muonPtr->absEta() >= 2.4 ) return false; 
    if( fabs( muonPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( muonPtr->dz() ) >= 0.1 ) return false;
    if( muonPtr->sip3d() >= 8 ) return false;
    if( muonPtr->miniIso() >= 0.4 ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isLoose2016() const{
    return true;
}


bool MuonSelector::isLoose2017() const{
    return true;
}


bool MuonSelector::isLoose2018() const{
    return true;
}

/*
--------------------------------------------------------------------------------
help function for FO ID's
-------------------------------------------------------------------------------
*/

//interpolation between two working points of deepFlavor between two pT values
double muonSlidingDeepFlavorThreshold( const double lowPt, const double lowPtWP, 
				    const double highPt, const double highPtWP,
				    const double pt ){
    if( pt < lowPt ){
        return lowPtWP;
    } else if( pt > highPt ){
        return highPtWP;
    } else {
        return ( lowPtWP + ( highPtWP - lowPtWP ) / ( highPt - lowPt ) * ( pt - lowPt ) );
    }
}

/*
---------------------------------------------------------------------------------
FO muon selection for (tight) tZq ID
---------------------------------------------------------------------------------
*/

bool MuonSelector::isFOBasetZq() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    // put tunable FO-cuts below
    //if( muonMVAValue(muonPtr) <= muonMVACut() ){
    //}
    return true;
}


bool MuonSelector::isFO2016tZq() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
	if( muonPtr->closestJetDeepFlavor() > 0.015 ) return false;
	if( muonPtr->ptRatio() < 0.5 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017tZq() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
	if( muonPtr->closestJetDeepFlavor() > 0.02 ) return false;
	if( muonPtr->ptRatio() < 0.6 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018tZq() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
	if( muonPtr->closestJetDeepFlavor() > 0.03 ) return false;
	if( muonPtr->ptRatio() < 0.4 ) return false;
    }
    return true;
}

/*
---------------------------------------------------------------------------------
FO muon selection for medium 0.4 tZq ID
---------------------------------------------------------------------------------
*/

bool MuonSelector::isFOBasetZqMedium0p4() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    // put tunable FO-cuts below
    //if( muonMVAValue(muonPtr) <= muonMVACut() ){
    //}
    return true;
}


bool MuonSelector::isFO2016tZqMedium0p4() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 25., 0.03, 40., 0.015, 
		muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() < 0.35 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017tZqMedium0p4() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 25., 0.05, 40., 0.02, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() < 0.35 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018tZqMedium0p4() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 25., 0.04, 40., 0.02, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() < 0.35 ) return false;
    }
    return true;
}


/*
---------------------------------------------------------------------------------
FO muon selection for ttH ID
---------------------------------------------------------------------------------
*/

bool MuonSelector::isFOBasettH() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        if( muonPtr->ptRatio() <= 0.65 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016ttH() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        double deepFlavorCut = muonSlidingDeepFlavorThreshold( 20, bTagWP::mediumDeepFlavor2016(), 
				45, bTagWP::looseDeepFlavor2016(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
        if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017ttH() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        double deepFlavorCut = muonSlidingDeepFlavorThreshold( 20, bTagWP::looseDeepFlavor2017(), 
				45, bTagWP::mediumDeepFlavor2017(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
       if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018ttH() const{
    if( muonMVAValue(muonPtr) <= muonMVACut() ){
        double deepFlavorCut = muonSlidingDeepFlavorThreshold( 20, bTagWP::looseDeepFlavor2018(), 
				45, bTagWP::mediumDeepFlavor2018(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
        if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    }
    return true;
}

/*
------------------------------------------------------------------------------
FO muon selection for old tZq ID
------------------------------------------------------------------------------
*/ 

bool MuonSelector::isFOBaseOldtZq() const{
    // function to copy as closely as possible lepton ID of tZq analysis note
    if(!isLoose()) return false;
    if(muonPtr->uncorrectedPt()<10) return false;
    if(!muonPtr->isMediumPOGMuon()) return false;
    if(muonMVAValue(muonPtr) < muonMVACut()){
	if(muonPtr->ptRatio()<0.6) return false;
    }
    return true;
}

bool MuonSelector::isFO2016OldtZq() const{
    if(muonMVAValue(muonPtr) < muonMVACut()){
        if(muonJetBTagValue(muonPtr) > 0.3) return false;
    }
    else{
        if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2016")) return false;
    }
    return true;    
}

bool MuonSelector::isFO2017OldtZq() const{
    if(muonMVAValue(muonPtr) < muonMVACut()){
        if(muonJetBTagValue(muonPtr) > 0.2) return false;
    }
    else{
        if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2017")) return false;
    }
    return true;
}

bool MuonSelector::isFO2018OldtZq() const{
    if(muonMVAValue(muonPtr) < muonMVACut()){
	if(muonJetBTagValue(muonPtr) > 0.2) return false;
    }
    else{
        if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2018")) return false;
    }
    return true;
}

/*
----------------------------------------------------------------------------
tight muon selection for (tight) tZq ID
----------------------------------------------------------------------------
*/

bool MuonSelector::isTightBasetZq() const{
    if( !isFOtZq() ) return false;
    if( muonMVAValue(muonPtr) <= muonMVACut() ) return false;
    return true;
}


bool MuonSelector::isTight2016tZq() const{
    return true;
}


bool MuonSelector::isTight2017tZq() const{
    return true;
}


bool MuonSelector::isTight2018tZq() const{
    return true;
}

/*
----------------------------------------------------------------------------
tight muon selection for medium 0p4 tZq ID
----------------------------------------------------------------------------
*/

bool MuonSelector::isTightBasetZqMedium0p4() const{
    if( !isFOtZqMedium0p4() ) return false;
    if( muonMVAValue(muonPtr) <= muonMVACut() ) return false;
    return true;
}


bool MuonSelector::isTight2016tZqMedium0p4() const{
    return true;
}


bool MuonSelector::isTight2017tZqMedium0p4() const{
    return true;
}


bool MuonSelector::isTight2018tZqMedium0p4() const{
    return true;
}

/*
----------------------------------------------------------------------------
tight muon selection for ttH ID
----------------------------------------------------------------------------
*/

bool MuonSelector::isTightBasettH() const{
    if( !isFOttH() ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    if( muonMVAValue(muonPtr) <= muonMVACut() ) return false;
    return true;
}


bool MuonSelector::isTight2016ttH() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    return true;
}


bool MuonSelector::isTight2017ttH() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    return true;
}


bool MuonSelector::isTight2018ttH() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    return true;
}

/* 
--------------------------------------------------------------------------
tight muon selection for old tZq ID
--------------------------------------------------------------------------
*/

bool MuonSelector::isTightBaseOldtZq() const{
    // function to copy as closely as possible lepton ID of tZq analysis note
    if(!isFOOldtZq()) return false;
    if(muonMVAValue(muonPtr) < muonMVACut()) return false;
    return true;
}

bool MuonSelector::isTight2016OldtZq() const{
    if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2016")) return false;
    return true;
}

bool MuonSelector::isTight2017OldtZq() const{
    if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2017")) return false;
    return true;
}

bool MuonSelector::isTight2018OldtZq() const{
    if(muonJetBTagValue(muonPtr) > muonJetBTagCut("tight", "2018")) return false;
    return true;
}


/*
cone correction
*/

double MuonSelector::coneCorrection() const{
    return ( muonConeCorrectionFactor() / muonPtr->ptRatio() );
}
