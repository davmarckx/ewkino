#ifndef ewkinoSelection_H
#define ewkinoSelection_H

//include ROOT classes
#include "TH2.h"

//include other parts of framework
#include "../../Event/interface/Event.h"

namespace ewkino{
    void applyBaselineObjectSelection( Event& event, const bool allowUncertainties = false );
    bool passLowMllVeto( const Event& event, const double vetoValue = 12. );
    bool passBaselineSelection( Event& event, const bool allowUncertainties = false, const bool bVeto = true, const bool mllVeto = true );
    JetCollection variedJetCollection( const Event& event, const std::string& uncertainty );
    JetCollection::size_type numberOfVariedBJets( const Event& event, const std::string& uncertainty );
    Met variedMet( const Event& event, const std::string& uncertainty );
    bool passVariedSelection( const Event& event, const std::string& uncertainty );
    bool passVariedSelectionWZCR( Event& event, const std::string& uncertainty );
    bool passVariedSelectionTTZCR( Event& event, const std::string& uncertainty );
    bool passVariedSelectionNPCR( Event& event, const std::string& uncertainty );
    bool passVariedSelectionXGammaCR( Event& event, const std::string& uncertainty );
    bool passTriggerSelection( const Event& event );
    bool passPtCuts( const Event& event );
    bool leptonsArePrompt( const Event& event );
    bool leptonsAreTight( const Event& event );
    double fakeRateWeight( const Event& event, const std::shared_ptr< TH2 >& muonMap, const std::shared_ptr< TH2 >& electronMap );
    bool passPhotonOverlapRemoval( const Event& event );
}

#endif
