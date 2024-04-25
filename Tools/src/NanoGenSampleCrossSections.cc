#include "../interface/NanoGenSampleCrossSections.h"

//include c++ library classes
#include <stdexcept>
#include <string>

//include ROOT classes
#include "TH1.h"
#include "TROOT.h"

NanoGenSampleCrossSections::NanoGenSampleCrossSections( const NanoGenTreeReader& treeReader ){

    // fill scale cross section ratios
    for( unsigned int i=0; i<treeReader._nSumLHEScaleWeights; i++){
	scaleCrossSectionRatios.push_back(treeReader._sumLHEScaleWeights[i]);
    }
    
    // fill pdf cross section ratios
    for( unsigned int i=0; i<treeReader._nSumLHEPdfWeights; i++){
        pdfCrossSectionRatios.push_back(treeReader._sumLHEPdfWeights[i]);
    }

    // fill ps cross section ratios
    // note: NanoGen/NanoAOD files do not seem to store the sum of PS weights,
    //       assume for now that they are already normalized...
    for( unsigned int i=0; i<4; i++){
        psCrossSectionRatios.push_back(1.);
    }
}


double NanoGenSampleCrossSections::crossSectionRatio_pdfVar( const size_type index ) const{
    if( index > pdfCrossSectionRatios.size() ){
        std::string msg = "Requesting pdf variation " + std::to_string( index );
	msg += " while only " + std::to_string(pdfCrossSectionRatios.size()) + " pdf variations are defined.";
	throw std::out_of_range(msg);
    }
    return pdfCrossSectionRatios[index];
}


double NanoGenSampleCrossSections::crossSectionRatio_scaleVar( const size_type index ) const{
    if( index > scaleCrossSectionRatios.size() ){
        std::string msg = "Requesting scale variation " + std::to_string( index ); 
        msg += " while only " + std::to_string(scaleCrossSectionRatios.size()) + " scale variations are defined.";
        throw std::out_of_range(msg);
    }
    return scaleCrossSectionRatios[index];
}


double NanoGenSampleCrossSections::crossSectionRatio_psVar( const size_type index ) const{
    if( index > psCrossSectionRatios.size() ){
        std::string msg = "Requesting ps variation " + std::to_string( index );
        msg += " while only " + std::to_string(psCrossSectionRatios.size()) + " ps variations are defined.";
        throw std::out_of_range(msg);
    }
    return psCrossSectionRatios[index];
}

