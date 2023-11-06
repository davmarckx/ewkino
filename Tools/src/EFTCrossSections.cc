#include "../interface/EFTCrossSections.h"

//include c++ library classes
#include <stdexcept>
#include <string>

//include ROOT classes
#include "TH1.h"
#include "TROOT.h"

EFTCrossSections::EFTCrossSections( const Sample& sample ){

    // open file and check content
    std::shared_ptr< TFile > sampleFile = sample.filePtr();
    std::shared_ptr< TH1 > eftCounter( dynamic_cast< TH1* >(
	sampleFile->Get( "blackJackAndHookers/eftCounter" ) ) );
    eftCounter->SetDirectory( gROOT );
    if( eftCounter == nullptr ){
        throw std::invalid_argument( std::string("ERROR in EFTCrossSections constructor:")
	    +" eftCounter is not present in file '" + sample.fileName() + "'." );
    }
    int numberOfEFTWeights = eftCounter->GetNbinsX();
    
    // printouts for testing
    std::cout << "INFO from EFTCrossSections constructor:" << std::endl;
    std::cout << "  number of EFT variations: " << numberOfEFTWeights << std::endl;

    // store all eft variations
    double nominalSumOfWeights = eftCounter->GetBinContent(1);
    for( int bin = 1; bin < eftCounter->GetNbinsX() + 1; ++bin ){
        double eftVariedSumOfWeights = eftCounter->GetBinContent( bin );
	eftSumOfWeights.push_back( eftVariedSumOfWeights );
        eftCrossSectionRatios.push_back( eftVariedSumOfWeights / nominalSumOfWeights );
    }
}


double EFTCrossSections::EFTSumOfWeights( const size_type index ) const{
    if( index > eftSumOfWeights.size() ){
	std::string msg = "Requesting EFT cross section variation " + std::to_string( index );
	msg += " while only " + std::to_string( eftSumOfWeights.size() ) + " are present.";
        throw std::out_of_range( msg );
    }
    return eftSumOfWeights[ index ];
}
double EFTCrossSections::EFTCrossSectionRatio( const size_type index ) const{
    if( index > eftCrossSectionRatios.size() ){
	std::string msg = "Requesting EFT cross section variation " + std::to_string( index ); 
        msg += " while only " + std::to_string( eftCrossSectionRatios.size() ) + " are present.";
        throw std::out_of_range( msg );
    }
    return eftCrossSectionRatios[ index ];
}
