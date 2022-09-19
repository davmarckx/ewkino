#include "../interface/ewkinoSelection.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../constants/particleMasses.h"

//include ewkino categorization
#include "../interface/ewkinoCategorization.h"


void ewkino::applyBaselineObjectSelection( Event& event, const bool allowUncertainties ){

    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.cleanJetsFromFOLeptons();
    if( allowUncertainties ){
        event.jetCollection().selectGoodAnyVariationJets();
    } else {
        event.selectGoodJets();
    }
}


bool ewkino::passLowMllVeto( const Event& event, const double vetoValue ){

    for( const auto& leptonPtrPair : event.lightLeptonCollection().pairCollection() ){

        //veto OSSF pairs of low mass
        Lepton& lepton1 = *( leptonPtrPair.first );
        Lepton& lepton2 = *( leptonPtrPair.second );
        if( !oppositeSignSameFlavor( lepton1, lepton2 ) ){
            continue;
        }
        if( ( lepton1 + lepton2 ).mass() < vetoValue ){
            return false;
        }
    }
    return true;
}


bool ewkino::passBaselineSelection( Event& event, const bool allowUncertainties, const bool bVeto, const bool mllVeto ){
    
    applyBaselineObjectSelection( event, allowUncertainties );
    if( event.numberOfLeptons() < 3 ) return false;
    if( mllVeto && !passLowMllVeto( event, 12 ) ) return false;
    event.selectFOLeptons();
    if( event.numberOfLeptons() < 3 ) return false;
    event.applyLeptonConeCorrection();
    event.sortLeptonsByPt();
    if( event.numberOfTaus() > 2 ) return false;
    if( bVeto ){
        if( allowUncertainties ){
            if( event.jetCollection().minNumberOfTightBTaggedJetsAnyVariation() > 0 ){
                return false;
            }
        } else {
            if( event.numberOfTightBTaggedJets() > 0 ){
                return false;
            }
        }
    }
    return true;
}


JetCollection ewkino::variedJetCollection( const Event& event, const std::string& uncertainty ){
    if( uncertainty == "nominal" ){
        return event.jetCollection().goodJetCollection();
    } else if( uncertainty == "JECDown" ){
        return event.jetCollection().JECDownCollection().goodJetCollection();
    } else if( uncertainty == "JECUp" ){
        return event.jetCollection().JECUpCollection().goodJetCollection();
    } else if( uncertainty == "JERDown" ){
        return event.jetCollection().JERDownCollection().goodJetCollection();
    } else if( uncertainty == "JERUp" ){
        return event.jetCollection().JERUpCollection().goodJetCollection();
    } else if( uncertainty == "UnclDown" ){
        return event.jetCollection().goodJetCollection();
    } else if( uncertainty == "UnclUp" ){
        return event.jetCollection().goodJetCollection();
    } else {
        throw std::invalid_argument( "Uncertainty source " + uncertainty + " is unknown." );
    }
}


JetCollection::size_type ewkino::numberOfVariedBJets( const Event& event, const std::string& uncertainty ){
    return ewkino::variedJetCollection( event, uncertainty ).numberOfTightBTaggedJets();
}


Met ewkino::variedMet( const Event& event, const std::string& uncertainty ){
    if( uncertainty == "nominal" ){
        return event.met();
    } else if( uncertainty == "JECDown" ){
        return event.met().MetJECDown();
    } else if( uncertainty == "JECUp" ){
        return event.met().MetJECUp();
    } else if( uncertainty == "JERDown" ){
        return event.met();
    } else if( uncertainty == "JERUp" ){
        return event.met();
    } else if( uncertainty == "UnclDown" ){
        return event.met().MetUnclusteredDown();
    } else if( uncertainty == "UnclUp" ){
        return event.met().MetUnclusteredUp();
    } else {
        throw std::invalid_argument( "Uncertainty source " + uncertainty + " is unknown." );
    }
}


bool ewkino::passVariedSelection( const Event& event, const std::string& uncertainty ){
    static constexpr double metCut = 50;
    if( numberOfVariedBJets( event, uncertainty ) > 0 ) return false;
    if( variedMet( event, uncertainty ).pt() < metCut ) return false;
    return true;
}


bool ewkino::passVariedSelectionWZCR( Event& event, const std::string& uncertainty ){
    static constexpr double minMet = 30;
    static constexpr double maxMet = 100;
    static constexpr double minMT = 50;
    static constexpr double maxMT = 100;
    if( ewkino::ewkinoCategory( event ) != ewkino::trilepLightOSSF ) return false;
    if( numberOfVariedBJets( event, uncertainty ) > 0 ) return false;
    Met met = variedMet( event, uncertainty );
    if( met.pt() < minMet || met.pt() > maxMet ) return false;
    if( std::abs( event.bestZBosonCandidateMass() - particle::mZ ) >= 15 ) return false;
    double mTW = mt( met, event.WLepton() );
    if( mTW < minMT || mTW > maxMT ) return false;
    return true;
}


bool ewkino::passVariedSelectionTTZCR( Event& event, const std::string& uncertainty ){
    static constexpr size_t numberOfBJets = 1;
    if( ewkino::ewkinoCategory( event ) != ewkino::trilepLightOSSF ) return false;
    if( numberOfVariedBJets( event, uncertainty ) < numberOfBJets ) return false;
    if( std::abs( event.bestZBosonCandidateMass() - particle::mZ ) >= 15 ) return false;
    if( std::abs( event.leptonCollection().objectSum().mass() - particle::mZ ) < 15 ) return false;
    return true;
}


bool ewkino::passVariedSelectionNPCR( Event& event, const std::string& uncertainty ){
    static constexpr size_t numberOfBJets = 1;
    if( numberOfVariedBJets( event, uncertainty ) < numberOfBJets ) return false;
    if( ewkino::ewkinoCategory( event ) == ewkino::trilepLightOSSF ){
        if( std::abs( event.bestZBosonCandidateMass() - particle::mZ ) < 15 ) return false;
    } else {
        if( ewkino::ewkinoCategory( event ) != ewkino::trilepLightNoOSSF ) return false;
    }
    return true;
}


bool ewkino::passVariedSelectionXGammaCR( Event& event, const std::string& uncertainty ){
    if( numberOfVariedBJets( event, uncertainty ) > 0 ) return false;
    if( ewkino::ewkinoCategory( event ) != ewkino::trilepLightOSSF ) return false;
    if( variedMet( event, uncertainty ).pt() >= 50 ) return false;
    if( event.bestZBosonCandidateMass() >= 75 ) return false;
    if( std::abs( event.leptonCollection().objectSum().mass() - particle::mZ ) >= 15 ) return false;
    return true;
}


bool ewkino::passTriggerSelection( const Event& event ){
    if( event.numberOfMuons() >= 1 ){
        if( event.passTriggers_m() ) return true;
    } 
    if( event.numberOfMuons() >= 2 ){
        if( event.passTriggers_mm() ) return true;
    }
    if( event.numberOfMuons() >= 3 ){
        if( event.passTriggers_mmm() ) return true;
    }
    if( event.numberOfElectrons() >= 1 ){
        if( event.passTriggers_e() ) return true;
    }
    if( event.numberOfElectrons() >= 2 ){
        if( event.passTriggers_ee() ) return true;
    }
    if( event.numberOfElectrons() >= 3 ){
        if( event.passTriggers_eee() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfElectrons() >= 1 ) ){
        if( event.passTriggers_em() ) return true;
    }
    if( ( event.numberOfMuons() >= 2 ) && ( event.numberOfElectrons() >= 1 ) ){
        if( event.passTriggers_emm() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfElectrons() >= 2 ) ){
        if( event.passTriggers_eem() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfTaus() >= 1 ) ){
        if( event.passTriggers_mt() ) return true;
    }
    if( ( event.numberOfElectrons() >= 1 ) && ( event.numberOfTaus() >= 1 ) ){
        if( event.passTriggers_et() ) return true;
    }

    return false;
}


bool ewkino::passPtCuts( const Event& event ){
    
    //assume leptons were ordered while applying baseline selection
    
    //leading lepton
    if( event.lepton( 0 ).isMuon() && event.lepton( 0 ).pt() <= 20 ) return false;
    if( event.lepton( 0 ).isElectron() && event.lepton( 0 ).pt() <= 25 ) return false;

    //subleading lepton
    if( event.lepton( 1 ).isMuon() && event.lepton( 1 ).pt() <= 10 ) return false;
    if( event.lepton( 1 ).isElectron() && event.lepton( 1 ).pt() <= 15 ) return false;
    return true;
}


bool ewkino::leptonsArePrompt( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && !leptonPtr->isPrompt() ) return false;
    }
    return true;
}


bool ewkino::leptonsAreTight( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && !leptonPtr->isTight() ) return false;
    }
    return true;
}


bool leptonFromMEExternalConversion( const Lepton& lepton ){
    if( !( lepton.matchPdgId() == 22 ) ) return false;
    if( !( lepton.isPrompt() && lepton.provenanceConversion() == 0 ) ) return false;
    return true;
}


bool ewkino::passPhotonOverlapRemoval( const Event& event ){
    bool isPhotonSample = false;
    bool isInclusiveSample = false;
    std::string sampleName = event.sample().fileName();
    if( stringTools::stringContains( sampleName, "DYJetsToLL" ) || stringTools::stringContains( sampleName, "TTTo" ) || stringTools::stringContains( sampleName, "TTJets" ) ){
        isInclusiveSample = true;
    } else if( stringTools::stringContains( sampleName, "TTGamma" ) || stringTools::stringContains( sampleName, "ZGToLLG" ) || stringTools::stringContains( sampleName, "WGToLNuG" ) ){
        isPhotonSample = true;
    }

    if( !( isPhotonSample || isInclusiveSample ) ){
        return true;
    }

    bool hasMEExternalConversion = false;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && leptonFromMEExternalConversion( *leptonPtr ) ){
            hasMEExternalConversion = true;
            break;
        }
    }
    if( isInclusiveSample ){
        return !hasMEExternalConversion;
    } else if( isPhotonSample ){
        return hasMEExternalConversion;
    }
    return true;
}


double ewkino::fakeRateWeight( const Event& event, const std::shared_ptr< TH2 >& muonMap, const std::shared_ptr< TH2 >& electronMap ){
    static constexpr double maxPt = 44.;

    double weight = -1.;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( !leptonPtr->isFO() ) continue;
        if( leptonPtr->isTight() ) continue;
        double fr;
        double pt = std::min( leptonPtr->pt(), maxPt );
        if( leptonPtr->isMuon() ){
            fr = histogram::contentAtValues( muonMap.get(), pt, leptonPtr->absEta() );
        } else if( leptonPtr->isElectron() ){
            fr = histogram::contentAtValues( electronMap.get(), pt, leptonPtr->absEta() );
        } else {
            throw std::invalid_argument( "we are not considering taus for now" );
        }
        weight *= - fr / ( 1. - fr );
    }
    return weight;
}
