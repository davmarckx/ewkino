// include header
#include "../interface/triggerTools.h"

std::vector<double> triggerTools::ptThresholds( 
    const std::string& id,
    const std::string& channel ){
    // return a vector of lepton pt thresholds
    if( id=="tzq" ){ return {25., 15., 10.}; }
    if( id=="ttz" ){ return {40., 20., 10.}; }
    if( id=="ttwdilep" ){
        if( channel=="mm"){ return {25., 20.}; }
        if( channel=="me"){ return {25., 20.}; }
        if( channel=="em"){ return {30., 20.}; }
        if( channel=="ee"){ return {30., 20.}; }
    }
    if( id=="ttwtrilep"){ return {25., 15., 15.}; }
    if( id=="ttttfourlep" ){ return {25., 15., 15., 10.}; } // (?)
    std::string msg = "ERROR in ptThresholds: id or channel not recognized";
    msg.append( " (id: "+id+", channel: "+channel+")." );
    throw std::runtime_error(msg);
}

bool triggerTools::passPtThresholds( 
    const std::vector<double>& pts, 
    const std::vector<double>& thresholds ){
    // determine whether a given vector of pts passes a given vector of thresholds.
    // note that both are implicitly assumed to be sorted (in the same way)!
    unsigned ptssize = pts.size();
    unsigned thresholdssize = thresholds.size();
    if( ptssize!=thresholdssize ){
	std::string msg = "ERROR in passPtThresholds: ";
	msg.append( " vectors of pTs and corresponding pT thresholds" );
	msg.append( " have different lengths." );
	throw std::runtime_error( msg );
    }
    for( unsigned i=0; i<ptssize; i++ ){
	if( pts[i]<thresholds[i] ){ return false; }
    }
    return true;
}

std::string triggerTools::getFlavourString( const Event& event ){
    // return a lepton flavor string of the form "eem".
    // note: event has to be cleaned and leptons sorted
    //       before calling this function!
    std::string flavourString = "";
    for( auto lepton: event.leptonCollection() ){
	if( lepton->isElectron() ) flavourString.append("e");
	else if( lepton->isMuon() ) flavourString.append("m");
	else if( lepton->isTau() ) flavourString.append("t");
    }
    return flavourString;
}

bool triggerTools::passTriggersRef( const Event& event ){
    // check whether an event passes reference triggers.
    // should be equivalent to event.passTriggers_ref(),
    // but the underlying ntuplizer variable is not in current skimmed samples...
    // see https://github.com/GhentAnalysis/heavyNeutrino/blob/
    //     UL_master/multilep/src/TriggerAnalyzer.cc
    std::vector<std::string> triggerNames;
    if( event.is2018() ){
	triggerNames = { "HLT_CaloMET350_HBHECleaned", 
			 "HLT_CaloJet500_NoJetID", 
			 "HLT_AK8PFJet500", 
			 "HLT_AK8PFJet400_TrimMass30",
			 "HLT_DiJet110_35_Mjj650_PFMET110", 
			 "HLT_PFHT800_PFMET75_PFMHT75_IDTight", 
			 "HLT_PFHT700_PFMET85_PFMHT85_IDTight",
                         "HLT_PFHT500_PFMET100_PFMHT100_IDTight", 
			 "HLT_PFHT1050", 
			 "HLT_PFJet500", 
			 "HLT_PFMET120_PFMHT120_IDTight", 
			 "HLT_PFMET250_HBHECleaned", 
			 "HLT_PFMET200_HBHE_BeamHaloCleaned", 
			 "HLT_PFMETTypeOne140_PFMHT140_IDTight",
                         "HLT_PFMETTypeOne200_HBHE_BeamHaloCleaned", 
			 "HLT_TripleJet110_35_35_Mjj650_PFMET110" };
    } else if( event.is2017() ){
	triggerNames = { "HLT_PFJet500", 
			 "HLT_PFMET140_PFMHT140_IDTight", 
			 "HLT_PFHT500_PFMET100_PFMHT100_IDTight",
                         "HLT_PFHT700_PFMET85_PFMHT85_IDTight", 
			 "HLT_PFHT800_PFMET75_PFMHT75_IDTight", 
			 "HLT_CaloJet500_NoJetID",
                         "HLT_AK8PFJet500" };
    } else{
	triggerNames = { "HLT_MET200", 
			 "HLT_PFMET300", 
			 "HLT_PFMET170_HBHECleaned", 
			 "HLT_PFMET120_PFMHT120_IDTight",
                         "HLT_PFHT300_PFMET110", 
			 "HLT_PFHT350_DiPFJetAve90_PFAlphaT0p53", 
			 "HLT_PFHT400_DiPFJetAve90_PFAlphaT0p52",
                         "HLT_PFHT400_SixJet30_DoubleBTagCSV_p056", 
			 "HLT_PFHT900", 
			 "HLT_PFHT650_WideJetMJJ900DEtaJJ1p5", 
			 "HLT_CaloJet500_NoJetID" };
    }
    for( std::string triggerName: triggerNames ){
	if( event.passTrigger(triggerName) ) return true;
    }
    return false;
}
