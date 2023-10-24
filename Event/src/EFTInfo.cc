#include "../interface/EFTInfo.h"

// include c++ library classes 
#include <stdexcept>


// constructor

EFTInfo::EFTInfo( const TreeReader& treeReader ) :
    _numberOfEFTCoefficients( treeReader._nEFTCoefficients )
{
    // check if sample contains the correct number of EFT coefficients
    if( _numberOfEFTCoefficients > maxNumberOfEFTCoefficients ){
        std::string message = "WARNING in EFTInfo::EFTInfo:";
        message.append( " _numberOfEFTCoefficients is " + std::to_string(_numberOfEFTCoefficients) );
        message.append( " which is larger than " + std::to_string(maxNumberOfEFTCoefficients) );
        message.append( " (will ignore overflowing entries)." );
        std::cerr << message << std::endl;
	_numberOfEFTCoefficients = maxNumberOfEFTCoefficients;
    }
    if( _numberOfEFTCoefficients < maxNumberOfEFTCoefficients ){
        std::string message = "WARNING in EFTInfo::EFTInfo:";
        message.append( " _numberOfEFTCoefficients is " + std::to_string(_numberOfEFTCoefficients) );
        message.append( " which is smaller than " + std::to_string(maxNumberOfEFTCoefficients) );
        message.append( " (might lead to unphysical weights)." );
        std::cerr << message << std::endl;
    }
    for( unsigned int i = 0; i < _numberOfEFTCoefficients; ++i ){
        _EFTCoefficients[i] = treeReader._EFTCoefficients[i];
    }
}


// get weight

double EFTInfo::nominalWeight() const{
    return _EFTCoefficients[0];
}

double EFTInfo::weight(const std::map<std::string, double>& WCValueMap) const{
    // calculate event weight based on a given configuration of wilson coefficients
    // input arguments:
    // - WCValueMap: maps wilson coefficient names to their values,
    //               e.g. {{"ctlTi",  0.5}, {"ctq1", 2.8}, ...}
    //               (coefficients not in the map are assumed to be zero).
    
    // initialize result
    double weight = 0;
    // put map keys in a vector
    std::vector<std::string> wcnames;
    for( auto el: WCValueMap ){ wcnames.push_back(el.first); }
    unsigned int nwcnames = wcnames.size();
    // iterate over provided wilson coefficients
    for( unsigned int i=0; i<nwcnames; i++){
	std::string wcname_i = wcnames[i];
	// linear term
	double linear_weight = WCValueMap.at(wcname_i) * _EFTCoefficients[getIndex(wcname_i)];
	weight += linear_weight;
	// iterate over provided wilson coefficients in second order
	/*for( unsigned int j=i; j<nwcnames; j++){
	    std::string wcname_j = wcnames[j];
	    // quadratic term
	    double quadratic_weight = WCValueMap.at(wcname_i) * WCValueMap.at(wcname_j);
	    quadratic_weight *= _EFTCoefficients[getIndex(wcname_i, wcname_j)];
	    weight += quadratic_weight;
	}*/
    }
    // return result
    return weight; 
}

double EFTInfo::relativeWeight(const std::map<std::string, double>& WCValueMap) const{
    // same as weight() but divide by nominal weight
    double absoluteWeight = weight(WCValueMap);
    double relativeWeight = absoluteWeight / nominalWeight();
    return relativeWeight;
}


// internal helper functions to get correct index
// note: this depends on the assumed order in the EFTCoefficients array in the ntuples.
//       for now: first nominal (SM), then linear terms (in the order defined by _WCNames),
//       then quadratic terms (not fully clear yet in what order)

void EFTInfo::checkWCName(const std::string& WCName) const{
    if( std::find(_WCNames.begin(), _WCNames.end(), WCName) == _WCNames.end() ){
	std::string msg = "ERROR: in EFTInfo::checkWCName:";
	msg += " wilson coefficient " + WCName + " not found.";
	throw std::runtime_error(msg);
    }
}

unsigned int EFTInfo::getIndex(const std::string& WCName) const{
    // get linear index for a given wilson coefficient name
    checkWCName(WCName);
    unsigned int idx = std::find(_WCNames.begin(), _WCNames.end(), WCName) - _WCNames.begin();
    return 1 + idx;
}

unsigned int EFTInfo::getIndex(const std::string& WCName1, const std::string& WCName2) const{
    // get quadratic index for a given combination of wilson coefficient names
    checkWCName(WCName1);
    checkWCName(WCName2);
    unsigned int idx1 = std::find(_WCNames.begin(), _WCNames.end(), WCName1) - _WCNames.begin();
    unsigned int idx2 = std::find(_WCNames.begin(), _WCNames.end(), WCName2) - _WCNames.begin();
    unsigned int nWCs = _WCNames.size();
    if( idx1 > idx2 ){ unsigned int temp = idx2; idx2 = idx1; idx1 = temp; }
    return 1 + nWCs + unsigned((idx2 + 1)*idx2/2) + idx1;
}
