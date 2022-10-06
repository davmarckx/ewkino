/*
Test for reading the correct scale factor from the map
*/

// import lepton ID reweighter
#include "../../interface/LeptonReweighter.h"
#include "../../interface/ConcreteSelection.h"
#include "../../interface/ConcreteLeptonReweighter.h"

// include c++ library classes
#include <iostream>
#include <memory>

// include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

// include other parts of framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../test/copyMoveTest.h"


double etaFunction( const Muon& muon ){
    return muon.absEta();
}


double etaFunction( const Electron& electron ){
    return fabs( electron.etaSuperCluster() );
}


template< typename LeptonType > double leptonSFManual( 
    const LeptonType& lepton, 
    const std::shared_ptr< TH2 >& weights ){
    // read lepton scale factor manually from a histogram
    // input arguments:
    // - lepton: an instance of Electron or Muon
    // - weights: 2D histogram with scale factors
    double weight = 1.;
    if( lepton.isTight() ){
        double maxEta = ( lepton.isMuon() ? 2.39 : 2.49 );
        int bin = weights->FindBin( std::min( etaFunction( lepton ), maxEta ), std::min( lepton.pt(), 119. ) );
        weight = weights->GetBinContent( bin );
    }
    return weight;
}


bool doubleEqual( double lhs, double rhs ){
    return fabs( ( lhs - rhs )/lhs ) < 1e-6;
}


int main( int argc, char* argv[] ){

    int nargs = 2;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: fillTestReweighterPileup.cc requires " << nargs;
        std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- input file" << std::endl;
        std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputFilePath = argvStr[1];
    long unsigned nEvents = std::stoul(argvStr[2]);

    // make a TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );
    std::string year = treeReader.getYearString();

    // read muon scale factors and make reweighter
    std::string muonSFFileName = std::string("../../weightFilesUL/leptonSF")
				    +"/leptonMVAUL_SF_muons_Medium_"+year+".root";
    TFile* muonSFFile = TFile::Open( (muonSFFileName).c_str() );
    std::shared_ptr< TH2 > muonSFHist_nom( dynamic_cast< TH2* >(
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt" ) ) );
    muonSFHist_nom->SetDirectory( gROOT );
    muonSFFile->Close();
    MuonReweighter muonReweighter_nom( muonSFHist_nom, new TightSelector );

    // read electron scale factors and make reweighter
    std::string eleSFFileName = std::string("../../weightFilesUL/leptonSF/")
				+"leptonMVAUL_SF_electrons_Tight_"+year+".root";
    TFile* eleSFFile = TFile::Open( (eleSFFileName).c_str() );
    std::shared_ptr< TH2 > electronSFHist_nom( dynamic_cast< TH2* >
    ( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist_nom->SetDirectory( gROOT );
    eleSFFile->Close();
    ElectronIDReweighter electronIDReweighter_nom( electronSFHist_nom, new TightSelector );
    
    // check behavior under copy/move etc
    copyMoveTest( muonReweighter_nom );
    copyMoveTest( electronIDReweighter_nom );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents==0 ) nEvents = numberOfEntries;
    else nEvents = std::min(nEvents, numberOfEntries);
    std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
    for( long unsigned entry = 0; entry < nEvents; ++entry ){
	Event event = treeReader.buildEvent( entry );
	// compare scale factors computed with LeptonReweighter to manual computation
        for( const auto& muonPtr : event.muonCollection() ){        
            double nominalWeight = muonReweighter_nom.weight( *muonPtr );
            double nominalWeightManual = leptonSFManual( *muonPtr, muonSFHist_nom );
            if( ! doubleEqual( nominalWeight, nominalWeightManual ) ){
                std::cout << "pt = " << muonPtr->pt() << "\teta = " << muonPtr->eta() << std::endl;
		std::string msg = "Nominal muon weight given by Reweighter is " + std::to_string( nominalWeight );
		msg.append( ", while manual computation gives " + std::to_string( nominalWeightManual ) );
                    throw std::runtime_error( msg );
            }
        }
        for( const auto& electronPtr : event.electronCollection() ){
	    double nominalWeight = electronIDReweighter_nom.weight( *electronPtr );
            double nominalWeightManual = leptonSFManual( *electronPtr, electronSFHist_nom );
            if( ! doubleEqual( nominalWeight, nominalWeightManual ) ){
		std::cout << "pt = " << electronPtr->pt() << "\teta = " << electronPtr->eta() << std::endl;
		std::string msg = "Nominal muon weight given by Reweighter is " + std::to_string( nominalWeight );
                msg.append( ", while manual computation gives " + std::to_string( nominalWeightManual ) );
		throw std::runtime_error( msg );
            }
        }
    }
    return 0;
}
