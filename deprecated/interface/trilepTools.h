#ifndef trilepTools_H
#define trilepTools_H
//include c++ library utilities
#include <utility>
//include ROOT utilities
#include "TLorentzVector.h"
namespace trilep{
    std::pair<unsigned, unsigned> bestZ(const TLorentzVector*, const std::vector<unsigned>&, const unsigned*, const int*, const unsigned); //return indices of leptons forming best Z-candidate
    unsigned flavorChargeComb(const std::vector<unsigned>& ind, const unsigned*, const int*, const unsigned); //Determine whether event contains OSSF or OSOF pair, or not 
    std::pair<double, double> neutrinoPZ(const TLorentzVector& wLep, const TLorentzVector& met);              //solve the neutrino Z momentum from the W mass constraint
    unsigned flavorComposition(const std::vector<unsigned>& ind, const unsigned*, const unsigned);
}
#endif
