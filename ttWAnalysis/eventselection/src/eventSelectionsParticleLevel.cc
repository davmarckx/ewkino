// include header 
#include "../interface/eventSelectionsParticleLevel.h"

//include c++ library classes
#include <functional>

bool eventSelectionsParticleLevel::passES(Event& event, const std::string& eventselection ){
    // arguments:
    // - object of type Event
    // - event selection identifier, see map below for allowd values

    // map event selection to function
    static std::map< std::string, std::function< bool(Event&) > > 
	    ESFunctionMap = {
		// signal regions
		{ "signalregion_dilepton_inclusive", pass_signalregion_dilepton_inclusive},
		{ "signalregion_dilepton_ee", pass_signalregion_dilepton_ee},
		{ "signalregion_dilepton_em", pass_signalregion_dilepton_em},
		{ "signalregion_dilepton_me", pass_signalregion_dilepton_me},
		{ "signalregion_dilepton_mm", pass_signalregion_dilepton_mm},
		{ "signalregion_trilepton", pass_signalregion_trilepton },
		// prompt control regions
		/*{ "wzcontrolregion", pass_wzcontrolregion },
		{ "zzcontrolregion", pass_zzcontrolregion },
		{ "zgcontrolregion", pass_zgcontrolregion },
		{ "fourleptoncontrolregion", pass_fourleptoncontrolregion },
		{ "trileptoncontrolregion", pass_trileptoncontrolregion },
		// nonprompt control regions
		{ "npcontrolregion_dilepton_inclusive", pass_npcontrolregion_dilepton_inclusive },
		{ "npcontrolregion_dilepton_ee", pass_npcontrolregion_dilepton_ee },
		{ "npcontrolregion_dilepton_em", pass_npcontrolregion_dilepton_em },
		{ "npcontrolregion_dilepton_me", pass_npcontrolregion_dilepton_me },
		{ "npcontrolregion_dilepton_mm", pass_npcontrolregion_dilepton_mm },
		// charge flip control region
		{ "cfcontrolregion", pass_cfcontrolregion }*/
	    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
	std::string msg = "ERROR in eventSelectionParticleLevel.cc / passES:";
	msg += " unknown event selection " + eventselection;
        throw std::invalid_argument( msg );
    }
    return (it->second)(event);
}

// help functions for event cleaning //

constexpr double halfwindow = 10;
constexpr double halfwindow_wide = 15;

void eventSelectionsParticleLevel::cleanLeptonsAndJets(Event& event){
    // do lepton cleaning
    event.leptonParticleLevelCollection().removeTaus();
    // do jet cleaning
    // (to implement if needed)
    // sort leptons and jets by pt
    event.leptonParticleLevelCollection().sortByPt();
    event.jetParticleLevelCollection().sortByPt();
}


// help functions for trigger and pt-threshold selections //

bool eventSelectionsParticleLevel::passTriLeptonPtThresholds(const Event& event){
    event.leptonParticleLevelCollection().sortByPt();
    if(event.leptonParticleLevelCollection()[0].pt() < 25.
	|| event.leptonParticleLevelCollection()[1].pt() < 15.
        || event.leptonParticleLevelCollection()[2].pt() < 15.) return false;
    return true; 
}

bool eventSelectionsParticleLevel::passDiLeptonPtThresholds(const Event& event){
    event.leptonParticleLevelCollection().sortByPt();
    if(event.leptonParticleLevelCollection()[0].pt() < 25.
        || event.leptonParticleLevelCollection()[1].pt() < 20.
        || (event.leptonParticleLevelCollection()[0].isElectron() 
	    && event.leptonParticleLevelCollection()[0].pt() < 30.)) return false;
    return true;
}


// help functions for determining number of jets and b-jets //

std::pair<int,int> eventSelectionsParticleLevel::nJetsNBJets(const Event& event){
    // determine the number of jets and b-jets
    int njets = event.jetParticleLevelCollection().numberOfJets();
    int nbjets = event.jetParticleLevelCollection().numberOfBJets();
    return std::make_pair(njets,nbjets);
}

// help function for lepton pair mass constraint //

bool eventSelectionsParticleLevel::passMllMassVeto( const Event& event ){
    for( LeptonParticleLevelCollection::const_iterator l1It = event.leptonParticleLevelCollection().cbegin(); 
	l1It != event.leptonParticleLevelCollection().cend(); l1It++ ){
	for( LeptonParticleLevelCollection::const_iterator l2It = l1It+1; 
	    l2It != event.leptonParticleLevelCollection().cend(); l2It++ ){
            LeptonParticleLevel& lep1 = **l1It;
            LeptonParticleLevel& lep2 = **l2It;
            if( LeptonParticleLevel::sameFlavor(lep1,lep2) && (lep1+lep2).mass() < 12. ) return false;
	}
    }
    return true;
}


// dedicated functions to check if event passes certain conditions //

// ---------------
// signal regions 
// ---------------

bool eventSelectionsParticleLevel::pass_signalregion_dilepton_inclusive(Event& event){
    // signal region with two same sign leptons,
    // inclusive in lepton flavours
    cleanLeptonsAndJets(event);
    LeptonParticleLevelCollection lepcollection = event.leptonParticleLevelCollection();
    // basic requirements
    if( lepcollection.numberOfLeptons()!=2 ) return false;
    if( !passDiLeptonPtThresholds(event) ) return false;
    // leptons must be same sign
    if( !LeptonParticleLevel::sameSign(lepcollection[0],lepcollection[1]) ) return false;
    // Z veto for electrons
    if( lepcollection[0].isElectron() 
	&& lepcollection[1].isElectron()
	&& lepcollection.hasZTollCandidate(halfwindow_wide, true) ) return false;
    // invariant mass safety
    if( lepcollection.mass()<30. ) return false;
    // MET
    if( event.metParticleLevel().pt()<30.) return false;
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event);
    //if( njetsnbjets.second < 2 ) return false;
    if( njetsnbjets.second < 1 ) return false;
    if( njetsnbjets.first < 2 ) return false;
    return true;
}

std::tuple<int,std::string> eventSelectionsParticleLevel::pass_signalregion_dilepton_inclusive_cutflow(
    Event& event){
    // copy of pass_signalregion_dilepton_inclusive but with different return type
    // to allow cutflow studies
    cleanLeptonsAndJets(event);
    LeptonParticleLevelCollection lepcollection = event.leptonParticleLevelCollection();
    // basic requirements
    if( lepcollection.numberOfLeptons()!=2 ) return std::make_tuple(0, "Fail 2 leptons");
    if( !passDiLeptonPtThresholds(event) ) return std::make_tuple(1, "Fail pT thresholds");
    // leptons must be same sign
    if( !LeptonParticleLevel::sameSign(lepcollection[0],lepcollection[1]) ){ 
	return std::make_tuple(2, "Fail same sign"); }
    // Z veto for electrons
    if( lepcollection[0].isElectron()
        && lepcollection[1].isElectron()
        && lepcollection.hasZTollCandidate(halfwindow_wide, true) ){
	return std::make_tuple(3, "Fail Z veto"); }
    // invariant mass safety
    if( lepcollection.mass()<30. ) return std::make_tuple(4, "Fail low mass veto");
    // MET
    if( event.metParticleLevel().pt()<30.) return std::make_tuple(5, "Fail MET");
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event);
    //if( njetsnbjets.second < 2 ) return std::make_tuple(6, "Fail b-jets");
    if( njetsnbjets.second < 1 ) return std::make_tuple(6, "Fail b-jets");
    if( njetsnbjets.first < 2 ) return std::make_tuple(7, "Fail jets");
    return std::make_tuple(8, "Pass");
}

bool eventSelectionsParticleLevel::pass_signalregion_dilepton_ee(Event& event){
    // signal region with two same sign electrons
    if( !pass_signalregion_dilepton_inclusive(event) ){ return false; }
    if( event.leptonParticleLevelCollection()[0].isElectron()
        && event.leptonParticleLevelCollection()[1].isElectron() ){ return true; }
    return false;
}

bool eventSelectionsParticleLevel::pass_signalregion_dilepton_em(Event& event){
    // signal region with same sign electron and muon
    if( !pass_signalregion_dilepton_inclusive(event) ){ return false; }
    if( event.leptonParticleLevelCollection()[0].isElectron()
        && event.leptonParticleLevelCollection()[1].isMuon() ){ return true; }
    return false;
}

bool eventSelectionsParticleLevel::pass_signalregion_dilepton_me(Event& event){
    // signal region with same sign muon and electron
    if( !pass_signalregion_dilepton_inclusive(event) ){ return false; }
    if( event.leptonParticleLevelCollection()[0].isMuon()
        && event.leptonParticleLevelCollection()[1].isElectron() ){ return true; }
    return false;
}

bool eventSelectionsParticleLevel::pass_signalregion_dilepton_mm(Event& event){
    // signal region with two same sign muons
    if( !pass_signalregion_dilepton_inclusive(event) ){ return false; }
    if( event.leptonParticleLevelCollection()[0].isMuon()
        && event.leptonParticleLevelCollection()[1].isMuon() ){ return true; }
    return false;
}

bool eventSelectionsParticleLevel::pass_signalregion_trilepton(Event& event){
    // signal region with three leptons and Z veto
    cleanLeptonsAndJets(event);
    LeptonParticleLevelCollection lepcollection = event.leptonParticleLevelCollection();
    // basic requirements
    if( lepcollection.numberOfLeptons()!=3 ) return false;
    if( !passTriLeptonPtThresholds(event) ) return false;
    // Z candidate veto
    if( lepcollection.hasOppositeSignSameFlavorPair() 
	&& lepcollection.hasZTollCandidate(halfwindow) ) return false;
    // invariant mass safety
    if( !passMllMassVeto(event) ) return false;
    // sum of charges needs to be 1 or -1
    if( !lepcollection.hasOppositeSignPair() ) return false;
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event);
    if( njetsnbjets.second < 1 ) return false;
    if( njetsnbjets.first < 2 ) return false;
    return true; 
}

std::tuple<int,std::string> eventSelectionsParticleLevel::pass_signalregion_trilepton_cutflow(
    Event& event){
    // copy of pass_signalregion_trilepton but with different return type
    // to allow cutflow studies
    cleanLeptonsAndJets(event);
    LeptonParticleLevelCollection lepcollection = event.leptonParticleLevelCollection();
    // basic requirements
    if( lepcollection.numberOfLeptons()!=3 ) return std::make_tuple(0, "Fail 3 leptons");
    if( !passTriLeptonPtThresholds(event) ) return std::make_tuple(1, "Fail pt thresholds");
    // Z candidate veto
    if( lepcollection.hasOppositeSignSameFlavorPair()
        && lepcollection.hasZTollCandidate(halfwindow) ) return std::make_tuple(2, "Fail Z veto");
    // invariant mass safety
    if( !passMllMassVeto(event) ) return std::make_tuple(3, "Fail low mass veto");
    // sum of charges needs to be 1 or -1
    if( !lepcollection.hasOppositeSignPair() ) return std::make_tuple(4, "Fail OS pair");
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event);
    if( njetsnbjets.second < 1 ) return std::make_tuple(5, "Fail b-jets");
    if( njetsnbjets.first < 2 ) return std::make_tuple(6, "Fail jets");
    return std::make_tuple(7, "Pass");
}
