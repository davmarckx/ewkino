#ifndef eventFlattening_H
#define eventFlattening_H

// include ROOT classes
#include "TMVA/Reader.h"
#include "TMVA/BDT.h"
#include "TMVA/RBDT.hxx"
// may be needed #include <xgboost/c_api.h>

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"
#include "../../../Tools/interface/readFakeRateTools.h"
#include "../../../Tools/interface/readChargeFlipTools.h"

// include analysis tools
#include "eventSelections.h"

// function declarations

namespace eventFlattening{
    void setVariables(std::map<std::string,double>);
    std::map< std::string, double > initVarMap();
    void initOutputTree(TTree*);
    std::shared_ptr<TMVA::Reader> initReader(const std::string&);
    std::shared_ptr< TH2D > readFRMap( const std::string&, const std::string&, const std::string&);
    double fakeRateWeight( const Event&, 
    			const std::shared_ptr< TH2D >&, const std::shared_ptr< TH2D >&);
    int fakeRateFlavour( const Event& );
    std::map< std::string, double > eventToEntry(Event& event,
				const CombinedReweighter& reweighter,
				const std::string& selection_type, 
				const std::shared_ptr< TH2D>& frMap_muon = nullptr, 
				const std::shared_ptr< TH2D>& frMap_electron = nullptr,
                                const std::shared_ptr< TH2D>& cfMap_electron = nullptr,
				const std::string& variation = "nominal",
                                const std::shared_ptr<TMVA::Experimental::RBDT<>>& bdt = nullptr,
                                const std::string& bdtYear = "2018"); 
				// note: 2018 returns 1, which is also good for single year bdts, 
				// even though this feature is filtered out in these bdts
    std::pair<double,double> pmzcandidates(Lepton&, Met&);
    std::pair<double,int> besttopcandidate(JetCollection&, Lepton&, Met&, double, double);
}

#endif
