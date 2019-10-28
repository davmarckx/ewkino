

//include c++ library classes 
#include <memory>
#include <thread>

//include ROOT classes
#include "TH2D.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "interface/fakeRateTools.h"
#include "interface/fakeRateSelection.h"
#include "../Tools/interface/systemTools.h"
#include "../plotting/plotCode.h"


void determineMCFakeRate( const std::string& flavor, const std::string& year, const std::string& sampleListFile, const std::string& sampleDirectory ){

    
    fakeRate::checkFlavorString( flavor );
    fakeRate::checkYearString( year );
    const bool isMuon =  ( flavor == "muon" );
    
    std::vector< double > ptBins = {10., 20., 30., 45., 65., 100.};
    std::vector< double > etaBins;
    if( isMuon ){
        etaBins = { 0., 1.2, 2.1, 2.4 }; 
    } else {
		etaBins = { 0., 0.8, 1.442, 2.5 };
    }
    const double maxPtBin = ( ptBins[ ptBins.size() - 1 ]  +  ptBins[ ptBins.size() - 2 ] )/2.;
    const double maxEtaBin = ( etaBins[ etaBins.size() - 1 ] + etaBins[ etaBins.size() - 2 ] )/2.;

	//initialize 2D histograms for numerator and denominator
	std::string numerator_name = "fakeRate_" + flavor + "_" + year;
    std::shared_ptr< TH2D > numeratorMap( new TH2D( numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV); |#eta|").c_str(), 5, &ptBins[0], 3, &etaBins[0] ) );
	std::string denominator_name = "fakeRate_denominator_" + flavor + "_" + year;
    std::shared_ptr< TH2D > denominatorMap( new TH2D( denominator_name.c_str(), denominator_name.c_str(), 5, &ptBins[0], 3, &etaBins[0] ) );


	//loop over samples to fill histograms
    TreeReader treeReader( sampleListFile, sampleDirectory );

    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample();
    
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );

            //apply fake-rate selection
            if( !fakeRate::passFakeRateEventSelection( event, isMuon, !isMuon, false, true, 0.7) ) continue;

            LightLepton& lepton = event.lightLeptonCollection()[ 0 ];

            //lepton should be nonprompt and should not originate from a photon
            if( lepton.isPrompt() ) continue;
            if( lepton.matchPdgId() == 22 ) continue;

            //fill denominator histogram 
            denominatorMap->Fill( std::min( lepton.pt(), maxPtBin ), std::min( lepton.absEta(), maxEtaBin ), event.weight() );
    
            //fill numerator histogram
            if( lepton.isTight() ){
                numeratorMap->Fill( std::min( lepton.pt(), maxPtBin ), std::min( lepton.absEta(), maxEtaBin ), event.weight() );
            }
        }
    }

    //divide numerator and denominator to get fake-rate
    numeratorMap->Divide( denominatorMap.get() );

    //create output directory if it does not exist 
    systemTools::makeDirectory( "fakeRateMaps" );

    //plot fake-rate map
    std::string plotOutputPath = "fakeRateMaps/fakeRateMap_MC_" + flavor + "_" + year + ".pdf";
    plot2DHistogram( numeratorMap.get(), plotOutputPath );

    //write fake-rate map to file 
    std::string rootOutputPath = "fakeRateMaps/fakeRateMap_MC_" + flavor + "_" + year + ".root";
    TFile* outputFile = TFile::Open( rootOutputPath.c_str(), "RECREATE" );
    numeratorMap->Write();
    outputFile->Close();
}


int main(){

    //TODO : expand implementation here to use job submission!

   	//make sure ROOT behaves itself when running multithreaded
    ROOT::EnableThreadSafety();
    std::vector< std::thread > threadVector;
    threadVector.reserve( 2 );
	for( const auto& flavor : {"muon", "electron" } ){
        for( const auto& year : {"2016", "2017", "2018" } ){
            std::string sampleListFile = std::string( "samples_tuneFOSelection_" ) + flavor + "_" + year + ".txt";
		    threadVector.emplace_back( determineMCFakeRate, flavor, year, sampleListFile, "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_fakerate/" );
        }
	}

    for( auto& t : threadVector ){
        t.join();
    }
	
    return 0;
}
