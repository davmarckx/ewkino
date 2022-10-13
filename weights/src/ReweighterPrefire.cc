#include "../interface/ReweighterPrefire.h"

// weight functions for ReweighterPrefire

double ReweighterPrefire::weight( const Event& event ) const{
    return event.generatorInfo().prefireWeight();
}

double ReweighterPrefire::weightDown( const Event& event ) const{
    return event.generatorInfo().prefireWeightDown();
}

double ReweighterPrefire::weightUp( const Event& event ) const{
    return event.generatorInfo().prefireWeightUp();
}


// weight functions for ReweighterPrefireMuon

double ReweighterPrefireMuon::weight( const Event& event ) const{
    return event.generatorInfo().prefireWeightMuon();
}

double ReweighterPrefireMuon::weightDown( const Event& event ) const{
    return event.generatorInfo().prefireWeightMuonDown();
}

double ReweighterPrefireMuon::weightUp( const Event& event ) const{
    return event.generatorInfo().prefireWeightMuonUp();
}


// weight functions for ReweighterPrefireECAL

double ReweighterPrefireECAL::weight( const Event& event ) const{
    return event.generatorInfo().prefireWeightECAL();
}

double ReweighterPrefireECAL::weightDown( const Event& event ) const{
    return event.generatorInfo().prefireWeightECALDown();
}

double ReweighterPrefireECAL::weightUp( const Event& event ) const{
    return event.generatorInfo().prefireWeightECALUp();
}
