#ifndef EFTInfo_H
#define EFTInfo_H

// include c++ modules
#include <map>
#include <vector>
#include <algorithm>

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"

class EFTInfo{
    
    public:
        EFTInfo( const TreeReader& );

        // basic getters
        unsigned int numberOfEFTCoefficients() const{ return _numberOfEFTCoefficients; }

	// calculate weight for a given configuration of wilson coefficients
	double nominalWeight() const;
	double weight(const std::map<std::string, double>& wcValueMap) const;
	double relativeWeight(const std::map<std::string, double>& wcValueMap) const;

	// get correct indices for wilson coefficient combinations
	void checkWCName(const std::string& WCName) const;
	unsigned int getIndex(const std::string& WCName) const;
	unsigned int getIndex(const std::string& WCName1, const std::string& WCName2) const;
	        
    private:
        static constexpr unsigned int maxNumberOfEFTCoefficients = 276;
	// (note: keep this number in sync with TreeReader and ntuplizer!)
        unsigned int _numberOfEFTCoefficients;
        double _EFTCoefficients[maxNumberOfEFTCoefficients];

	// define order of the wilson coefficients / operators
	// note: ideally this information should be in the samples,
	// but for now this is not the case so need to hard-code it here;
	// also check the getIndex functions in the source file.
	std::vector<std::string> _WCNames = {
	    "ctlTi",  "ctq1",  "ctq8",  "cQq83",  "cQq81",
	    "cQlMi",  "cbW",  "cpQ3",  "ctei",  "cQei",
	    "ctW",  "cpQM",  "ctlSi",  "ctZ",  "cQl3i",
	    "ctG",  "cQq13",  "cQq11",  "cptb",  "ctli",
	    "ctp",  "cpt"
	};
};

#endif 
