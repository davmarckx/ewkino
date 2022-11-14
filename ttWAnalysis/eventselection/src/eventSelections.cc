// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection, 
		const std::string& selectiontype, const std::string& variation,
		const bool selectbjets ){
    // arguments:
    // - object of type Event
    // - event selection identifier, see map below for allowd values
    // - selection type identifier, i.e. tight, prompt, fakerate or 2tight
    // - variation identifier, i.e. all, nominal, or any JEC/JER/Uncl variation
    // - boolean whether to select b-jets (set to false for b-tag shape normalization)

    // check if selectiontype is valid
    std::vector< std::string > seltypes{ "tight","prompt","fakerate","2tight"};
    if( std::find(seltypes.cbegin(), seltypes.cend(), selectiontype)==seltypes.cend() ){
	throw std::invalid_argument("unknown selection type: "+selectiontype);
    }
    // map event selection to function
    static std::map< std::string, std::function< 
	bool(Event&, const std::string&, const std::string&, const bool) > > 
	    ESFunctionMap = {
		// signal regions
		{ "signalregion_dilepton_inclusive", pass_signalregion_dilepton_inclusive},
		{ "signalregion_dilepton_ee", pass_signalregion_dilepton_ee},
		{ "signalregion_dilepton_em", pass_signalregion_dilepton_em},
		{ "signalregion_dilepton_me", pass_signalregion_dilepton_me},
		{ "signalregion_dilepton_mm", pass_signalregion_dilepton_mm},
		{ "signalregion_trilepton", pass_signalregion_trilepton },
		// prompt control regions
		{ "wzcontrolregion", pass_wzcontrolregion },
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
		{ "cfcontrolregion", pass_cfcontrolregion }
	    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    }
    return (it->second)(event, selectiontype, variation, selectbjets);
}

// help functions for event cleaning //

constexpr double halfwindow = 10;
constexpr double halfwindow_wide = 15;

void cleanLeptonsAndJets(Event& event){
    // select leptons
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
    // select jets
    event.cleanJetsFromFOLeptons();
    event.jetCollection().selectGoodAnyVariationJets();
    // sort leptons and apply cone correction
    event.sortLeptonsByPt();
    event.applyLeptonConeCorrection();
}

// help functions for determining if event belongs to a sub-category //

bool hasLeptonFromMEExternalConversion( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && leptonFromMEExternalConversion( *leptonPtr ) ){
            return true;
        }
    }
    return false;
}

// help functions for overlap removal between inclusive and dedicated photon samples //

bool leptonFromMEExternalConversion( const Lepton& lepton ){
    // from Willem's ewkinoAnalysis code
    if( !( lepton.matchPdgId() == 22 ) ) return false;
    if( !( lepton.isPrompt() && lepton.provenanceConversion() == 0 ) ) return false;
    return true;
}

bool passPhotonOverlapRemoval( const Event& event ){
    bool isPhotonSample = false;
    bool isInclusiveSample = false;
    std::string sampleName = event.sample().fileName();
    if( stringTools::stringContains( sampleName, "DYJetsToLL" ) 
	|| stringTools::stringContains( sampleName, "TTTo" )
	|| stringTools::stringContains( sampleName, "TTJets" )
	|| (stringTools::stringContains( sampleName, "WJets" )
	    && !stringTools::stringContains( sampleName, "TTWJets")) ){
        isInclusiveSample = true;
    } else if( stringTools::stringContains( sampleName, "ZGToLLG" ) 
        || stringTools::stringContains( sampleName, "ZGTo2LG" )
	|| stringTools::stringContains( sampleName, "TTGamma" )
	|| stringTools::stringContains( sampleName, "TTGJets" ) 
	|| stringTools::stringContains( sampleName, "WGToLNuG" ) ){
        isPhotonSample = true;
    }

    if( !( isPhotonSample || isInclusiveSample ) ){
        return true;
    }

    bool usePhotonSample = false;
    // method 1: check for prompt leptons matched to photons without provenanceConversion
    //if( hasLeptonFromMEExternalConversion( event ) ) usePhotonSample = true;
    // method 2: simply check if all leptons are prompt (note: need to select FO leptons first!)
    if( allLeptonsArePrompt(event) ){
	// if all leptons are prompt -> use ZG sample
	usePhotonSample = true;
    }

    if( isInclusiveSample ){
        return !usePhotonSample;
    } else if( isPhotonSample ){
        return usePhotonSample;
    }
    return true;
}

// help functions for trigger and pt-threshold selections //

bool passAnyTrigger(Event& event){
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_ee()
                        || event.passTriggers_eee() || event.passTriggers_m()
                        || event.passTriggers_mm() || event.passTriggers_mmm()
                        || event.passTriggers_em() || event.passTriggers_eem()
                        || event.passTriggers_emm();
    return passanytrigger;
}

bool passTriLeptonPtThresholds(Event& event){
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
	|| event.leptonCollection()[1].pt() < 15.
        || event.leptonCollection()[2].pt() < 15.) return false;
    return true; 
}

bool passDiLeptonPtThresholds(Event& event){
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 20.
        || (event.leptonCollection()[0].isElectron() 
	    && event.leptonCollection()[0].pt() < 30.)) return false;
    return true;
}

// help functions for determining the number of leptons with correct ID //

bool hasnFOLeptons(Event& event, int n, bool select){
    int nFO = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() ){ ++nFO; }
    }
    if( n!=nFO ){ return false; }
    if( select ){ event.selectFOLeptons(); }
    return true;
}

bool hasnTightLeptons(Event& event, int n, bool select){
    int nTight = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isTight() ){ ++nTight; }
    }
    if( n!=nTight ){ return false; }
    if( select ){ event.selectTightLeptons(); }
    return true;
}

bool allLeptonsArePrompt( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( !leptonPtr->isPrompt() ) return false;
    }
    return true;
}

bool allLeptonsAreCorrectCharge( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->matchPdgId()==22 ) continue;
	if( leptonPtr->isChargeFlip() ) return false;
    }
    return true;
}

// help functions for determining number of jets and b-jets //

std::pair<int,int> nJetsNBJets(Event& event, const std::string& variation){
    // determine the number of jets and medium b-jets
    // return values:
    // - std::pair of number of jets, number of b-tagged jets

    int njets = 0;
    int nbjets = 0;

    // if variation = "all", only apply most general selection: 
    if( variation=="all" ){
	njets = event.jetCollection().numberOfGoodAnyVariationJets();
        nbjets = event.jetCollection().maxNumberOfMediumBTaggedJetsAnyVariation();
    }
    // else, determine number of jets and b-jets in correct variation
    else{
	JetCollection jetc = event.getJetCollection(variation);
	njets = jetc.size();
	nbjets = jetc.numberOfMediumBTaggedJets();
    } 
    return std::make_pair(njets,nbjets);
}

std::pair<int,int> nJetsNLooseBJets(Event& event, const std::string& variation){
    // determine the number of jets and loose b-jets
    // return values:
    // - std::pair of number of jets, number of b-tagged jets
    
    int njets = 0;
    int nbjets = 0;
    
    // if variation = "all", only apply most general selection: 
    if( variation=="all" ){
        njets = event.jetCollection().numberOfGoodAnyVariationJets();
        nbjets = event.jetCollection().maxNumberOfLooseBTaggedJetsAnyVariation();
    }
    // else, determine number of jets and b-jets in correct variation
    else{
        JetCollection jetc = event.getJetCollection(variation);
        njets = jetc.size();
        nbjets = jetc.numberOfLooseBTaggedJets();
    }
    return std::make_pair(njets,nbjets);
}

// help function for lepton pair mass constraint //

bool passMllMassVeto( const Event& event ){
    for( LeptonCollection::const_iterator l1It = event.leptonCollection().cbegin(); 
	l1It != event.leptonCollection().cend(); l1It++ ){
	for( LeptonCollection::const_iterator l2It = l1It+1; 
	    l2It != event.leptonCollection().cend(); l2It++ ){
            Lepton& lep1 = **l1It;
            Lepton& lep2 = **l2It;
            if((lep1+lep2).mass() < 12.) return false;
	}
    }
    return true;
}


// dedicated functions to check if event passes certain conditions //

// ---------------
// signal regions 
// ---------------

bool pass_signalregion_dilepton_inclusive(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign leptons,
    // inclusive in lepton flavours
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,2,true)) return false;
    if(not passDiLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 2, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 2, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 2, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // leptons must be same sign
    if( !event.leptonsAreSameSign() ) return false;
    // Z veto for electrons
    if( event.leptonCollection()[0].isElectron() 
	&& event.leptonCollection()[1].isElectron()
	&& event.hasZTollCandidate(halfwindow_wide, true) ) return false;
    // invariant mass safety
    if( event.leptonSystem().mass()<30. ) return false;
    // MET
    if( variation=="all" ){ if(event.met().maxPtAnyVariation()<30) return false; } 
    else{ if(event.getMet(variation).pt()<30.) return false; }
    // number of jets and b-jets
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnbjets.second < 1 && njetsnloosebjets.second < 2 ) return false;
    if( njetsnbjets.first < 2 ) return false;
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

bool pass_signalregion_dilepton_ee(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign electrons
    if( !pass_signalregion_dilepton_inclusive(event, 
	    selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isElectron() ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_em(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign electron and muon
    if( !pass_signalregion_dilepton_inclusive(event,   
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isMuon() ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_me(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign muon and electron
    if( !pass_signalregion_dilepton_inclusive(event,   
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isElectron() ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_mm(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign muons
    if( !pass_signalregion_dilepton_inclusive(event,   
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isMuon() ){ return true; }
    return false;
}

bool pass_signalregion_trilepton(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with three leptons and Z veto
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passTriLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // Z candidate veto
    if( event.hasOSSFLightLeptonPair() && event.hasZTollCandidate(halfwindow) ) return false;
    // invariant mass safety
    if(not passMllMassVeto(event)) return false;
    // sum of charges needs to be 1 or -1
    if( !event.hasOSLeptonPair() ) return false;
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnbjets.second < 1 ) return false;
    if( njetsnbjets.first < 2 ) return false;
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true; 
}

// -----------------------
// prompt control regions 
//------------------------

bool pass_wzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    // control region focusing on WZ,
    // i.e. the presence of a Z -> dilepton decay + additional lepton and b-veto.
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passTriLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    // require OSSF pair making a Z mass
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // b-jet veto and minimum MET threshold
    if( variation=="all" ){
	if(selectbjets 
	    && event.jetCollection().minNumberOfMediumBTaggedJetsAnyVariation()>0) return false;
	if(event.met().maxPtAnyVariation()<50) return false;
    } else{
	if(selectbjets 
	    && event.getJetCollection(variation).numberOfMediumBTaggedJets()>0) return false;
	if(event.getMet(variation).pt()<50.) return false;
    }
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    // control region focusing on ZZ,
    // i.e. four leptons making two Z bosons.
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,4,true)) return false;
    if(not passTriLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 4, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 4, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 4, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 3, false)) return false;
        if(hasnTightLeptons(event, 4, false)) return false;
    } else return false;
    if(!event.hasOSSFLeptonPair()) return false;
    if(!(event.numberOfUniqueOSSFLeptonPairs()==2)) return false;
    // first Z candidate
    std::pair< std::pair< int, int >, double > temp;
    temp = event.bestZBosonCandidateIndicesAndMass();
    if(fabs(temp.second-particle::mZ)>halfwindow) return false;
    // second Z candidate
    PhysicsObject lvec;
    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
        lIt != event.leptonCollection().cend(); lIt++){
        Lepton& lep = **lIt;
        if(lIt-event.leptonCollection().cbegin()==temp.first.first
                or lIt-event.leptonCollection().cbegin()==temp.first.second) continue;
        lvec += lep;
    }
    double llmass = lvec.mass();
    if(fabs(llmass-particle::mZ)>halfwindow) return false;
    // dummy condition on variation to avoid warnings
    if(variation=="dummy") return true;
    if(selectbjets){}
    return true;
}

bool pass_zgcontrolregion(Event& event, const std::string& selectiontype,
			    const std::string& variation, const bool selectbjets){
    // control region focusing on conversions,
    // i.e. three leptons making a Z boson.
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passTriLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    // mass constraints on OSSF pair and trilepton system
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(fabs(event.leptonSystem().mass()-particle::mZ)>halfwindow) return false;
    bool pairZmass = false;
    for(LeptonCollection::const_iterator lIt1 = event.leptonCollection().cbegin();
        lIt1 != event.leptonCollection().cend(); lIt1++){
        Lepton& lep1 = **lIt1;
	for(LeptonCollection::const_iterator lIt2 = lIt1+1; 
	    lIt2!=event.leptonCollection().cend(); lIt2++){
	    Lepton& lep2 = **lIt2;
	    if(fabs((lep1+lep2).mass()-particle::mZ)<halfwindow) pairZmass = true;
	    if(oppositeSignSameFlavor(lep1,lep2) && (lep1+lep2).mass() < 35.) return false;
	}
    }
    if(pairZmass) return false;
    // dummy condition on variation to avoid warnings
    if(variation=="dummy") return true;
    if(selectbjets){}
    return true;
}

bool pass_trileptoncontrolregion(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // same as trilepton signal region but with inverted Z veto and no jet requirements
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passTriLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;       
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // inverted Z candidate veto
    if( event.hasOSSFLightLeptonPair() && !event.hasZTollCandidate(halfwindow) ) return false;
    // invariant mass safety
    if(not passMllMassVeto(event)) return false;
    // sum of charges needs to 1 or -1
    if( !event.hasOSLeptonPair()) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

bool pass_fourleptoncontrolregion(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // control region with four leptons
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event, 4, true)) return false;
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 15.
        || event.leptonCollection()[2].pt() < 15.
        || event.leptonCollection()[3].pt() < 10. ) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 4, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 4, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 4, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // require presence of OSSF pair
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

// --------------------------
// nonprompt control regions 
// --------------------------

bool pass_npcontrolregion_dilepton_inclusive(
	Event& event, 
	const std::string& selectiontype,
        const std::string& variation, 
	const bool selectbjets){
    // control region for nonprompts with two same sign leptons,
    // inclusive in lepton flavours
    // (same as dilepton signal region, but with inverted MET cut)
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,2,true)) return false;
    if(not passDiLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 2, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 2, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 2, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // leptons must be same sign
    if( !event.leptonsAreSameSign()) return false;
    // Z veto for electrons
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isElectron()
        && event.hasZTollCandidate(halfwindow_wide, true) ) return false;
    // invariant mass safety
    if( event.leptonSystem().mass()<30. ) return false;
    // MET
    if( variation=="all" ){ if(event.met().maxPtAnyVariation()>30.) return false; }
    else{ if(event.getMet(variation).pt()>30.) return false; }
    // number of jets and b-jets
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnbjets.second < 1 && njetsnloosebjets.second < 2 ) return false;
    if( njetsnbjets.first < 2 ) return false;
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

bool pass_npcontrolregion_dilepton_ee(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign electrons
    if( !pass_npcontrolregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isElectron() ){ return true; }
    return false;
}

bool pass_npcontrolregion_dilepton_em(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign electron and muon
    if( !pass_npcontrolregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isMuon() ){ return true; }
    return false;
}

bool pass_npcontrolregion_dilepton_me(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign muon and electron
    if( !pass_npcontrolregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isElectron() ){ return true; }
    return false;
}

bool pass_npcontrolregion_dilepton_mm(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign muons
    if( !pass_npcontrolregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isMuon() ){ return true; }
    return false;
}

// ---------------------------------
// control regions for charge flips 
// ---------------------------------

bool pass_cfcontrolregion(Event& event,
            const std::string& selectiontype,
            const std::string& variation,
            const bool selectbjets){
    // control region for nonprompts with two same sign electrons on the Z peak
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event, 2, true)) return false;
    if(not passDiLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="tight"){
        if(!hasnTightLeptons(event, 2, true)) return false;
    } else if(selectiontype=="prompt"){
        if(!hasnTightLeptons(event, 2, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 2, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else return false;
    // select di-electron events
    if(event.leptonCollection()[0].isMuon() || event.leptonCollection()[1].isMuon()) return false;
    // leptons must be same sign
    if(!event.leptonsAreSameSign()) return false;
    // leptons must make a Z candidate
    if(!event.hasZTollCandidate(halfwindow, true)) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}
