#include "../interface/ewkinoVariables.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework
#include "../interface/ewkinoSelection.h"


std::map< std::string, double > ewkino::computeVariables( Event& event, const std::string& unc ){
    Met variedMet = ewkino::variedMet( event, unc );
    JetCollection variedJetCollection = ewkino::variedJetCollection( event, unc );
    PhysicsObject leptonSum = event.leptonCollection().objectSum();
    double mll, mtW;
    try{
        mll = event.bestZBosonCandidateMass();
        mtW = mt( event.WLepton(), variedMet ); 
    } catch( std::domain_error& ){
        mll = ( event.lepton( 0 ) + event.lepton( 1 ) ).mass();
        mtW = mt( event.lepton( 2 ), variedMet );
    }
    std::map< std::string, double > ret = {
        { "met", variedMet.pt() },
        { "mll", mll },
        { "mtW", mtW },
        { "ltmet", event.LT() + variedMet.pt() },
        { "m3l", leptonSum.mass() },
        { "mt3l", mt( leptonSum, variedMet ) },
        { "ht", variedJetCollection.scalarPtSum()},
        { "numberOfJets", variedJetCollection.size() },
        { "numberOfBJets", variedJetCollection.numberOfTightBTaggedJets() }
    };
    return ret;
}
