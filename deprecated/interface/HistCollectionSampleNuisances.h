/*
Full histogram collection for a particular sample, including nuisances
*/

#ifndef HistCollectionSampleNuisances_H
#define HistCollectionSampleNuisances_H

//include other parts of code 
#include "HistCollectionSample.h"
#include "HistCollectionBaseNuisances.h"

//include c++ library classes
#include <map>

//include ROOT classes

class HistCollectionSampleNuisances : public HistCollectionSample {
    public:
    	HistCollectionSampleNuisances(const std::vector< std::shared_ptr< HistInfo > >&, const std::shared_ptr< Sample >&, const std::shared_ptr< Category >&, 
			const std::vector< std::string >& nuisanceNames, const bool includeSB = false);
    	HistCollectionSampleNuisances(const std::vector< HistInfo>&, const Sample&, const Category&, const std::vector< std::string>& nuisanceNames, const bool includeSB = false);
    	HistCollectionSampleNuisances(const std::vector< HistInfo>&, const Sample&, const std::vector< std::vector < std::string > >& categoryVec, 
			const std::vector < std::string>& nuisanceNames, const bool includeSB = false);

		std::shared_ptr< TH1D > accessNuisanceDown(const std::string&, const size_t, const std::vector<size_t>&, const bool sb = false);
		std::shared_ptr< TH1D > accessNuisanceUp(const std::string&, const size_t, const std::vector<size_t>&, const bool sb = false);
    private: 
        std::vector < HistCollectionBaseNuisances > nuisanceCollection;
		std::map< std::string, size_t> nuisanceIndexMap; 
};



#endif 
