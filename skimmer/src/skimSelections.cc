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
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    if( event.numberOfLightLeptons() != 1 ) return false;

    event.selectGoodJets();
    event.cleanJetsFromLooseLeptons();
    if( event.numberOfJets() < 1 ) return false;
    return true;
}


bool passDiLightLeptonSkim( Event& event ){
    // event passes if
    // - number of light leptons >= 2
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.removeTaus();
    if( event.numberOfLightLeptons() < 2 ) return false;
    return true;
}


bool passMultiLightLeptonSkim( Event& event ){
    // event passes if
    // - number of light leptons >= 3
    // - OR number of light leptons = 2 and they are same sign
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.removeTaus();
    if( event.numberOfLightLeptons() > 2 ) return true;
    if( event.numberOfLightLeptons() < 2 ) return false;
    return (event.leptonCollection()[0].charge()==event.leptonCollection()[1].charge());
}


bool passSkim( Event& event, const std::string& skimCondition ){
    static std::map< std::string, std::function< bool(Event&) > > skimFunctionMap = {
        { "noskim", [](Event&){ return true; } },
        { "singlelepton", passSingleLeptonSkim },
        { "dilepton", passDileptonSkim },
        { "trilepton", passTrileptonSkim },
        { "fourlepton", passFourLeptonSkim },
        { "fakerate", passFakeRateSkim },
	{ "dilightlepton", passDiLightLeptonSkim },
	{ "multilightlepton", passMultiLightLeptonSkim }
    };
    auto it = skimFunctionMap.find( skimCondition );
    if( it == skimFunctionMap.cend() ){
        throw std::invalid_argument( "unknown skim condition " + skimCondition );
    } else {
        return (it->second)(event);
    }
}
