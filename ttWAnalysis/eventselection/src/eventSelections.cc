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
    std::vector< std::string > seltypes{ "tight", 
					 "prompt", "chargegood", "irreducible",
					 "fakerate", "efakerate", "mfakerate", "chargeflips" };
    if( std::find(seltypes.cbegin(), seltypes.cend(), selectiontype)==seltypes.cend() ){
	throw std::invalid_argument("unknown selection type: "+selectiontype);
    }
    // map event selection to function
    static std::map< std::string, std::function< 
	bool(Event&, const std::string&, const std::string&, const bool) > > 
	    ESFunctionMap = {
		// no selection
		{ "noselection", pass_noselection },    
		// signal regions
		{ "signalregion_dilepton_inclusive", pass_signalregion_dilepton_inclusive },
		{ "signalregion_dilepton_ee", pass_signalregion_dilepton_ee },
		{ "signalregion_dilepton_em", pass_signalregion_dilepton_em },
		{ "signalregion_dilepton_me", pass_signalregion_dilepton_me },
		{ "signalregion_dilepton_mm", pass_signalregion_dilepton_mm },
                { "signalregion_dilepton_pmm", pass_signalregion_dilepton_pmm },
                { "signalregion_dilepton_nmm", pass_signalregion_dilepton_nmm },
                { "signalregion_dilepton_pee", pass_signalregion_dilepton_pee },
                { "signalregion_dilepton_nee", pass_signalregion_dilepton_nee },
		{ "signalregion_dilepton_plus", pass_signalregion_dilepton_plus },
		{ "signalregion_dilepton_minus", pass_signalregion_dilepton_minus },
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

// help functions for overlap removal between inclusive and dedicated photon samples //

bool hasLeptonFromMEExternalConversion( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && leptonFromMEExternalConversion( *leptonPtr ) ){
            return true;
        }
    }
    return false;
}

bool leptonFromMEExternalConversion( const Lepton& lepton ){
    // from Willem's ewkinoAnalysis code
    // this function checks whether a leptons originates
    // from a prompt matrix-element photon that converted externally;
    // technically: 
    // - lepton must be matched to a photon (with pdg id 22) (to select conversion leptons)
    // - lepton must be prompt (to select prompt photons)
    // - provenanceConversion must be 0 (to select external rather than internal conversions?)
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
    if( hasLeptonFromMEExternalConversion( event ) ) usePhotonSample = true;
    // method 2: simply check if all leptons are prompt (note: need to select FO leptons first!)
    /*if( allLeptonsArePrompt(event) ){
	// if all leptons are prompt -> use ZG sample
	usePhotonSample = true;
    }*/
    // method 3: simply do not use specific photon samples
    //usePhotonSample = false;

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
	if( leptonPtr->isChargeFlip() ) return false;
    }
    return true;
}

bool doLeptonSelection( Event& event, std::string selectiontype, int nleptons ){
    // internal helper function common to all specific selections
    if(selectiontype=="tight"){
	// normal selection of tight leptons for data vs MC
        if(!hasnTightLeptons(event, nleptons, true)) return false;
    } else if(selectiontype=="prompt"){
	// selection of tight prompt leptons (for nonprompt from data)
        if(!hasnTightLeptons(event, nleptons, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="chargegood"){
	// selection of tight leptons with correct charge (for charge flips from data)
	if(!hasnTightLeptons(event, nleptons, true)) return false;
	if(event.isMC() and !allLeptonsAreCorrectCharge(event)) return false;
    } else if(selectiontype=="irreducible"){
	// combination of prompt and chargegood (for both nonprompt and charge flips from data)
	if(!hasnTightLeptons(event, nleptons, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
        if(event.isMC() and !allLeptonsAreCorrectCharge(event)) return false;
    } else if(selectiontype=="fakerate"){
	// selection of at least one non-tight leptons (for nonprompt from data)
        if(hasnTightLeptons(event, nleptons, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="efakerate"){
	// same as above but electron fake only (for splitting in muon and electron fakes)
	if(hasnTightLeptons(event, nleptons, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
	for( const auto& leptonPtr : event.leptonCollection() ){
	    if( !leptonPtr->isTight() ){
		if( !leptonPtr->isElectron() ) return false;
		break;
	    }
	}
    } else if(selectiontype=="mfakerate"){
	// same as above but muon fakes only (for splitting in muon and electron fakes)
	if(hasnTightLeptons(event, nleptons, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
	for( const auto& leptonPtr : event.leptonCollection() ){
            if( !leptonPtr->isTight() ){
		if( !leptonPtr->isMuon() ) return false;
		break;
	    }
        }
    } else if(selectiontype=="chargeflips"){
	// selection of OS events with tight leptons
	// (note: OS/SS selection has to be done in specific selection functions!)
	if(!hasnTightLeptons(event, nleptons, true)) return false;
        if(event.isMC()) return false;
    } else{
	std::string msg = "ERROR in eventSelections.cc / doLeptonSelection:";
	msg.append(" selection type " + selectiontype + " not recognized.");
	throw std::invalid_argument(msg);
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
            if( sameFlavor(lep1,lep2) && (lep1+lep2).mass() < 12. ) return false;
	}
    }
    return true;
}


// dedicated functions to check if event passes certain conditions //

// -------------
// no selection 
// -------------

bool pass_noselection(Event& event, const std::string& selectiontype,
			const std::string& variation, const bool selectbjets){
    cleanLeptonsAndJets(event);
    if(selectiontype=="dummy"){} // dummy to avoid unused parameter warning
    if(variation=="dummy"){} // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

// ---------------
// signal regions 
// ---------------

/*bool pass_signalregion_dilepton_inclusive(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign leptons,
    // inclusive in lepton flavours.
    // legacy version used before 06/02/2023, maybe return to it later
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,2,true)) return false;
    if(not passDiLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 2) ) return false;
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ if( event.leptonsAreSameSign() ) return false; }
    else{ if( !event.leptonsAreSameSign() ) return false; }
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
    if(selectbjets){ if( njetsnbjets.second < 1 && njetsnloosebjets.second < 2 ) return false; }
    if( njetsnbjets.first < 2 ) return false;
    return true;
}*/

bool pass_signalregion_dilepton_inclusive(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign leptons,
    // inclusive in lepton flavours.
    // new version used on 06/02/2023 for dummy xsec measurement sync with Oviedo.
    cleanLeptonsAndJets(event);
    // apply trigger, pt thresholds and mll veto
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,2,true)) return false;
    if(!passMllMassVeto(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 2) ) return false;
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25. 
        || event.leptonCollection()[1].pt() < 15. ) return false;
    // re-added on 20/02/2023: invariant mass safety
    if( event.leptonSystem().mass()<30. ) return false;
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ if( event.leptonsAreSameSign() ) return false; }
    else{ if( !event.leptonsAreSameSign() ) return false; }
    // Z veto for electrons
    if( event.leptonCollection()[0].isElectron() 
        && event.leptonCollection()[1].isElectron()
        && event.hasZTollCandidate(10., true) ) return false;
    // MET cut for electrons
    // re-added on 18/04/2023: MET cut for all lepton flavours
    if( variation=="all" ){ if(event.met().maxPtAnyVariation()<30) return false; } 
    else{ if(event.getMet(variation).pt()<30.) return false; }
    // number of jets and b-jets
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( selectbjets ){ if( njetsnloosebjets.second < 2 ) return false; }
    if( njetsnbjets.first < 3 ) return false;
    return true;
}

/*std::tuple<int,std::string> eventSelections::pass_signalregion_dilepton_inclusive_cutflow(
    Event& event, 
    const std::string& selectiontype,
    const std::string& variation, 
    const bool selectbjets){
    // copy of pass_signalregion_dilepton_inclusive
    // but different return type to allow cutflow studies
    // legacy version used before 06/02/2023, maybe return to it later
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return std::make_tuple(0, "Fail MET filters");
    if(not passAnyTrigger(event)) return std::make_tuple(1, "Fail trigger");
    if(!hasnFOLeptons(event,2,true)) return std::make_tuple(2, "Fail 2 FO leptons");
    if(not passDiLeptonPtThresholds(event)) return std::make_tuple(3, "Fail pT thresholds");
    if(not passPhotonOverlapRemoval(event)) return std::make_tuple(4, "Fail photon overlap");
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 2) ) return std::make_tuple(5, "Fail 2 tight leptons");
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ 
	if( event.leptonsAreSameSign() ) return std::make_tuple(6, "Fail same sign"); }
    else{ if( !event.leptonsAreSameSign() ) return std::make_tuple(6, "Fail same sign"); }
    // Z veto for electrons
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isElectron()
        && event.hasZTollCandidate(halfwindow_wide, true) ){ 
	return std::make_tuple(7, "Fail electron Z veto"); }
    // invariant mass safety
    if( event.leptonSystem().mass()<30. ) return std::make_tuple(8, "Fail low mass veto");
    // MET
    if( variation=="all" ){ 
	if(event.met().maxPtAnyVariation()<30) return std::make_tuple(9, "Fail MET"); }
    else{ if(event.getMet(variation).pt()<30.) return std::make_tuple(9, "Fail MET"); }
    // number of jets and b-jets
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnbjets.second < 1 && njetsnloosebjets.second < 2 ){ 
	return std::make_tuple(10, "Fail b-jets"); }
    if( njetsnbjets.first < 2 ) return std::make_tuple(11, "Fail jets");
    if(selectbjets){} // dummy to avoid unused parameter warning
    return std::make_tuple(12, "Pass");
}*/

std::tuple<int,std::string> eventSelections::pass_signalregion_dilepton_inclusive_cutflow(
    Event& event, 
    const std::string& selectiontype,
    const std::string& variation,
    const bool selectbjets){
    // copy of pass_signalregion_dilepton_inclusive
    // but different return type to allow cutflow studies
    // new version used on 06/02/2023 for dummy xsec measurement sync with Oviedo.
    cleanLeptonsAndJets(event);
    // apply trigger, pt thresholds and mll veto
    if(not event.passMetFilters()) return std::make_tuple(0, "Fail MET filters");
    if(not passAnyTrigger(event)) return std::make_tuple(1, "Fail trigger");
    if(!hasnFOLeptons(event,2,true)) return std::make_tuple(2, "Fail 2 FO leptons");
    if(!passMllMassVeto(event)) return std::make_tuple(3, "Fail low mass veto");
    if(not passPhotonOverlapRemoval(event)) return std::make_tuple(4, "Fail photon overlap");
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 2) ) return std::make_tuple(5, "Fail 2 tight leptons");
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 15. ) return std::make_tuple(6, "Fail pT thresholds");
    // re-added on 20/02/2023: invariant mass safety
    if( event.leptonSystem().mass()<30. ) return std::make_tuple(7, "Fail invariant mass veto");
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ if( event.leptonsAreSameSign() ) return std::make_tuple(8, "Fail same sign"); }
    else{ if( !event.leptonsAreSameSign() ) return std::make_tuple(8, "Fail same sign"); }
    // Z veto for electrons
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isElectron()
        && event.hasZTollCandidate(10., true) ) return std::make_tuple(9, "Fail electron Z veto");
    // MET cut for electrons
    // re-added on 18/04/2023: MET cut for all lepton flavours
    if( variation=="all" ){ if(event.met().maxPtAnyVariation()<30) return std::make_tuple(10, "Fail MET"); }
    else{ if(event.getMet(variation).pt()<30.) return std::make_tuple(10, "Fail MET"); }
    // number of jets and b-jets
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnloosebjets.second < 2 ) return std::make_tuple(11, "Fail b-jets");
    if( njetsnbjets.first < 3 ) return std::make_tuple(12, "Fail jets");
    if(selectbjets){} // dummy to avoid unused parameter warning
    return std::make_tuple(13, "Pass");
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

bool pass_signalregion_dilepton_pee(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign electrons
    if( !pass_signalregion_dilepton_inclusive(event,
        selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
         && event.leptonCollection()[1].isElectron()
         && event.leptonCollection()[0].charge() == 1 ){ return true; }
    return false;
    }

bool pass_signalregion_dilepton_nee(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign electrons
    if( !pass_signalregion_dilepton_inclusive(event,
        selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
         && event.leptonCollection()[1].isElectron() 
         && event.leptonCollection()[0].charge() == -1 ){ return true; }
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

bool pass_signalregion_dilepton_pem(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign electron and muon
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isMuon() 
        && event.leptonCollection()[0].charge() == 1 ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_nem(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign electron and muon
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isElectron()
        && event.leptonCollection()[1].isMuon() 
        && event.leptonCollection()[0].charge() == -1 ){ return true; }
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

bool pass_signalregion_dilepton_pme(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign muon and electron
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isElectron() 
        && event.leptonCollection()[0].charge() == 1 ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_nme(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with same sign muon and electron
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isElectron() 
        && event.leptonCollection()[0].charge() == -1 ){ return true; }
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

bool pass_signalregion_dilepton_pmm(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign muons
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isMuon() 
        && event.leptonCollection()[0].charge() == 1 ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_nmm(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with two same sign muons
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].isMuon()
        && event.leptonCollection()[1].isMuon() 
        && event.leptonCollection()[0].charge() == -1 ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_plus(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    // signal region with positive sign leptons
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].charge()==1
        && event.leptonCollection()[1].charge()==1 ){ return true; }
    return false;
}

bool pass_signalregion_dilepton_minus(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets){
    // signal region with negative sign leptons
    if( !pass_signalregion_dilepton_inclusive(event,
            selectiontype, variation, selectbjets) ){ return false; }
    if( event.leptonCollection()[0].charge()==-1
        && event.leptonCollection()[1].charge()==-1 ){ return true; }
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
    if( !doLeptonSelection(event, selectiontype, 3) ) return false;
    // Z candidate veto
    if( event.hasOSSFLightLeptonPair() && event.hasZTollCandidate(halfwindow) ) return false;
    // invariant mass safety
    if(not passMllMassVeto(event)) return false;
    // sum of charges needs to be 1 or -1
    if( !event.hasOSLeptonPair() ) return false;
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( selectbjets ){ if( njetsnbjets.second < 1 ) return false; }
    if( njetsnbjets.first < 2 ) return false;
    return true; 
}

std::tuple<int,std::string> eventSelections::pass_signalregion_trilepton_cutflow(
    Event& event, 
    const std::string& selectiontype,
    const std::string& variation, 
    const bool selectbjets){
    // copy of pass_signalregion_trilepton but with different return type
    // to allow cutflow studies
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return std::make_tuple(0, "Fail MET filters");
    if(not passAnyTrigger(event)) return std::make_tuple(1, "Fail trigger");
    if(!hasnFOLeptons(event,3,true)) return std::make_tuple(2, "Fail 3 FO leptons");
    if(not passTriLeptonPtThresholds(event)) return std::make_tuple(3, "Fail pT thresholds");
    if(not passPhotonOverlapRemoval(event)) return std::make_tuple(4, "Fail photon overlap");
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 3) ) return std::make_tuple(5, "Fail 3 tight leptons");
    // Z candidate veto
    if( event.hasOSSFLightLeptonPair() && event.hasZTollCandidate(halfwindow) ){ 
	return std::make_tuple(6, "Fail Z veto"); }
    // invariant mass safety
    if(not passMllMassVeto(event)) return std::make_tuple(7, "Fail low mass veto");
    // sum of charges needs to be 1 or -1
    if( !event.hasOSLeptonPair() ) return std::make_tuple(8, "Fail OS pair");
    // number of jets and b-jets
    std::pair<int,int> njetsnbjets = nJetsNBJets(event, variation);
    if( njetsnbjets.second < 1 ) return std::make_tuple(9, "Fail b-jets");
    if( njetsnbjets.first < 2 ) return std::make_tuple(10, "Fail jets");
    if(selectbjets){} // dummy to avoid unused parameter warning
    return std::make_tuple(11, "Pass");
}

// -----------------------
// prompt control regions 
// -----------------------

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
    if( !doLeptonSelection(event, selectiontype, 3) ) return false;
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
    if( !doLeptonSelection(event, selectiontype, 4) ) return false;
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
    if(!passMllMassVeto(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    //if(not passTriLeptonPtThresholds(event)) return false;
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 20.
        || event.leptonCollection()[2].pt() < 10.) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 3) ) return false;
    // mass constraints on OSSF pair and trilepton system
    if(!event.hasOSSFLightLeptonPair()) return false;
    //if(event.bestZBosonCandidateMass()>75.) return false; 
    // (enable cut above for partial syncing with below)
    if(fabs(event.leptonSystem().mass()-particle::mZ)>halfwindow_wide) return false;
    bool pairZmass = false;
    for(LeptonCollection::const_iterator lIt1 = event.leptonCollection().cbegin();
        lIt1 != event.leptonCollection().cend(); lIt1++){
        Lepton& lep1 = **lIt1;
	for(LeptonCollection::const_iterator lIt2 = lIt1+1; 
	    lIt2!=event.leptonCollection().cend(); lIt2++){
	    Lepton& lep2 = **lIt2;
	    if(fabs((lep1+lep2).mass()-particle::mZ)<halfwindow_wide) pairZmass = true;
            if(oppositeSignSameFlavor(lep1,lep2)
               && fabs((lep1+lep2).mass()-particle::mZ)<halfwindow_wide) pairZmass = true;
	    if(oppositeSignSameFlavor(lep1,lep2) 
               && (lep1+lep2).mass() < 35.) return false;
	    // (disable cut above for partial syncing with below)
	}
    }
    if(pairZmass) return false;
    // (disable cut above for partial syncing with below)
    // dummy condition on variation to avoid warnings
    if(variation=="dummy") return true;
    if(selectbjets){}
    return true;
}

/*bool pass_zgcontrolregion(Event& event, const std::string& selectiontype,
                            const std::string& variation, const bool selectbjets){
    // control region focusing on conversions,
    // ALTERNATIVE VERSION FOR SYNCING WITH NIELS FOR SOME CHECKS
    // i.e. three leptons making a Z boson.
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!passMllMassVeto(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 20.
        || event.leptonCollection()[2].pt() < 10.) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 3) ) return false;
    // something called 'lean selection'
    // not needed in principle since nloosebjets is always zero (see below)
    std::pair<int,int> njetsnloosebjets = nJetsNLooseBJets(event, variation);
    bool passlean = ( njetsnloosebjets.first >= 2 
			&& njetsnloosebjets.second >= 1 
			&& event.getJetCollection(variation).scalarPtSum() > 200 );
    if( passlean ) return false;
    // mass constraints on OSSF pair and trilepton system
    if(fabs(event.leptonSystem().mass()-particle::mZ)>15.) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(event.bestZBosonCandidateMass()>75.) return false;
    // dummy condition on variation to avoid warnings
    if(variation=="dummy") return true;
    if(selectbjets){
	if(njetsnloosebjets.second!=0) return false;
    }
    return true;
}*/

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
    if( !doLeptonSelection(event, selectiontype, 3) ) return false;
    // Z candidate
    if( !(event.hasOSSFLightLeptonPair() && event.hasZTollCandidate(halfwindow)) ) return false;
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
    if( !doLeptonSelection(event, selectiontype, 4) ) return false;
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
    if( !doLeptonSelection(event, selectiontype, 2) ) return false;
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ if( event.leptonsAreSameSign() ) return false; }
    else{ if( !event.leptonsAreSameSign() ) return false; }
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
    if(selectbjets){ if( njetsnbjets.second < 1 && njetsnloosebjets.second < 2 ) return false; }
    if( njetsnbjets.first < 2 ) return false;
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
    // control region for charge flips with two same sign electrons on the Z peak
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event, 2, true)) return false;
    if(not passDiLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // do lepton selection for different types of selections
    if( !doLeptonSelection(event, selectiontype, 2) ) return false;
    // select di-electron events
    if(event.leptonCollection()[0].isMuon() || event.leptonCollection()[1].isMuon()) return false;
    // leptons must be same sign
    if( selectiontype=="chargeflips" ){ if( event.leptonsAreSameSign() ) return false; }
    else{ if( !event.leptonsAreSameSign() ) return false; }
    // leptons must make a Z candidate
    if(!event.hasZTollCandidate(halfwindow, true)) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}
