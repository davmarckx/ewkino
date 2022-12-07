#include "../interface/chargeFlipTools.h"

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"


// help function for determining the charge flip weight
double chargeFlips::chargeFlipWeight( 
    double l1Pt, double l1AbsEta, unsigned int l1Flavor,
    double l2Pt, double l2AbsEta, unsigned int l2Flavor,
    const std::shared_ptr< TH2 >& chargeFlipMap, unsigned int flavor ){
    // P( A + B ) = P( A ) + P( B ) - P( A & B )
    double summedProbabilities = 0.;
    double multipliedProbabilities = 1.;
    if( l1Flavor==flavor ){
	double flipRate = histogram::contentAtValues( chargeFlipMap.get(), l1Pt, l1AbsEta );
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    if( l2Flavor==flavor ){
	double flipRate = histogram::contentAtValues( chargeFlipMap.get(), l2Pt, l2AbsEta );
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    return ( summedProbabilities - multipliedProbabilities );
}

// help function for determining the charge flip weight
double chargeFlips::chargeFlipWeight( const Event& event, const std::shared_ptr< TH2 >& chargeFlipMap ){
    // P( A + B ) = P( A ) + P( B ) - P( A & B )
    double summedProbabilities = 0.;
    double multipliedProbabilities = 1.;
    for( const auto& electronPtr : event.electronCollection() ){
	double flipRate = histogram::contentAtValues( chargeFlipMap.get(), electronPtr->pt(), electronPtr->absEta() );
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    return ( summedProbabilities - multipliedProbabilities );
}

// help function for reading the charge flip map
std::shared_ptr< TH2D > chargeFlips::readChargeFlipMap( const std::string& filePath, 
					   const std::string& year,
					   const std::string flavour ){
    TFile* frFile = TFile::Open( filePath.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >(
    frFile->Get( ( "chargeFlipRate_" + flavour + "_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();
    return frMap;
}

// help function for reading the charge flip map
std::shared_ptr< TH2D > chargeFlips::readChargeFlipMap( const std::string& directory,
					   const std::string& year, 
					   const std::string& flavour,
					   const std::string& process,
					   const std::string& binning ){
    std::string fileName = "chargeFlipMap_MC_" + flavour + "_" + year;
    fileName += "_process_" + process + "_binning_" + binning + ".root";
    std::string filePath = stringTools::formatDirectoryName(directory)+fileName;
    return readChargeFlipMap(filePath, year, flavour);
}
