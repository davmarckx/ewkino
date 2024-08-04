/*
Calculate and store b-tagging scale factor normalization factors
Update: normalization factors are capped off at 10 (11/05/24)
*/

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <fstream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include b-tagging reweighter
#include "../../weights/interface/ReweighterBTagShape.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/Sample.h"

// include analysis tools
#include "../eventselection/interface/eventSelections.h"

// include dedicated tools
#include "interface/bTaggingTools.h"


void writeBTagNorms(
	const std::string& inputDirectory,
	const std::string& sampleList,
	int sampleIndex,
	unsigned long nEvents,
	const std::string& outputDirectory,
	const std::vector<std::string>& event_selections,
	const std::vector<std::string>& variations_all ){
    
    // initialize TreeReader from input file
    std::cout<<"initializing TreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    Sample sample = treeReader.currentSample();
    std::vector<Sample> samples;
    samples.push_back(sample);

    // clean the variations if they contain flavor splits
    std::vector<std::string> flavorvariations = {};
    std::vector<std::string> variations = {};
    bool hasJECFlavor = false;
    for( std::string var: variations_all ){
        if(var.find("_flavor")!= std::string::npos){ 
            hasJECFlavor = true;
            flavorvariations.push_back(var);
        }
        else{variations.push_back(var);}
    } 


    // make the b-tag shape reweighter
    // step 1: set correct csv file
    std::string bTagSFFileName = "bTagReshaping_unc_"+year+".csv";
    std::string sfFilePath = "weightFilesUL/bTagSF/"+bTagSFFileName;
    std::string weightDirectory = "../../weights";
    // step 2: set other parameters
    std::string flavor = "all";
    std::string bTagAlgo = "deepFlavor";
    // step 3: make the reweighter
    std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
        weightDirectory, sfFilePath, flavor, bTagAlgo, variations, samples );

    // initialize the output maps (this is done for all variations)
    std::map< std::string, std::map< std::string, std::map< int, double >>> averageOfWeights;
    std::map< std::string, std::map<std::string, std::map< int, int >>> nEntries;
    for( std::string es: event_selections ){
	averageOfWeights[es]["central"][0] = 0.;
	nEntries[es]["central"][0] = 0;
	for( std::string var: variations_all ){
	    averageOfWeights[es]["up_"+var][0] = 0.;
	    averageOfWeights[es]["down_"+var][0] = 0.;
	    nEntries[es]["up_"+var][0] = 0;
	    nEntries[es]["down_"+var][0] = 0;
	}
    }

    // determine number of entries to run over
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents>0 && nEvents<numberOfEntries ){
	if( !treeReader.isData() ){
	    // loop over a smaller number of entries
	    std::cout << "limiting number of entries to " << nEvents << std::endl;
            numberOfEntries = nEvents;
        }
    }

    // do event loop
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// build event
	Event event = treeReader.buildEvent(entry, false, false, false, true);
        
	// loop over event selections
	for(std::string es: event_selections){
	    if(!passES(event, es, "tight", "nominal", false)) continue;

	    // add nominal b-tag reweighting factors
	    double btagreweight = reweighterBTagShape->weight( event );
	    int njets = reweighterBTagShape->getNJets( event );
            if(njets>10){njets=10;}
	    if( btagreweight > 1e-12 ){
	    if( nEntries.at(es).at("central").find(njets)==nEntries.at(es).at("central").end() ){
		averageOfWeights.at(es).at("central")[njets] = btagreweight;
		nEntries.at(es).at("central")[njets] = 1;
	    } else{
		averageOfWeights.at(es).at("central").at(njets) += btagreweight;
		nEntries.at(es).at("central").at(njets) += 1;
	    }}

	    // add varied b-tag reweighting factors (flavor split variations are treated differently)
	    for( std::string var: variations ){ 
		// get up weight
		double btagup = reweighterBTagShape->weightUp(event, var);
		int njetsup = reweighterBTagShape->getNJets(event, "up_"+var);
                if(njetsup>10){njetsup=10;}
		if( btagup > 1e-12 ){
		if( nEntries.at(es).at("up_"+var).find(njetsup)==nEntries.at(es).at("up_"+var).end() ){
		    averageOfWeights.at(es).at("up_"+var)[njetsup] = btagup;
		    nEntries.at(es).at("up_"+var)[njetsup] = 1;
		} else{
		    averageOfWeights.at(es).at("up_"+var).at(njetsup) += btagup;
		    nEntries.at(es).at("up_"+var).at(njetsup) += 1;
		}}
		// get down weight
		double btagdown = reweighterBTagShape->weightDown(event, var);
		int njetsdown = reweighterBTagShape->getNJets(event, "down_"+var);
                if(njetsdown>10){njetsdown=10;}
		if( btagdown > 1e-12 ){
		if( nEntries.at(es).at("down_"+var).find(njetsdown)==nEntries.at(es).at("down_"+var).end() ){
		    averageOfWeights.at(es).at("down_"+var)[njetsdown] = btagdown;
		    nEntries.at(es).at("down_"+var)[njetsdown] = 1;
		} else{
		    averageOfWeights.at(es).at("down_"+var).at(njetsdown) += btagdown;
		    nEntries.at(es).at("down_"+var).at(njetsdown) += 1;
		}}
	    } // end loop over variations
            for( std::string var: flavorvariations ){
               // get some info of split JEC
               std::string modvar = stringTools::removeOccurencesOf(var,"jes");
               unsigned long flav = std::strtoul(&modvar.back(), NULL, 0);

               // get up weight
               modvar = modvar.substr(0, modvar.size()-8)+"Up";
               double btagup = reweighterBTagShape->weightJecVar_FlavorFilter(event, modvar ,flav);
               
               int njetsup = reweighterBTagShape->getNJets(event, "up_"+var);
               if(njetsup>10){njetsup=10;}
               if( btagup > 1e-12 ){
               if( nEntries.at(es).at("up_"+var).find(njetsup)==nEntries.at(es).at("up_"+var).end() ){
                    averageOfWeights.at(es).at("up_"+var)[njetsup] = btagup;
                    nEntries.at(es).at("up_"+var)[njetsup] = 1;
               } else{
                    averageOfWeights.at(es).at("up_"+var).at(njetsup) += btagup;
                    nEntries.at(es).at("up_"+var).at(njetsup) += 1;
               }}
               // get down weight
               modvar = modvar.substr(0, modvar.size()-2)+"Down";
               double btagdown = reweighterBTagShape->weightJecVar_FlavorFilter(event, modvar ,flav);
         
               int njetsdown = reweighterBTagShape->getNJets(event, "down_"+var);
               if(njetsdown>10){njetsdown=10;}
               if( btagdown > 1e-12 ){
               if( nEntries.at(es).at("down_"+var).find(njetsdown)==nEntries.at(es).at("down_"+var).end() ){
                    averageOfWeights.at(es).at("down_"+var)[njetsdown] = btagdown;
                    nEntries.at(es).at("down_"+var)[njetsdown] = 1;
               } else{
                    averageOfWeights.at(es).at("down_"+var).at(njetsdown) += btagdown;
                    nEntries.at(es).at("down_"+var).at(njetsdown) += 1;
               }}
               
            }
	} // end loop over event selections
    } // end loop over events

    // divide sum by number to get average
    for( std::string es: event_selections ){
	for( std::map<int,int>::iterator it = nEntries.at(es).at("central").begin(); 
	    it != nEntries.at(es).at("central").end(); ++it ){
	    averageOfWeights.at(es).at("central").at(it->first) /= it->second;
	    if( it->second==0 ){ averageOfWeights.at(es).at("central").at(it->first) = 1; }
	}
	for( std::string var: variations_all ){
	    for( std::map<int,int>::iterator it = nEntries.at(es).at("up_"+var).begin(); 
		it != nEntries.at(es).at("up_"+var).end(); ++it ){
		averageOfWeights.at(es).at("up_"+var).at(it->first) /= it->second;
		if( it->second==0 ){ averageOfWeights.at(es).at("up_"+var).at(it->first) = 1; }
	    }
	    for( std::map<int,int>::iterator it = nEntries.at(es).at("down_"+var).begin();
		it != nEntries.at(es).at("down_"+var).end(); ++it ){
		averageOfWeights.at(es).at("down_"+var).at(it->first) /= it->second;
		if( it->second==0 ){ averageOfWeights.at(es).at("down_"+var).at(it->first) = 1; }
	    }
	}
    }

    // make output file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += stringTools::replace(inputFileName, ".root", ".txt");
    std::vector<std::string> lines = bTaggingTools::mapToText(averageOfWeights);
    std::ofstream outFile(outputFilePath, std::ofstream::out);
    if( outFile.is_open() ){
	for( std::string line: lines ){
	    outFile << line << "\n";
	}
	outFile.close();
    } else{
	std::string msg = "ERROR: output file could not be opened.";
	throw std::runtime_error(msg);
    }
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc != 8 ){
	std::cerr << "ERROR: need following command line arguments:" << std::endl;
        std::cerr << " - input directory" << std::endl;
        std::cerr << " - sample list" << std::endl;
        std::cerr << " - sample index" << std::endl;
        std::cerr << " - output directory" << std::endl;
        std::cerr << " - event selections" << std::endl;
        std::cerr << " - variations" << std::endl;
	std::cerr << " - number of events" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    std::string& variation = argvStr[6];
    std::vector<std::string> variations;
    if( variation!="none" ) variations = stringTools::split(variation,",");
    unsigned long nevents = std::stoul(argvStr[7]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - variation: " << variation << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;

    // fill the histograms
    writeBTagNorms( input_directory, sample_list, sample_index, nevents,
		    output_directory,
		    event_selections, variations );
    std::cerr << "###done###" << std::endl;
    return 0;
}
