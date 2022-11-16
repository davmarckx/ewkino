/*
Tools for reading charge flip maps and weights
*/

// include header
#include "../interface/readChargeFlipTools.h"

// help function for reading charge flip weights
double readChargeFlipTools::chargeFlipWeight( 
	const Event& event, 
	const std::shared_ptr< TH2 >& chargeFlipMap,
	bool doCorrectionFactor ){
    // P( A + B ) = P( A ) + P( B ) - P( A & B )
    double summedProbabilities = 0.;
    double multipliedProbabilities = 1.;
    for( const auto& electronPtr : event.electronCollection() ){
        double flipRate = histogram::contentAtValues( 
	    chargeFlipMap.get(), electronPtr->pt(), electronPtr->absEta() );
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    double totalProbability = summedProbabilities - multipliedProbabilities;
    if( !doCorrectionFactor ) return totalProbability;
    std::string year = event.sample().year();
    if( year=="2016PreVFP" ) return 0.85 * totalProbability;
    if( year=="2016PostVFP" ) return 0.95 * totalProbability;
    if( year=="2017" ) return 1.4 * totalProbability;
    if( year=="2018" ) return 1.4 * totalProbability;
    return 0.;
}

// help function for reading the charge flip map
std::shared_ptr< TH2D > readChargeFlipTools::readChargeFlipMap( 
	const std::string& filePath,
	const std::string& year,
        const std::string& flavour ){
    TFile* frFile = TFile::Open( filePath.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >(
    frFile->Get( ( "chargeFlipRate_" + flavour + "_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();
    return frMap;
}

// help function for reading the charge flip map
std::shared_ptr< TH2D > readChargeFlipTools::readChargeFlipMap( 
	const std::string& directory,
        const std::string& year,
        const std::string& flavour,
	const std::string& process,
	const std::string& binning ){
    std::string fileName = "chargeFlipMap_MC_" + flavour + "_" + year;
    fileName += "_process_" + process + "_binning_" + binning + ".root";
    std::string filePath = stringTools::formatDirectoryName(directory)+fileName;
    return readChargeFlipMap(filePath, year, flavour);
}
