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
    // note: the values of the coefficients have been tuned
    // to obtain "reasonably" varying distributions on a TTW sample
    std::map<std::string, double> wcconfig = {
        {"ctlTi", 0.1},
        {"ctq1", 0.5},
        {"ctq8", 0.3},
        {"cQq83", 0.5},
        {"cQq81", 1.},
        {"cQlMi", 1.},
        {"cbW", 5.},
        {"cpQ3", 5.},
        {"ctei", 5.},
        {"cQei", 5.},
        {"ctW", 1.},
        {"cpQM", 5.},
        {"ctlSi", 0.3},
        {"ctZ", 2.5},
        {"cQl3i", 1.},
        {"ctG", 0.6},
        {"cQq13", 0.3},
        {"cQq11", 1.},
        {"cptb", 5.},
        {"ctli", 0.1},
        {"ctp", 5.},
        {"cpt", 5.},
    };
    std::vector<std::string> wcnames;
    for( auto el: wcconfig ){ wcnames.push_back(el.first); }

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
    HistInfo histInfo_yield = HistInfo( "yield", "yield", 1, 0, 1 );
    histograms["yield"]["nominal"] = histInfo_yield.makeHist("yield_nominal");
    for( std::string wcname: wcnames ){
        histograms["yield"][wcname] = histInfo_yield.makeHist( "yield_"+wcname );
    }
    
    // do event loop
    for(long unsigned entry = 0; entry < nEvents; entry++){
        Event event = treeReader.buildEvent(entry,false,false,false,false,false,true);
	// get observable quantities
        float yield = 0.5;
	int njets = event.jetCollection().goodJetCollection().size();
	// get nominal weight
	double nominal_weight = 1; // use 1 since we will use relative weight below
	// loop over wilson coefficients and derive alternative weights
	std::map<std::string, double> wcweights;
	for( std::string wcname: wcnames ){
	    std::map<std::string, double> wcvalues = {{wcname, wcconfig.at(wcname)}};
	    double weight = event.eftInfo().relativeWeight(wcvalues);
	    wcweights[wcname] = weight;
	}
	// fill histograms
	histograms["njets"]["nominal"]->Fill(njets, nominal_weight);
        histograms["yield"]["nominal"]->Fill(yield, nominal_weight);
	for( std::string wcname: wcnames ){
	    histograms["njets"][wcname]->Fill(njets, wcweights[wcname]);
            histograms["yield"][wcname]->Fill(yield, wcweights[wcname]);
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
