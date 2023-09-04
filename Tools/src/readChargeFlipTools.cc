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
    bool verbose = false; // set to true for testing
    double summedProbabilities = 0.;
    double multipliedProbabilities = 1.;
    if( verbose ){
	std::cout << "start chargeFlipWeight for event " << event.eventNumber() << std::endl; }
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
	double flipRate = 0;
	if( leptonPtr->isElectron() ){
	    flipRate = histogram::contentAtValues( 
		chargeFlipMap.get(), leptonPtr->pt(), leptonPtr->absEta() );
	    if( verbose ){
		std::cout << "  - electron with following properties:";
		std::cout << " pt: " << leptonPtr->pt() << " / absEta: " << leptonPtr->absEta();
		std::cout << " / flipRate: " << flipRate << std::endl;
	    }
	} else if( leptonPtr->isMuon() ){
	    flipRate = 0; // maybe extend later with muon CF rate
	    if( verbose ) std::cout << " - muon (flipRate = 0)" << std::endl;
	}
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    double totalProbability = summedProbabilities - multipliedProbabilities;
    if( verbose ){
	std::cout << "total weight for this event (before correction factor):";
	std::cout << " " << totalProbability << std::endl;
    }
    if( !doCorrectionFactor ) return totalProbability;
    std::string year = event.sample().year();
    // need extra step for year since it is not split in Pre and Post for 2016 data.
    std::string sampleName = event.sample().fileName();
    if( year=="2016" ){
	if( stringTools::stringContains(sampleName,"HIPM") ) year = "2016PreVFP";
        else year = "2016PostVFP";
    }
    if( year=="2016PreVFP" ) return 0.85 * totalProbability;
    if( year=="2016PostVFP" ) return 0.95 * totalProbability;
    if( year=="2017" ) return 1.4 * totalProbability;
    if( year=="2018" ) return 1.4 * totalProbability;
    std::string msg = "ERROR in readChargeFlipTools::chargeFlipWeight:";
    msg += " year " + year + " not recognized (needed for correction factor).";
    throw std::runtime_error(msg);
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
