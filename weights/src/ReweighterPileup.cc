#include "../interface/ReweighterPileup.h"

//include c++ library classes
#include <vector>

//include ROOT classes
#include "TFile.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/histogramTools.h"


// -----------------
// helper functions 
// -----------------

std::shared_ptr<TH1> getMCPileupHistogram( const Sample& sample ){
    // helper function to retrieve the generator pileup histogram from a MC sample
    std::shared_ptr< TFile > sampleFilePtr = sample.filePtr();
    std::string histPath = "blackJackAndHookers/nTrueInteractions";
    std::shared_ptr< TH1 > pileupMC( dynamic_cast< TH1* >( 
	sampleFilePtr->Get(histPath.c_str()) ) );
    if( pileupMC == nullptr ){
        throw std::runtime_error( std::string("ERROR in ReweighterPileup.getMCPileupHistogram:")
            + " file " + sample.fileName()
            + " does not contain expected histogram " + histPath );
    }
    // make sure the pileup distribution is normalized to unity
    pileupMC->Scale( 1. / pileupMC->GetSumOfWeights() );
    return pileupMC;
}

void computeAndWritePileupWeights( const Sample& sample, const std::string& weightDirectory ){
    // helper function to produce a file with pileup weights for a MC sample
    // update: no longer used for UL!

    // skip data samples
    if( sample.isData() ) return;

    // retrieve the generator pileup histogram
    std::shared_ptr< TH1 > pileupMC = getMCPileupHistogram( sample );
    std::string year = sample.year();

    // initialize pileup weights map
    std::map< std::string, std::shared_ptr< TH1 > > pileupWeights;

    // loop over variations
    for( const auto& var : { "central", "down", "up" } ){
	// read data pileup distribution from given file
        std::string dataPuFilePath = ( stringTools::formatDirectoryName( weightDirectory ) 
	    + "weightFiles/pileupData/" + "dataPuHist_" + year + "Inclusive_" + var + ".root" );
        if( !systemTools::fileExists( dataPuFilePath ) ){
            throw std::runtime_error( std::string("ERROR in ReweighterPileup.computeAndWritePileupWeights:")
		+" file " + dataPuFilePath 
		+" with data pileup weights, necessary for reweighting, is not present." );
        }
        TFile* dataPileupFilePtr = TFile::Open( dataPuFilePath.c_str() );
	std::string histPath = "pileup";
        std::shared_ptr< TH1 > pileupData( dynamic_cast< TH1* >( 
	    dataPileupFilePtr->Get(histPath.c_str()) ) );
	if( pileupData==nullptr ){
            throw std::runtime_error( std::string("ERROR in ReweighterPileup.computeAndWritePileupWeights:")
                +" file " + dataPuFilePath
                +" does not contain expected histogram " + histPath );
        }

        pileupData->SetDirectory( gROOT );

        // make sure the pileup distribution is normalized to unity
        pileupData->Scale( 1. / pileupData->GetSumOfWeights() );

        // divide data and MC histograms to get the weights
        pileupData->Divide( pileupMC.get() );
        pileupWeights[ var ] = std::shared_ptr< TH1 >( 
		dynamic_cast< TH1* >( pileupData->Clone() ) );

        // close the file and make sure the histogram persists
        pileupWeights[ var ]->SetDirectory( gROOT );
        dataPileupFilePtr->Close();
    }

    // write pileup weights to a new ROOT file 
    std::string outputFilePath = stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFiles/pileupWeights/pileupWeights_" + sample.fileName();
    systemTools::makeDirectory( stringTools::directoryNameFromPath( outputFilePath ) );
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str(), "RECREATE" );
    for( const auto& var : { "central", "down", "up" } ){
        pileupWeights[ var ]->Write( ( std::string( "pileupWeights_" ) 
		+ year + "_" + var ).c_str() );
    }
    outputFilePtr->Close();
}


// ------------
// constructor 
// ------------
// note: old version, as used in pre-UL analyses.
//       for UL analyses, a list of samples is not needed anymore, 
//       as all samples are generated with the same pileup distribution,
//	 and hence the weights can be determined centrally, read in and applied directly,
//       without needing to compare the MC pileup to data pileup distribution.

ReweighterPileup::ReweighterPileup( const std::vector< Sample >& sampleList, 
				    const std::string& weightDirectory ){
    
    // read each of the pileup weights into the maps
    for( const auto& sample : sampleList ){

        // skip data samples
        if( sample.isData() ) continue;

        std::string pileupWeightPath = ( stringTools::formatDirectoryName( weightDirectory ) 
	    + "weightFiles/pileupWeights/pileupWeights_" + sample.fileName() );

        // for each sample check if the necessary pileup weights are available, and produce them if not 
        if( !systemTools::fileExists( pileupWeightPath ) ){
            computeAndWritePileupWeights( sample, weightDirectory );
        }

        // extract the pileupweights from the file
        std::string year = sample.year();
	TFile* puWeightFilePtr = TFile::Open( pileupWeightPath.c_str() );
        puWeightsCentral[ sample.uniqueName() ] = std::shared_ptr< TH1 >( 
	    dynamic_cast< TH1* >( puWeightFilePtr->Get( 
	    ( "pileupWeights_" + year + "_central" ).c_str() ) ) );
        puWeightsCentral[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightsDown[ sample.uniqueName() ] = std::shared_ptr< TH1 >( 
	    dynamic_cast< TH1* >( puWeightFilePtr->Get( 
	    ( "pileupWeights_" + year + "_down" ).c_str() ) ) );
        puWeightsDown[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightsUp[ sample.uniqueName() ] = std::shared_ptr< TH1 >( 
	    dynamic_cast< TH1* >( puWeightFilePtr->Get( 
	    ( "pileupWeights_" + year + "_up" ).c_str() ) ) );
        puWeightsUp[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightFilePtr->Close();
    }
}


// ------------
// constructor
// ------------
// note: new version, suitable for UL analyses.

ReweighterPileup::ReweighterPileup( const std::string& pileupWeightPath ) {
    // input arguments:
    // - pileupWeightFile: path to a root file containing directly the reweighting factors
    //                     per number of vertices.
    TFile* puWeightFilePtr = TFile::Open( pileupWeightPath.c_str() );
    puWeightsCentralUL = std::shared_ptr< TH1 >( 
	dynamic_cast< TH1* >( puWeightFilePtr->Get( "nominal" ) ) );
    puWeightsCentralUL->SetDirectory( gROOT );
    puWeightsDownUL = std::shared_ptr< TH1 >( 
	dynamic_cast< TH1* >( puWeightFilePtr->Get( "down" ) ) );
    puWeightsDownUL->SetDirectory( gROOT );
    puWeightsUpUL = std::shared_ptr< TH1 >( 
	dynamic_cast< TH1* >( puWeightFilePtr->Get( "up" ) ) );
    puWeightsUpUL->SetDirectory( gROOT );
    puWeightFilePtr->Close();
    isUL = true;
}


// ---------------------------
// weight retrieval functions 
// ---------------------------

double ReweighterPileup::weight( 
	const Event& event, 
	const std::map< std::string, std::shared_ptr< TH1 > >& weightMap ) const{
    // retrieve weight for an event
    // input arguments:
    // - event: event for which to retrieve the weight
    // - weightMap: map of sample names to weight histograms
    auto it = weightMap.find( event.sample().uniqueName() );
    if( it == weightMap.cend() ){
        throw std::invalid_argument( std::string("ERROR in ReweighterPileup.weight:")
	    +" no pileup weights for sample " + event.sample().uniqueName() 
	    +" found, this sample was probably not present"
	    +" in the vector used to construct the Reweighter." );
    }
    return weight( event, it->second );
}

double ReweighterPileup::weight( 
	const Event& event, 
	const std::shared_ptr< TH1 >& weightHist) const {
    // retrieve weight for an event
    // input arguments:
    // - event: event for which to retrieve the weight
    // - weightHist: histogram containing the weights
    return histogram::contentAtValue( weightHist.get(), event.generatorInfo().numberOfTrueInteractions() );
}

double ReweighterPileup::weight( const Event& event ) const{
    // retrieve weight for an event
    if(isUL){ return weight( event, puWeightsCentralUL ); }
    else{ return weight( event, puWeightsCentral ); }
}

double ReweighterPileup::weightDown( const Event& event ) const{
    // retrieve down-varied weight for and event
    if(isUL){ return weight( event, puWeightsDownUL ); }
    else{ return weight( event, puWeightsDown ); }
}

double ReweighterPileup::weightUp( const Event& event ) const{
    // retrieve up-varied weight for an event
    if(isUL){ return weight( event, puWeightsUpUL ); }
    else{ return weight( event, puWeightsUp ); }
}
