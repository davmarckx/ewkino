/*
Cross section variations for a sample
*/

// Note: use this one for NanoGen/NanoAOD files;
// for custom ntuples, use SampleCrossSections instead.
// At some point, might consider merging them into a single class,
// but for now it seems easier to keep them separate.


#ifndef NanoGenSampleCrossSections_H
#define NanoGenSampleCrossSections_H

// include other parts of the framework
#include "Sample.h"
#include "../../TreeReader/interface/NanoGenTreeReader.h"


class NanoGenSampleCrossSections{

	public:
	    using size_type = std::vector< double >::size_type;

	    NanoGenSampleCrossSections() = default;
	    NanoGenSampleCrossSections( const NanoGenTreeReader& );

	    // get counters
	    size_type numberOfScaleVariations() const{ return scaleCrossSectionRatios.size(); }
	    size_type numberOfPdfVariations() const{ return pdfCrossSectionRatios.size(); }
	    size_type numberOfPsVariations() const{ return psCrossSectionRatios.size(); }	    

	    // get a pdf variation
	    double crossSectionRatio_pdfVar( const size_type ) const;

	    // get a scale variation.
	    // for the number and order of the weights,
	    // see https://cms-nanoaod-integration.web.cern.ch/autoDoc/
	    // note: keep in sync with Event/src/GeneratorInfo.cc!
	    double crossSectionRatio_MuR_1_MuF_2() const{ return crossSectionRatio_scaleVar(5); }
	    double crossSectionRatio_MuR_1_MuF_0p5() const{ return crossSectionRatio_scaleVar(3); }
	    double crossSectionRatio_MuR_2_MuF_1() const{ return crossSectionRatio_scaleVar(7); }
	    double crossSectionRatio_MuR_2_MuF_2() const{ return crossSectionRatio_scaleVar(8); }
	    double crossSectionRatio_MuR_2_MuF_0p5() const{ return crossSectionRatio_scaleVar(6); }
	    double crossSectionRatio_MuR_0p5_MuF_1() const{ return crossSectionRatio_scaleVar(1); }
	    double crossSectionRatio_MuR_0p5_MuF_2() const{ return crossSectionRatio_scaleVar(2); }
	    double crossSectionRatio_MuR_0p5_MuF_0p5() const{ return crossSectionRatio_scaleVar(0); }

	    // get a PS variation.
	    // for the number and order of the weights,
            // see https://cms-nanoaod-integration.web.cern.ch/autoDoc/
            // note: keep in sync with Event/src/GeneratorInfo.cc!
	    double crossSectionRatio_ISR_0p5() const{ return crossSectionRatio_psVar(2); }
	    double crossSectionRatio_FSR_0p5() const{ return crossSectionRatio_psVar(3); }
	    double crossSectionRatio_ISR_2() const{ return crossSectionRatio_psVar(0); }
	    double crossSectionRatio_FSR_2() const{ return crossSectionRatio_psVar(1); }

	private:
	    std::vector< double > scaleCrossSectionRatios;
	    std::vector< double > pdfCrossSectionRatios;
	    std::vector< double > psCrossSectionRatios;

	    double crossSectionRatio_scaleVar(const size_type) const;
	    double crossSectionRatio_psVar(const size_type) const;
};


#endif
