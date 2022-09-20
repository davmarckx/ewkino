/*
Tools related to fake rate measurement
*/

// include header
#include "../interface/readFakeRateTools.h"


std::shared_ptr< TH2D > readFakeRateTools::readFRMap( 
				    const std::string& pathToFile,
				    const std::string& histName ){
    // read a TH2D fake rate map from specified file and hsitogram name
    TFile* frFile = TFile::Open( pathToFile.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >( frFile->Get( histName.c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();

    /* // printouts for testing
    std::cout<<"values:"<<std::endl;
    for(unsigned xbin=1; xbin<=5; ++xbin){
        for(unsigned ybin=1; ybin<=3; ++ybin){
            std::cout<<"bin: "<<xbin<<" "<<ybin<<std::endl;
            std::cout<<frMap->GetBinContent(xbin,ybin)<<std::endl;
        }
    }*/

    return frMap;
}


std::shared_ptr< TH2D > readFakeRateTools::readFRMap( 
				    const std::string& pathToFile,
				    const std::string& flavor, 
				    const std::string& year ){
    // read a TH2D fake rate map for given flavor and year,
    // with default naming convention
    std::string histName = "fakeRate_" + flavor + "_" + year;
    return readFRMap( pathToFile, histName );
}


double readFakeRateTools::fakeRateWeight( 
			const Event& event,
			const std::shared_ptr< TH2D >& frMap_muon,
			const std::shared_ptr< TH2D >& frMap_electron ){
    // return the fake rate weight for a given event and fake rate maps
    double weight = -1.;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
	if( !(leptonPtr->isFO() && !leptonPtr->isTight()) ) continue;

	double ptMax = 44.9; // limit to bin up to 45 GeV
	double croppedPt = std::min( leptonPtr->pt(), ptMax );
	double croppedAbsEta = std::min( leptonPtr->absEta(), (leptonPtr->isMuon() ? 2.4 : 2.5) );
	double fr = 0;
	if( leptonPtr->isMuon() ){
	    fr = frMap_muon->GetBinContent( 
		 frMap_muon->FindBin( croppedPt, croppedAbsEta ) );
	} else {
	    fr = frMap_electron->GetBinContent( 
		 frMap_electron->FindBin( croppedPt, croppedAbsEta ) );
	}
    
	// printouts for testing:
	/*std::cout<<"--- lepton ---"<<std::endl;
	std::cout<<"isMuon: "<<leptonPtr->isMuon()<<std::endl;
	std::cout<<"cropped pt: "<<croppedPt<<std::endl;
	std::cout<<"cropped eta: "<<croppedAbsEta<<std::endl;
	std::cout<<"fake rate: "<<fr<<std::endl;*/

	// calculate weight
	weight *= ( - fr / ( 1. - fr ) );
    }
    //std::cout<<"weight: "<<weight<<std::endl;
    return weight;
}


int readFakeRateTools::fakeRateFlavour( const Event& event ){
    // return flavour of failing lepton:
    // -1 if none,
    // 0 if muon
    // 1 if electron
    // 2 if multiple
    int frflav = -1;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
        if( !(leptonPtr->isFO() && !leptonPtr->isTight()) ) continue;
        if( frflav==-1 && leptonPtr->isMuon() ) frflav = 0;
        else if( frflav==-1 && leptonPtr->isElectron() ) frflav = 1;
        else{ frflav = 2; }
    }
    return frflav;
}
