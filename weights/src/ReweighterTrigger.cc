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

std::shared_ptr<TH2> getScaleFactorHistogram(
    const std::string& triggerWeightPath,
    const std::string& year,
    const std::string& eventClass ){
    // helper function to retrieve the trigger efficiency SF histogram from a root file
    // arguments:
    // - triggerWeightPath: path to root file containing histograms with scalefactors
    // - year: data-taking year
    // - eventClass: choose from ee, em, me or mm
    // note: the histograms are assumed to be named "scalefactors_<year>_<eventClass>"
    TFile* triggerWeightFile = TFile::Open( (triggerWeightPath).c_str() );
    std::string histPath = std::string("scalefactors_" + year  + "_" + eventClass);
    std::shared_ptr< TH2 > SFs(dynamic_cast< TH2* >(triggerWeightFile->Get(histPath.c_str()) ) );
    if( SFs == nullptr ){
	std::string msg = "ERROR in ReweighterTrigger.getScaleFactorHistogram:";
	msg += " file " + triggerWeightPath + " does not contain expected histogram";
        msg += " " + histPath;
        throw std::runtime_error(msg);
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

ReweighterTrigger::ReweighterTrigger(
    const std::string& triggerWeightPath, 
    const std::string& year,
    const double uncertainty ) {
    // input arguments:
    // - triggerWeightPath: path to a root file containing histograms with reweighting factors
    //   (currently ../weightFilesUL/triggerSF/scalefactors_allYears.root)
    // - year: data taking year
    // - uncerainty: double with systematic uncertainty (e.g. 0.02 for 2%)
    // notes:
    // - the histograms with scale factors have leading lepton pt on x-axis
    //   and subleading lepton pt on y-axis; events with more (or less) than 2 leptons
    //   are not reweighted.
    // - the fixed uncertainty is only used for events with more (or less) than 2 leptons;
    //   for events with 2 leptons the bin error from the histogram is used.

    triggerWeights_ee = getScaleFactorHistogram(triggerWeightPath, year, "ee");
    triggerWeights_ee->SetDirectory( gROOT );

    triggerWeights_mm = getScaleFactorHistogram(triggerWeightPath, year, "mm");
    triggerWeights_mm->SetDirectory( gROOT );

    triggerWeights_em = getScaleFactorHistogram(triggerWeightPath, year, "em");
    triggerWeights_em->SetDirectory( gROOT );

    triggerWeights_me = getScaleFactorHistogram(triggerWeightPath, year, "me");
    triggerWeights_me->SetDirectory( gROOT );

    systUnc = uncertainty;
}


// ---------------------------
// weight retrieval functions 
// ---------------------------

double ReweighterTrigger::weight( 
	const Event& event, 
	const std::shared_ptr< TH2 >& weightHist) const {
    // retrieve weight for a 2L event
    // input arguments:
    // - event: event for which to retrieve the weight
    // - weightHist: histogram containing the weights
    return histogram::contentAtValues( weightHist.get(),
	event.leptonCollection()[0].pt(),
	event.leptonCollection()[1].pt() );
}

double ReweighterTrigger::uncertainty(
	const Event& event,
	const std::shared_ptr< TH2 >& weightHist) const{
    // same as above but return bin error rahter than content
    return histogram::uncertaintyAtValues( weightHist.get(),
        event.leptonCollection()[0].pt(),
        event.leptonCollection()[1].pt() );
}

std::pair<double, double> ReweighterTrigger::weightAndUncertainty( const Event& event ) const{
    // retrieve weight and uncertainty for a 2L event
    double weightval = 1.;
    double uncertaintyval = 0.;
    bool flavor1 = event.leptonCollection()[0].isMuon(); // 0 if electron, 1 if muon
    bool flavor2 = event.leptonCollection()[1].isMuon(); // 0 if electron, 1 if muon
    if(!flavor1 && !flavor2){
	weightval = weight( event,  triggerWeights_ee );
	uncertaintyval = uncertainty( event, triggerWeights_ee );
    }
    else if(flavor1 && flavor2){
	weightval = weight( event,  triggerWeights_mm );
	uncertaintyval = uncertainty( event, triggerWeights_mm );
    }
    else if(flavor1 && !flavor2){
	weightval = weight( event,  triggerWeights_me );
	uncertaintyval = uncertainty( event, triggerWeights_me );
    }
    else{
	weightval = weight( event,  triggerWeights_em );
	uncertaintyval = uncertainty( event, triggerWeights_em );
    }
    return std::make_pair(weightval, uncertaintyval);
}

double ReweighterTrigger::weight( const Event& event ) const{
    // retrieve weight for an event
    // note: only events with exactly 2 tight leptons get a reweighting factor!
    // this is ok as the trigger efficiency for 3- and 4-lepton selections
    // is measured to be compatible with 100%.
    event.sortLeptonsByPt();
    if(!hasnTightLeptons(event, 2)){ return 1; }
    return weightAndUncertainty(event).first;
}

double ReweighterTrigger::weightDown( const Event& event ) const{
    // retrieve down-varied weight for and event
    if(!hasnTightLeptons(event, 2)){ return 1-systUnc; }
    std::pair<double, double> weights = weightAndUncertainty(event);
    return weights.first-weights.second;
}

double ReweighterTrigger::weightUp( const Event& event ) const{
    // retrieve up-varied weight for an event
    if(!hasnTightLeptons(event, 2)){ return 1+systUnc; }
    std::pair<double, double> weights = weightAndUncertainty(event);
    return weights.first+weights.second;
}
