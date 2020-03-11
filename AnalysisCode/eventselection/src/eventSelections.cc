// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection){
    static std::map< std::string, std::function< bool(Event&) > > ESFunctionMap = {
        { "signalregion", pass_signalregion },
        { "wzcontrolregion", pass_wzcontrolregion },
        { "zzcontrolregion", pass_zzcontrolregion },
        { "zgcontrolregion", pass_zgcontrolregion },
        { "ttzcontrolregion", pass_ttzcontrolregion },
        { "fr_QCD_FO", pass_fr_QCD_FO},
        { "fr_QCD_Tight", pass_fr_QCD_Tight},
        { "fr_EW_FO", pass_fr_EW_FO},
        { "fr_EW_Tight", pass_fr_EW_Tight}
    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    } else {
        return (it->second)(event);
    }
}

// help functions for signal and control region selections //

constexpr double halfwindow = 7.5;

void cleanleptoncollection(Event& event){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
}

void cleanjetcollection(Event& event){
    event.cleanJetsFromLooseLeptons();
}

bool hasnFOLeptons(Event& event, int n){
    event.selectFOLeptons();
    int nFOLeptons = event.numberOfLeptons();
    if(nFOLeptons == n) return true;
    return false;
}

bool hasnTightLeptons(Event& event, int n){
    event.selectTightLeptons();
    int nTightLeptons = event.numberOfLeptons();
    if(nTightLeptons == n) return true;
    return false;
}

int eventCategory(Event& event){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!
    int njets = event.numberOfJets();
    int nbjets = event.numberOfMediumBTaggedJets();
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    if(nbjets == 1) return 2;
    return 3;
}

// dedicated functions to check if event passes certain conditions //

bool pass_signalregion(Event& event){
    // clean jet collection (warning: need to check whether after or before lepton cleaning)
    cleanjetcollection(event);
    // clean lepton collections (see also ewkino/skimmer/src/skimSelections.cc)
    cleanleptoncollection(event);
    // select 'good' jets (see ewkino/objectSelection/jetSelector.cc for definition)
    event.selectGoodJets();
    // select FO leptons
    if(!hasnFOLeptons(event,3)) return false;    
    // select tight leptons
    if(!hasnTightLeptons(event,3)) return false;
    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // number of jets and b-jets
    if(eventCategory(event)==-1) return false;
    return true;
}

bool pass_wzcontrolregion(Event& event){
    // very similar to signal region but b-jet veto and other specificities
    // cleaning and selecting leptons is done implicitly in pass_signalregion
    cleanjetcollection(event);
    cleanleptoncollection(event);
    event.selectGoodJets();
    if(!hasnFOLeptons(event,3)) return false;
    if(!hasnTightLeptons(event,3)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    std::cout<<"before selection"<<std::endl;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
      jIt != event.jetCollection().cend(); jIt++){
	Jet& jet = **jIt;
	std::cout<<jet.deepCSV()<<std::endl;
	std::cout<<jet.isBTaggedMedium()<<std::endl;
    }
    std::cout<<"number of b jets "<<event.numberOfMediumBTaggedJets()<<std::endl;
    if(event.numberOfMediumBTaggedJets()>0) return false;
    if(event.metPt()<50.) return false;
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    event.selectGoodJets();
    if(!hasnFOLeptons(event,4)) return false;
    if(!hasnTightLeptons(event,4)) return false;
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
    return true;
}

bool pass_zgcontrolregion(Event& event){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    event.selectGoodJets();
    if(!hasnFOLeptons(event,3)) return false;
    if(!hasnTightLeptons(event,3)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(fabs(event.leptonSystem().mass()-particle::mZ)>halfwindow) return false;
    bool pairZmass = false;
    for(LeptonCollection::const_iterator lIt1 = event.leptonCollection().cbegin();
        lIt1 != event.leptonCollection().cend(); lIt1++){
        Lepton& lep1 = **lIt1;
	for(LeptonCollection::const_iterator lIt2 = lIt1+1; lIt2!=event.leptonCollection().cend(); lIt2++){
	    Lepton& lep2 = **lIt2;
	    if(fabs((lep1+lep2).mass()-particle::mZ)<halfwindow) pairZmass = true;
	}
    }
    if(pairZmass) return false;
    return true;
}

bool pass_ttzcontrolregion(Event& event){
    // no dedicated ttz control region yet (see analysis note)
    return false;
}

// help functions for fake rate determination //

bool hasgoodjet(Event& event){
    // helper function for fake rate event selection
    // determines whether the event has a jet with high pT separated from leptons

    bool hasgoodjet = false;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin(); 
        jIt <= event.jetCollection().cend(); jIt++){
        Jet& jet = **jIt;
        if(jet.pt()<30.) continue;
        for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
            lIt <= event.leptonCollection().cend(); lIt++){
            Lepton& lep = **lIt;
            if(deltaR(jet,lep)>1.) hasgoodjet = true;
        }
    }
    return hasgoodjet;
}

bool pass_fr_QCD_FO(Event& event){
    // select events targeting QCD background with one FO lepton

    cleanjetcollection(event);
    cleanleptoncollection(event);
    // condition on number of leptons
    event.removeTaus();
    event.selectFOLeptons();
    if(event.numberOfLeptons() != 1) return false;
    // condition on ptmiss and mt
    if(event.metPt()>20.) return false;
    if(event.mtLeptonMet(0)>20.) return false;
    return hasgoodjet(event);
}

bool pass_fr_QCD_Tight(Event& event){
    // select events targeting QCD background with one tight lepton

    if(!pass_fr_QCD_FO(event)) return false;
    event.selectTightLeptons();
    if(event.numberOfLeptons() != 1) return false;
    return true;
}

bool pass_fr_EW_FO(Event& event){
    // select events targeting EW contamination with one FO lepton

    cleanjetcollection(event);
    cleanleptoncollection(event);
    // condition on number of leptons
    event.removeTaus();
    event.selectFOLeptons();
    if(event.numberOfLeptons() != 1) return false;
    // conditions on ptmiss and mt
    if(event.metPt()<20.) return false;
    if(event.mtLeptonMet(0)<80. or event.mtLeptonMet(0)>150.) return false;
    return hasgoodjet(event);
}

bool pass_fr_EW_Tight(Event& event){
    // select events targeting EW contamination with one tight lepton
    
    if(!pass_fr_EW_FO(event)) return false;
    event.selectTightLeptons();
    if(event.numberOfLeptons() != 1) return false;
    return true;
}
