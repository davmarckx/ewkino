#include "../interface/skimSelections.h"

//include c++ library classes
#include <functional>


bool passLeptonicSkim( Event& event, LeptonCollection::size_type numberOfLeptons ){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    return ( event.numberOfLeptons() >= numberOfLeptons );
}


bool passSingleLeptonSkim( Event& event ){
    return passLeptonicSkim( event, 1 );
}


bool passDileptonSkim( Event& event ){
    return passLeptonicSkim( event, 2 );
}


bool passTrileptonSkim( Event& event ){
    return passLeptonicSkim( event, 3 );
}


bool passFourLeptonSkim( Event& event ){
    return passLeptonicSkim( event, 4 );
}


bool passFakeRateSkim( Event& event ){
    // original: 
    /*event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    if( event.numberOfLightLeptons() != 1 ) return false;
    event.selectGoodJets();
    event.cleanJetsFromLooseLeptons();
    if( event.numberOfJets() < 1 ) return false;
    return true;*/

    // replace by:
    // for data: require to pass a fake rate trigger
    if( event.isData() && !event.passTriggers_FR() ) return false;
    // remainder: copy of passSingleLeptonSkim
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    if( event.numberOfLightLeptons() < 1 ) return false;
    return true;
}

bool passTriFOLeptonSkim( Event& event){
    // special skim for the TT and DY samples to be used in MC closure tests
    event.removeTaus();
    event.applyLeptonConeCorrection();
    event.cleanElectronsFromLooseMuons();
    event.selectFOLeptons();
    return (event.numberOfLeptons() >= 3);
}

bool passDilightleptonSkim( Event& event){
    // special skim analogous to dilepton but removing taus to save memory
    if( !passLeptonicSkim( event, 2 ) ) return false;
    event.removeTaus();
    return ( event.numberOfLeptons() >= 2);
}

bool passSkim( Event& event, const std::string& skimCondition ){
    static std::map< std::string, std::function< bool(Event&) > > skimFunctionMap = {
        { "noskim", [](Event&){ return true; } },
        { "singlelepton", passSingleLeptonSkim },
        { "dilepton", passDileptonSkim },
        { "trilepton", passTrileptonSkim },
        { "fourlepton", passFourLeptonSkim },
        { "fakerate", passFakeRateSkim },
	// added the following selection:
	{ "trifolepton", passTriFOLeptonSkim },
	{ "dilightlepton", passDilightleptonSkim}
    };
    auto it = skimFunctionMap.find( skimCondition );
    if( it == skimFunctionMap.cend() ){
        throw std::invalid_argument( "unknown skim condition " + skimCondition );
    } else {
        return (it->second)(event);
    }
}
