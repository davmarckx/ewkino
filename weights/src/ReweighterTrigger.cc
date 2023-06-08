#include "../interface/ReweighterTrigger.h"

//include c++ library classes
#include <vector>

//include ROOT classes
#include "TFile.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/histogramTools.h"


// -----------------
// helper functions 
// -----------------

std::shared_ptr<TH2> getMCTriggerHistogram( const std::string& triggerWeightPath, const std::string& year , const std::string& eventClass ){
    // helper function to retrieve the trigger efficiency SF histogram from a root file
    //std::shared_ptr< TFile > filePtr = triggerWeightPath.filePtr();
    TFile* triggerWeightFile = TFile::Open( (triggerWeightPath).c_str() );

    std::string histPath = std::string("scalefactors_" + year  + "_" + eventClass);
    std::shared_ptr< TH2 > SFs(dynamic_cast< TH2* >(triggerWeightFile->Get(histPath.c_str()) ) );
    if( SFs == nullptr ){
        throw std::runtime_error( std::string("ERROR in ReweighterTrigger.getMCTriggerHistogram:") + std::string("does not contain expected histogram ") + std::string(histPath));
    }
    return SFs;
}


bool hasnTightLeptons(const Event& event, int n){
    int nTight = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isTight() ){ ++nTight; }
    }
    if( n!=nTight ){ return false; }
    return true;
}

std::vector<double> leptonPts(const Event& event){
    std::vector<double> Pts;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isTight() ){ Pts.push_back(leptonPtr->pt()); }
    }
    return Pts;
}

// ------------
// constructor
// ------------
// note: new version, suitable for UL analyses.

ReweighterTrigger::ReweighterTrigger(
    const std::string& triggerWeightPath, 
    const std::string& year,
    const double uncertainty ) {
    // input arguments:
    // - triggerWeightPath: path to a root file containing histograms with reweighting factors
    //   (currently ../weightFilesUL/triggerSF/scalefactors_allYears.root)
    // - year: data taking year
    // - uncerainty: double with systematic uncertainty (e.g. 0.02 for 2%)

    triggerWeights_ee = getMCTriggerHistogram(triggerWeightPath, year, "ee");
    triggerWeights_ee->SetDirectory( gROOT );

    triggerWeights_mm = getMCTriggerHistogram(triggerWeightPath, year, "mm");
    triggerWeights_mm->SetDirectory( gROOT );

    triggerWeights_em = getMCTriggerHistogram(triggerWeightPath, year, "em");
    triggerWeights_em->SetDirectory( gROOT );

    triggerWeights_me = getMCTriggerHistogram(triggerWeightPath, year, "me");
    triggerWeights_me->SetDirectory( gROOT );

    systUnc = uncertainty;

    isUL = true;
}


// ---------------------------
// weight retrieval functions 
// ---------------------------
/*
double ReweighterTrigger::weight( 
	const Event& event, 
	const std::map< std::string, std::shared_ptr< TH2 > >& weightMap ) const{
    // retrieve weight for an event
    // input arguments:
    // - event: event for which to retrieve the weight
    // - weightMap: map of sample names to weight histograms
    auto it = weightMap.find( event.sample().uniqueName() );
    if( it == weightMap.cend() ){
        throw std::invalid_argument( std::string("ERROR in ReweighterPileup.weight:")
	    +" no pileup weights for sample " + event.sample().uniqueName() 
	    +" found, this sample was probably not present"
	    +" in the vector used to construct the Reweighter." );
    }
    return weight( event, it->second );
}
*/

double ReweighterTrigger::weight( 
	const Event& event, 
	const std::shared_ptr< TH2 >& weightHist) const {
    // retrieve weight for an event
    // input arguments:
    // - event: event for which to retrieve the weight
    // - weightHist: histogram containing the weights
    return histogram::contentAtValues( weightHist.get(),
	event.leptonCollection()[0].pt(),
	event.leptonCollection()[1].pt() );
}

double ReweighterTrigger::weight( const Event& event ) const{
    // retrieve weight for an event, for safety apply tight criteria again
    // note: only events with exactly 2 tight leptons get a reweighting factor!
    // this is ok as the trigger efficiency for 3- and 4-lepton selections
    // is measured to be compatible with 100%.
    if(!hasnTightLeptons(event, 2)){ return 1; }
    event.sortLeptonsByPt();
    bool flavor1 = event.leptonCollection()[0].isMuon();     // 0 if electron, 1 if muon
    bool flavor2 = event.leptonCollection()[1].isMuon();     // 0 if electron, 1 if muon
    if(!flavor1 && !flavor2){return weight( event,  triggerWeights_ee);}
    if(flavor1 && flavor2){return weight( event,  triggerWeights_mm);}
    if(flavor1 && !flavor2){return weight( event,  triggerWeights_me);}
    return weight( event,  triggerWeights_em);
    
}

double ReweighterTrigger::weightDown( const Event& event ) const{
    // retrieve down-varied weight for and event
    return weight(event)*(1-systUnc);
}

double ReweighterTrigger::weightUp( const Event& event ) const{
    // retrieve up-varied weight for an event
    return weight(event)*(1+systUnc);
}
