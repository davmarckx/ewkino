/*
Test reading of EFT coefficients
*/

// include other parts of the framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../Event/interface/EFTInfo.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/HistInfo.h"


int main( int argc, char* argv[] ){

    int nargs = 2;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: EFTInfo_test.cc requires " << nargs;
	std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- path to input file (.root)" << std::endl;
	std::cerr << "- number of events to process" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputFilePath = argvStr[1];
    unsigned long nEvents = std::stoul(argvStr[2]);

    // define wilson coefficients to check
    std::vector<std::string> wcnames = {
            "ctlTi",  "ctq1",  "ctq8",  "cQq83",  "cQq81",
            "cQlMi",  "cbW",  "cpQ3",  "ctei",  "cQei",
            "ctW",  "cpQM",  "ctlSi",  "ctZ",  "cQl3i",
            "ctG",  "cQq13",  "cQq11",  "cptb",  "ctli",
            "ctp",  "cpt"
    };

    // read the input file
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    // initialize the first event and do some printouts
    Event event = treeReader.buildEvent(0, false, false, false, false, false, true);
    std::cout << event.eftInfo().numberOfEFTCoefficients() << std::endl;

    // initialize histograms for observables
    HistInfo histInfo = HistInfo( "njets", "njets", 8, -0.5, 7.5 );
    std::map< std::string, std::map< std::string, std::shared_ptr<TH1D> >> histograms;
    histograms["njets"]["nominal"] = histInfo.makeHist("njets_nominal");
    for( std::string wcname: wcnames ){
        histograms["njets"][wcname] = histInfo.makeHist( "njets_"+wcname );
    }
    
    // do event loop
    for(long unsigned entry = 0; entry < nEvents; entry++){
        Event event = treeReader.buildEvent(entry,false,false,false,false,false,true);
	// get observable quantities
	int njets = event.jetCollection().goodJetCollection().size();
	// get nominal weight
	double nominal_weight = 1; // use 1 since we will use relative weight below
	// loop over wilson coefficients and derive alternative weights
	std::map<std::string, double> wcweights;
	for( std::string wcname: wcnames ){
	    std::map<std::string, double> wcvalues = {{wcname, 1.0}};
	    double weight = nominal_weight + event.eftInfo().weight(wcvalues);
	    wcweights[wcname] = weight;
	}
	// fill histograms
	histograms["njets"]["nominal"]->Fill(njets, nominal_weight);
	for( std::string wcname: wcnames ){
	    histograms["njets"][wcname]->Fill(njets, wcweights[wcname]);
	}
    }

    // write output file
    TFile* filePtr = TFile::Open( "output_test.root", "recreate" );
    for( const auto el: histograms ){
	for( const auto el2: el.second ){
	    el2.second->Write();
	}
    }
    filePtr->Close(); 
}
