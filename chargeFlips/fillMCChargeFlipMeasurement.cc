/*
Perform a charge flip measurement in MC
*/

// include c++ library classes 
#include <memory>
#include <thread>

// include ROOT classes
#include "TH2D.h"
#include "TStyle.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/histogramTools.h"
#include "../Tools/interface/analysisTools.h"
#include "../plotting/interface/plotCode.h"
#include "../plotting/interface/tdrStyle.h"

// include dedicated tools
#include "interface/chargeFlipSelection.h"


void determineMCChargeFlipRate( const std::string& year, 
				const std::string& sampleListFile, 
				const std::string& sampleDirectory,
				const long nEntries ){

    // simple check on provided year identifier
    analysisTools::checkYearString( year );

    // initialize bins
    const std::vector< double > ptBins = {10., 20., 30., 45., 65., 100., 200.};
    const std::vector< double > etaBins = { 0., 0.8, 1.442, 2.5 };

    // initialize 2D histogram for numerator
    std::string numerator_name = "chargeFlipRate_numerator_electron_" + year;
    std::shared_ptr< TH2D > numeratorMap( 
	new TH2D( numerator_name.c_str(), (numerator_name+"; p_{T} (GeV); |#eta|").c_str(), 
	ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    numeratorMap->Sumw2();

    // initialize 2D histogram for denominator
    std::string denominator_name = "chargeFlipRate_denominator_electron_" + year;
    std::shared_ptr< TH2D > denominatorMap( 
	new TH2D( denominator_name.c_str(), (denominator_name+"; p_{T} (GeV); |#eta|").c_str(), 
	ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    denominatorMap->Sumw2();

    // initialize 2D histogram for ratio
    std::string ratio_name = "chargeFlipRate_electron_" + year;
    std::shared_ptr< TH2D > ratioMap(
        new TH2D( ratio_name.c_str(), (ratio_name+"; p_{T} (GeV); |#eta|").c_str(),
        ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    ratioMap->Sumw2();

    // make TreeReader and loop over samples
    TreeReader treeReader( sampleListFile, sampleDirectory );
    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample();
    
	// loop over entries
	long unsigned numberOfEntries = treeReader.numberOfEntries();
	if( nEntries>0 && (unsigned)nEntries<numberOfEntries ){ 
	    numberOfEntries = (unsigned) nEntries; 
	}
	std::cout << "starting loop over " << numberOfEntries << " events." << std::endl;
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){

	    // build the event
            Event event = treeReader.buildEvent( entry );

            // apply electron selection
	    // arguments are: diElectron, onZ, bVeto
            if( ! chargeFlips::passChargeFlipEventSelection(event, false, false, false) ) continue;

	    // loop over electrons in the event
            for( auto& electronPtr : event.electronCollection() ){
                Electron& electron = *electronPtr;
            
                // require prompt leptons
                if( !( electron.isPrompt() ) ) continue;
		// require that the lepton does not come from photon conversion
		// (since they are matched to photons so the matched charge is 0)
                if( electron.matchPdgId() == 22 ) continue;

                // fill denominator histogram 
                histogram::fillValues( denominatorMap.get(), electron.pt(), electron.absEta(), 1. );
    
                //fill numerator histogram
                if( electron.isChargeFlip() ){
                    histogram::fillValues( numeratorMap.get(), electron.pt(), electron.absEta(), 1. );
		    histogram::fillValues( ratioMap.get(), electron.pt(), electron.absEta(), 1. );
                }
            }
        }
    }

    // divide numerator by denominator to get charge flip rate
    ratioMap->Divide( denominatorMap.get() );

    // create output directory if it does not exist 
    std::string outputDirectory = "chargeFlipMaps";
    systemTools::makeDirectory( outputDirectory );
    
    // plot fake-rate map
    // write numbers in exponential notation because charge flip rates tend to be very small
    gStyle->SetPaintTextFormat( "4.2e" );
    std::string plotOutputPath =  stringTools::formatDirectoryName( outputDirectory );
    plotOutputPath += "chargeFlipMap_MC_" + year + ".pdf";
    plot2DHistogram( ratioMap.get(), plotOutputPath );

    // write fake-rate map to file 
    std::string rootOutputPath = stringTools::formatDirectoryName( outputDirectory ); 
    rootOutputPath += "chargeFlipMap_MC_" + year + ".root";
    TFile* outputFile = TFile::Open( rootOutputPath.c_str(), "RECREATE" );
    ratioMap->Write();
    numeratorMap->Write();
    denominatorMap->Write();
    outputFile->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 6 ) ){
        std::cerr << "ERROR: found " << argc-1 << " command line args,";
	std::cerr << " while 5 are needed:" << std::endl;
        std::cerr << "  - flavour (only 'electron' supported for now)" << std::endl;
	std::cerr << "  - year" << std::endl;
	std::cerr << "  - sample list" << std::endl;
	std::cerr << "  - sample directory" << std::endl;
	std::cerr << "  - number of entries" << std::endl;
        return 1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2];
    std::string sampleList = argvStr[3];
    std::string sampleDirectory = argvStr[4];
    long nEntries = std::stol(argvStr[5]);
    setTDRStyle();
    determineMCChargeFlipRate(
	year, sampleList, sampleDirectory, nEntries);
    std::cerr << "###done###" << std::endl;
    return 0;
}
