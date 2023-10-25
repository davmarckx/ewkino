/*
Particle-level event variable computing.
This file is similar to eventFlattening.cc,
but uses particle-level variables rather than detector-level variables.
*/


// include header
#include "../interface/eventFlatteningParticleLevel.h"


// helper function to initialize a map of variables to default values
std::map< std::string, double > eventFlatteningParticleLevel::initVarMap(){
    // initialize a map of variables set to their default values
    // note: only simple variables for now, extend when needed.
    std::map< std::string, double> varmap = {
	{"_yield",0.5},
	{"_nJets",0}, {"_nBJets",0},
	{"_nLooseBJets",0},
	{"_nMuons",0},{"_nElectrons",0},
	{"_leptonPtLeading",0.}, {"_leptonPtSubLeading",0.}, {"_leptonPtTrailing",0.},
	{"_leptonEtaLeading",0.}, {"_leptonEtaSubLeading",0.}, {"_leptonEtaTrailing",0.},
        {"_leptonAbsEtaLeading",0.}, {"_leptonAbsEtaSubLeading",0.}, {"_leptonAbsEtaTrailing",0.},
	{"_jetPtLeading",0.}, {"_jetPtSubLeading",0.},
	{"_jetEtaLeading",0.}, {"_jetEtaSubLeading",0.},
        {"_jetAbsEtaLeading",0.}, {"_jetAbsEtaSubLeading",0.},
        {"_bjetPtLeading",0.}, {"_bjetEtaLeading",0.}, {"_bjetAbsEtaLeading",0.},
	{"_nJetsNBJetsCat",-1}, {"_HT",0.},
	{"_dRl1jet",99.}, {"_dRl1bjet",99.}, {"_dRl1l2",99.},
	{"_deltaEtaLeadingLeptonPair",0.}, {"_leptonMaxEta",0.},
	{"_M3l",0.}, {"_leptonPtSum",0.}
    };
    return varmap;    
}

 
// main function //

std::map< std::string, double > eventFlatteningParticleLevel::eventToEntry( Event& event ){
    // fill one entry with event variables.

    // initialize all variables in the map
    std::map< std::string, double > varmap = initVarMap();
 
    // get correct object collections and met
    JetParticleLevelCollection jetcollection = event.jetParticleLevelCollection();
    JetParticleLevelCollection bjetcollection = jetcollection.PLbJetCollection();
    MetParticleLevel met = event.metParticleLevel();
    LeptonParticleLevelCollection lepcollection = event.leptonParticleLevelCollection();

    // remove taus
    lepcollection.removeTaus();

    // sort leptons and jets by pt
    lepcollection.sortByPt();
    jetcollection.sortByPt();

    // yield (fixed value of 0.5)
    varmap["_yield"] = 0.5;

    // number of muons and electrons
    varmap["_nMuons"] = lepcollection.numberOfMuons();
    varmap["_nElectrons"] = lepcollection.numberOfElectrons();

    // lepton pt and eta
    if(lepcollection.numberOfLightLeptons()>=1){
	varmap["_leptonPtLeading"] = lepcollection[0].pt();
	varmap["_leptonEtaLeading"] = lepcollection[0].eta();
        varmap["_leptonAbsEtaLeading"] = fabs(lepcollection[0].eta());
        varmap["_leptonMaxEta"] = varmap["_leptonAbsEtaLeading"];

	if(jetcollection.size()>=1){
	    for(JetParticleLevelCollection::const_iterator jIt = jetcollection.cbegin();
		jIt != jetcollection.cend(); jIt++){
		JetParticleLevel& jet = **jIt;
		if(deltaR(lepcollection[0],jet)<varmap["_dRl1jet"]) varmap["_dRl1jet"] = deltaR(lepcollection[0],jet);
	    }
	}
	if(bjetcollection.size()>=1){
	    for(JetParticleLevelCollection::const_iterator jIt = bjetcollection.cbegin();
		jIt != bjetcollection.cend(); jIt++){
		JetParticleLevel& jet = **jIt;
		if(deltaR(lepcollection[0],jet)<varmap["_dRl1bjet"]) varmap["_dRl1bjet"] = deltaR(lepcollection[0],jet);
	    }
	}
    }

    if(lepcollection.numberOfLightLeptons()>=2){
	varmap["_leptonPtSubLeading"] = lepcollection[1].pt();
	varmap["_leptonEtaSubLeading"] = lepcollection[1].eta();
        varmap["_leptonAbsEtaSubLeading"] = fabs(lepcollection[1].eta());
        if( varmap["_leptonAbsEtaSubLeading"] > varmap["_leptonMaxEta"]){ varmap["_leptonMaxEta"] = varmap["_leptonAbsEtaSubLeading"]; }
        varmap["_dRl1l2"] = deltaR(lepcollection[0],lepcollection[1]);
	varmap["_deltaEtaLeadingLeptonPair"] = deltaEta(lepcollection[0],lepcollection[1]);
    }
    if(lepcollection.numberOfLightLeptons()>=3){
	varmap["_leptonPtTrailing"] = lepcollection[2].pt();
        varmap["_leptonEtaTrailing"] = lepcollection[2].eta();
        varmap["_leptonAbsEtaTrailing"] = fabs(lepcollection[2].eta());
    }

    // number of jets and b-jets
    varmap["_nJets"] = jetcollection.numberOfJets();
    varmap["_nBJets"] = jetcollection.numberOfBJets();
    varmap["_nLooseBJets"] = varmap["_nBJets"];
    // (just for syntax, there needs to be an entry nLooseBJets to correspond to the same detector level variable)

    // HT and similar pt sums
    varmap["_HT"] = jetcollection.scalarPtSum();
    varmap["_leptonPtSum"] = lepcollection.scalarPtSum();

    // invariant masses
    varmap["_M3l"] = lepcollection.mass();

    // jet pt and eta
    jetcollection.sortByPt();
    if(jetcollection.numberOfJets()>=1){
	varmap["_jetPtLeading"] = jetcollection[0].pt();
	varmap["_jetEtaLeading"] = jetcollection[0].eta();
        varmap["_jetAbsEtaLeading"] = fabs(jetcollection[0].eta());
    }
    if(jetcollection.numberOfJets()>=2){
	varmap["_jetPtSubLeading"] = jetcollection[1].pt();
	varmap["_jetEtaSubLeading"] = jetcollection[1].eta();
        varmap["_jetAbsEtaSubLeading"] = fabs(jetcollection[1].eta());
    }

    // b jet pt
    bjetcollection.sortByPt();
    if(bjetcollection.numberOfJets()>=1){
        varmap["_bjetPtLeading"] = bjetcollection[0].pt();
        varmap["_bjetEtaLeading"] = bjetcollection[0].eta();
        varmap["_bjetAbsEtaLeading"] = fabs(bjetcollection[0].eta());
    }

    // definition of categorization variables
    int njets = varmap["_nJets"];
    int nbjets = varmap["_nBJets"];

    // (note: default is -1)
    if( nbjets==0 ) varmap["_nJetsNBJetsCat"] = std::min(njets,4);
    else if( nbjets==1 ) varmap["_nJetsNBJetsCat"] = 5 + std::min(njets,4) - 1;
    else if( nbjets>1 ) varmap["_nJetsNBJetsCat"] = 9 + std::min(njets,4) - 2;

    // now return the varmap (e.g. to fill histograms)
    return varmap;
}
