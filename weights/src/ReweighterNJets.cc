/*
Reweighter that has no nominal change,
but that varies the event weight up and down by a fixed amount
if the number of jets or b-jets is in a given range.
*/

#include "../interface/ReweighterNJets.h"


// constructor

ReweighterNJets::ReweighterNJets( const std::map<unsigned int, double>& nJetUncertainties,
                                  bool doBJets ){
    // input arguments:
    // - nJetUncertainties: map of number of jets to relative uncertainty to add.
    // - doBJets: use number of medium b-tagged jets instead of number of jets.
    // note: an uncertainty of e.g. 30% should be entered as 0.3.
    // note: the maps are assumed to be sorted by number of (b-tagged) jets.
    // note: if a given number of jets is not in the map,
    //       the previous element is taken, e.g. {2:0.1} will apply a 10% uncertainty
    //       on events with 2 or more jets and no uncertainty for events with 0 or 1 jets.
    
    // copy the doBJets argument
    _doBJets = doBJets;
    // fill the internal njets map with all values (also the ones missing in the argument) 
    // for faster accessing
    _nMax = 15;
    unsigned int firstNJets = nJetUncertainties.begin()->first;
    double currentUnc = nJetUncertainties.begin()->second;
    if( firstNJets>0 ){ currentUnc = 0; }
    for(unsigned int i=0; i<_nMax+1; i++){
	if( nJetUncertainties.find(i)!=nJetUncertainties.end() ){
	    currentUnc = nJetUncertainties.at(i);
	}
	_nJetUncertainties[i] = currentUnc;
    }
}

// help functions

double ReweighterNJets::getUnc( const Event& event ) const{
    // determine number of jets
    unsigned int njets = 0;
    if( !_doBJets ){ njets = event.jetCollection().goodJetCollection().size(); }
    else{ njets = event.jetCollection().goodJetCollection().numberOfMediumBTaggedJets(); }
    // get relative uncertainty
    double unc = 0;
    if( njets>_nMax ){ unc = _nJetUncertainties.end()->second; }
    else{ unc = _nJetUncertainties.at(njets); }
    // return result
    return unc;
}

// weight functions

double ReweighterNJets::weight( const Event& event ) const{
    // dummy condition on event to avoid unused parameter warning
    if( event.isData() ){ return 1; }
    return 1;
}

double ReweighterNJets::weightDown( const Event& event ) const{
    return 1 - getUnc(event);
}

double ReweighterNJets::weightUp( const Event& event ) const{
    return 1 + getUnc(event);
}
