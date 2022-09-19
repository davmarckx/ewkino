/*
Perform a closure test for the MC charge flip rates
*/

// include c++ library classes 
#include <memory>
#include <thread>

// include ROOT classes
#include "TH2D.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/HistInfo.h"
#include "../plotting/interface/plotCode.h"
#include "../plotting/interface/tdrStyle.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"

// include dedicated tools
#include "interface/chargeFlipSelection.h"
#include "interface/chargeFlipTools.h"


// help function for initializing the histograms
std::vector< HistInfo > makeDistributionInfo( const std::string& process ){
	std::vector< HistInfo > histInfoVec = {
		HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 10, 25, 200 ),
		HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 10, 15, 150 ),

		HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
		HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),
		
		HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 0, 300 ),
		( ( process == "DY" ) ? 
		    HistInfo( "mll", "M_{ll} (GeV)", 10, 70, 120 ) : 
		    HistInfo( "mll", "M_{ll} (GeV)", 10, 0, 200 ) ),
		HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 300 ),
		HistInfo( "ht", "H_{T} (GeV)", 10, 0, 600 ),
		HistInfo( "mt2l", "M_{T}^{2l} (GeV)", 10, 0, 300 ),

		HistInfo( "nJets", "number of jets", 8, 0, 8 ),
		HistInfo( "nBJets", "number of b-jets (medium deep CSV)", 4, 0, 4 ),
		HistInfo( "nVertex", "number of vertices", 10, 0, 70 )
    };
    return histInfoVec;
}


// help function for reading the charge flip map
std::shared_ptr< TH2D > readChargeFlipMap( const std::string& year ){
    TFile* frFile = TFile::Open( ("chargeFlipMaps/chargeFlipMap_MC_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >( frFile->Get( ( "chargeFlipRate_electron_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();
    return frMap;
} 


void closureTest_MC( const std::string& process, 
		     const std::string& year, 
		     const std::string& sampleListFile,
		     const std::string& sampleDirectory,
		     const long nEntries ){

    // check process string
    if( ! (process == "TT" || process == "DY" ) ){
        throw std::invalid_argument( "Given closure test process argument is '" + process 
		+ "' while it should be DY or TT." );
    }

    // make collection of histograms
    std::vector< std::shared_ptr< TH1D > > observedHists; 
    std::vector< std::shared_ptr< TH1D > > predictedHists;

    std::vector< HistInfo > histInfoVec = makeDistributionInfo( process );

    for( const auto& histInfo : histInfoVec ){
        observedHists.push_back( histInfo.makeHist( 
	    histInfo.name() + "_observed_" + process + "_" + year ) );
        predictedHists.push_back( histInfo.makeHist( 
	    histInfo.name() + "_predicted_"  + process + "_" + year ) );
    }
    
    // read fake-rate map corresponding to this year and flavor 
    std::shared_ptr< TH2D > chargeFlipMap_electron = readChargeFlipMap( year );


    // make TreeReader and loop over samples
    TreeReader treeReader( sampleListFile, sampleDirectory );
    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample();
    
        // loop over entries
        long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEntries>0 && (unsigned)nEntries<numberOfEntries ){
            numberOfEntries = (unsigned) nEntries;
        }
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
 
	    // build event
	    Event event = treeReader.buildEvent( entry );

            // apply event selection
            if( !chargeFlips::passChargeFlipEventSelection( event, true, false, false ) ) continue;

            // get electron collection
            ElectronCollection electrons = event.electronCollection();

	    // consider only events with only prompt leptons
            bool promptElectrons = true;
            for( const auto& electronPtr : electrons ){
                if( ! electronPtr->isPrompt() ){
                    promptElectrons = false;
                    break;
                }
            }
            if( !promptElectrons ) continue;

            // compute plotting variables 
            std::vector< double > variables = { electrons[0].pt(), electrons[1].pt(),
                electrons[0].absEta(), electrons[1].absEta(),
                event.metPt(),
                ( event.electron( 0 ) + event.electron( 1 ) ).mass(),
                electrons.scalarPtSum() + event.metPt(),
                event.HT(),
                mt( electrons.objectSum(), event.met() ),
                static_cast< double >( event.numberOfJets() ),
                static_cast< double >( event.numberOfMediumBTaggedJets() ),
                static_cast< double >( event.numberOfVertices() )
            };
                
            // event is 'observed' if an electron is assigned the wrong charge
            bool isObserved = false;
            for( const auto& electronPtr:  electrons ){
                if( electronPtr->isChargeFlip() ){
                    isObserved = true;
                }
            }

            if( isObserved ){
                for( std::vector< double >::size_type v = 0; v < variables.size(); ++v ){
                    observedHists[v]->Fill( std::min( variables[v],  
			histInfoVec[v].maxBinCenter() ), event.weight() );
                }

            } else {
                // compute event weight with fake-rate
                double weight = event.weight()*chargeFlips::chargeFlipWeight( 
				    event, chargeFlipMap_electron );
                for( std::vector< double >::size_type v = 0; v < variables.size(); ++v ){
                    predictedHists[v]->Fill( std::min( variables[v],  
			histInfoVec[v].maxBinCenter() ), weight );
                }
            }
        }
    }

    // make plot output directory
    std::string outputDirectory_name = "./closurePlots_chargeFlips_MC_" + process + "_" + year; 
    systemTools::makeDirectory( outputDirectory_name );
    
    // make plots 
    for( std::vector< HistInfo >::size_type v = 0; v < histInfoVec.size(); ++v ){
		std::string names[2] = {"MC observed", "charge-flip rate prediction"};
        std::vector< TH1D* > predicted = { predictedHists[v].get() };
        std::string header;
        if( year == "2016" ){
            header = "35.9 fb^{-1}";
        } else if( year == "2017" ){
            header = "41.5 fb^{-1}";
        } else{
            header = "59.7 fb^{-1}";
        }
       	plotDataVSMC( observedHists[v].get(), &predicted[0], names, 1, 
	    stringTools::formatDirectoryName( outputDirectory_name ) + histInfoVec[v].name() 
	    + "_closureTest_chargeFlips_MC_" + process + "_" + year + ".pdf", "", 
	    false, false, header );
    }
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 7 ) ){
        std::cerr << "ERROR: found " << argc-1 << " command line args,";
        std::cerr << " while 6 are needed:" << std::endl;
        std::cerr << "  - process" << std::endl;
	std::cerr << "  - flavour (only 'electron' supported for now)" << std::endl;
        std::cerr << "  - year" << std::endl;
        std::cerr << "  - sample list" << std::endl;
        std::cerr << "  - sample directory" << std::endl;
        std::cerr << "  - number of entries" << std::endl;
        return 1;
    }
    std::string process = argvStr[1];
    std::string flavor = argvStr[2];
    std::string year = argvStr[3];
    std::string sampleList = argvStr[4];
    std::string sampleDirectory = argvStr[5];
    long nEntries = std::stol(argvStr[6]);
    setTDRStyle();
    closureTest_MC(
        process, year, sampleList, sampleDirectory, nEntries);
    std::cerr << "###done###" << std::endl;
    return 0; 
}
