#ifndef EFTCrossSections_H
#define EFTCrossSections_H

/* 
Note: so far only nominal cross-section has been implemented.
To correctly normalize the EFT samples, we need the sum of nominal weights;
the sums of weights of other variations are also stored in the samples but not yet used.
*/


#include "Sample.h"


class EFTCrossSections{

	public:
	    using size_type = std::vector< double >::size_type;

	    EFTCrossSections() = default;
	    EFTCrossSections( const Sample& );

	    // get number of EFT variations
	    size_type numberOfEFTVariations() const{ return eftCrossSectionRatios.size(); }
	    
	    // get an EFT variation
	    double nominalSumOfWeights() const;
	    double EFTSumOfWeights( const size_type ) const;
	    double EFTCrossSectionRatio( const size_type ) const;

	private:
	    std::vector< double > eftSumOfWeights;
	    std::vector< double > eftCrossSectionRatios;
};


#endif
