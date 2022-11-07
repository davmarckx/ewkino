#include "../interface/SampleCrossSections.h"

//include c++ library classes
#include <stdexcept>
#include <string>

//include ROOT classes
#include "TH1.h"
#include "TROOT.h"

SampleCrossSections::SampleCrossSections( const Sample& sample ){

    // open file and check content
    std::shared_ptr< TFile > sampleFile = sample.filePtr();

    std::shared_ptr< TH1 > hCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/hCounter" ) ) );
    hCounter->SetDirectory( gROOT );
    if( hCounter == nullptr ){
        throw std::invalid_argument( std::string("ERROR in SampleCrossSections constructor:")
	    +" hCounter is not present in file '" + sample.fileName() + "'." );
    } 

    std::shared_ptr< TH1 > lheCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/lheCounter" ) ) );
    lheCounter->SetDirectory( gROOT );
    if( lheCounter == nullptr ){
        throw std::invalid_argument( std::string("ERROR in SampleCrossSections constructor:")
	    +" lheCounter is not present in file '" + sample.fileName() + "'." );
    } 

    std::shared_ptr< TH1 > psCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/psCounter" ) ) );
    psCounter->SetDirectory( gROOT );
    if( psCounter == nullptr ){
        throw std::invalid_argument( std::string("ERROR in SampleCrossSections constructor:")
	    +" psCounter is not present in file '" + sample.fileName() + "'." );
    }

    double nominalSumOfWeights = hCounter->GetBinContent( 1 );
    int numberOfLheWeights = lheCounter->GetNbinsX();
    int numberOfPsWeights = psCounter->GetNbinsX();
    
    // printouts for testing
    std::cout << "INFO from SampleCrossSections constructor:" << std::endl;
    std::cout << "  number of lhe variations: " << numberOfLheWeights << std::endl;
    std::cout << "  number of ps variations: " << numberOfPsWeights << std::endl;

    // store all lhe variations
    bool doLheWarning = true;
    for( int bin = 1; bin < lheCounter->GetNbinsX() + 1; ++bin ){
        double lheVariedSumOfWeights = lheCounter->GetBinContent( bin );
        // 0 entries indicates that a sample didn't have the respective weights
        if( lheVariedSumOfWeights < 1e-6 ){
	    if( doLheWarning ){
		std::string msg = "WARNING in SampleCrossSection constructor:";
		msg.append( " some or all of the lhe varied sum-of-weights are zero, " );
		msg.append( " will use nominal sum-of-weights instead." );
		std::cerr << msg << std::endl;
		doLheWarning = false;
	    }
	    lheVariedSumOfWeights = nominalSumOfWeights;
	}
        lheCrossSectionRatios.push_back( lheVariedSumOfWeights / nominalSumOfWeights );
    }

    // store all parton shower variations
    bool doPsWarning = true;
    for( int bin = 1; bin < psCounter->GetNbinsX() + 1; ++bin ){
        double psVariedSumOfWeights = psCounter->GetBinContent( bin );
        // 0 entries indicates that a sample didn't have the respective weights
        if( psVariedSumOfWeights < 1e-6 ){
	    if( doPsWarning ){
                std::string msg = "WARNING in SampleCrossSection constructor:";
                msg.append( " some or all of the ps varied sum-of-weights are zero, " );
                msg.append( " will use nominal sum-of-weights instead." );
                std::cerr << msg << std::endl;
                doPsWarning = false;
            }
            psVariedSumOfWeights = nominalSumOfWeights;
	}
        psCrossSectionRatios.push_back( psVariedSumOfWeights / nominalSumOfWeights );
    }
}


double SampleCrossSections::crossSectionRatio_lheVar( const size_type index ) const{
    if( index > lheCrossSectionRatios.size() ){
        throw std::out_of_range( "Requesting lhe cross section variation " + std::to_string( index ) + " while only " + std::to_string( lheCrossSectionRatios.size() ) + " are present." );
    }
    return lheCrossSectionRatios[ index ];
}


double SampleCrossSections::crossSectionRatio_pdfVar( const size_type index ) const{

    //the tenth entry in the lhe cross section ratios is the first one that corresponds to pdfs
    return crossSectionRatio_lheVar( index + 9 );
}


double SampleCrossSections::crossSectionRatio_scaleVar( const size_type index ) const{

    //entries up to the ninth one in the lhe cross sectio ratios correspond to scales
    if( index > 8 ){
        throw std::out_of_range( "Requesting scale variation " + std::to_string( index ) + " while only 9 scale variations are defined." );
    }
    return crossSectionRatio_lheVar( index );
}


double SampleCrossSections::crossSectionRatio_psVar( const size_type index ) const{
    if( index > psCrossSectionRatios.size() ){
        throw std::out_of_range( "Requesting ps cross section variation " + std::to_string( index ) + " while only " + std::to_string( psCrossSectionRatios.size() ) + " are present." );
    }
    return psCrossSectionRatios[ index ];
}
