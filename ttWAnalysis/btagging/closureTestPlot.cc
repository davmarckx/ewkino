/*
Closure test for b-tagging normalization
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

void closureTestPlot(
	const std::string& inputDirectory,
	const std::string& sampleList,
	int sampleIndex,
	unsigned long nEvents,
	const std::string& txtInputDirectory,
	const std::vector<std::string>& event_selections,
	const std::vector<std::string>& variations,
        const std::string& outputDirectory ){
    
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

    // read txt file with normalization factors
    std::string txtInputFile = stringTools::formatDirectoryName( txtInputDirectory );
    std::string findstr = ".root";
    std::string replacestr = ".txt";
    txtInputFile += inputFileName.replace(inputFileName.find(".root"),findstr.length(),replacestr);
    std::map< std::string, std::map< std::string, std::map< int, double >>> weightMap;
    std::vector<std::string> variationsToRead = {"central"};
    for( std::string var: variations ){ 
	variationsToRead.push_back("up_"+var);
	variationsToRead.push_back("down_"+var);
    }
    std::cout<<txtInputFile;
    weightMap = bTaggingTools::textToMap( txtInputFile, event_selections, variationsToRead );

    // make the b-tag shape reweighter
    // step 1: set correct csv file
    std::string bTagSFFileName = "bTagReshaping_unc_"+year+".csv";
    std::string sfFilePath = "weightFilesUL/bTagSF/"+bTagSFFileName;
    std::string weightDirectory = "../../weights";
    // step 2: set other parameters
    std::string flavor = "all";
    std::string bTagAlgo = "deepFlavor";
    //std::vector<std::string> variations = {
    //	"hf","lf","hfstats1","hfstats2",
    //	"lfstats1","lfstats2","cferr1","cferr2" };
    // step 3: make the reweighter
    std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
        weightDirectory, sfFilePath, flavor, bTagAlgo, variations, samples );

    // initialize the output maps
    /*std::map< std::string, std::map< std::string, std::map< int, double >>> averageOfWeights;
    std::map< std::string, std::map<std::string, std::map< int, int >>> nEntries;
    for( std::string es: event_selections ){
	averageOfWeights[es]["central"][0] = 0.;
	nEntries[es]["central"][0] = 0;
	for( std::string var: variations ){
	    averageOfWeights[es]["up_"+var][0] = 0.;
	    averageOfWeights[es]["down_"+var][0] = 0.;
	    nEntries[es]["up_"+var][0] = 0;
	    nEntries[es]["down_"+var][0] = 0;
	}
    }*/
    

    //make output tree structure
    std::string outputdir = "blackJackAndHookers";
    std::string treename = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );          //TDB
    outputFilePath += inputFileName.replace(inputFileName.find(".txt"),replacestr.length(),findstr);                                                           //TBD
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );

    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );
    std::shared_ptr< TTree > outputTree( std::make_shared< TTree >(
                                            treename.c_str(), treename.c_str() ) );

    Float_t _weight = 0; // generator weight scaled by cross section and lumi
    Float_t _nonormreweight = 0; // total weight, including reweighting and fake rate
    Float_t _reweight = 1; // total reweighting factor
    Float_t _HT = 0;
    Float_t _nJets = 0;
    Float_t _jetPtLeading = 0.;
    Float_t _jetPtSubLeading = 0.;
    Float_t _Mjj_max = 0;
    Float_t _pTjj_max = 0;
    Float_t _dRl1jet = 99.;


    outputTree.get()->Branch("_weight", &_weight, "_weight/F");
    outputTree.get()->Branch("_nonormreweight", &_nonormreweight, "_nonormreweight/F");
    outputTree.get()->Branch("_reweight", &_reweight, "_reweight/F");
    outputTree.get()->Branch("_HT", &_HT, "_HT/F");
    outputTree.get()->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_jetPtLeading", &_jetPtLeading, "_jetPtLeading/F");
    outputTree->Branch("_jetPtSubLeading", &_jetPtSubLeading, "_jetPtSubLeading/F");
    outputTree->Branch("_dRl1jet", &_dRl1jet, "_dRl1jet/F");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/F");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/F");


    // determine number of entries to run over
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){
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
        _Mjj_max = 0;
        _pTjj_max = 0;
        _dRl1jet = 99.;    
	// loop over event selections
	for(std::string es: event_selections){
	    if(!passES(event, es, "tight", "nominal", false)) continue;      // event selection without the bjet selection

	    // set  the correct normalization
	    reweighterBTagShape->setNormFactors( sample, weightMap[es] );

	    // add nominal b-tag reweighting factors
	    _reweight = reweighterBTagShape->weight( event );
            _nonormreweight = reweighterBTagShape->weightNoNorm( event );
            _weight = event.weight();
            JetCollection jetcollection = event.getJetCollection("nominal");
            LeptonCollection lepcollection = event.leptonCollection();

            _HT = jetcollection.scalarPtSum();
            _nJets = jetcollection.size();
            for(JetCollection::const_iterator jIt = jetcollection.cbegin();
              jIt != jetcollection.cend(); jIt++){
              Jet& jet = **jIt;
              if(deltaR(lepcollection[0],jet)<_dRl1jet) _dRl1jet = deltaR(lepcollection[0],jet);
            }
            jetcollection.sortByPt();
            _jetPtLeading = jetcollection[0].pt();
            _jetPtSubLeading = jetcollection[1].pt();

            for(JetCollection::const_iterator jIt = jetcollection.cbegin();
                jIt != jetcollection.cend(); jIt++){
                Jet& jet = **jIt;
                for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != jetcollection.cend(); jIt2++){
                    Jet& jet2 = **jIt2;
                    if((jet+jet2).mass()>_Mjj_max) _Mjj_max = (jet+jet2).mass();
                    if((jet+jet2).pt() >_pTjj_max) _pTjj_max = (jet+jet2).pt();
                }
            }
    
 
            outputTree->Fill();
	} // end loop over event selections
    } // end loop over events
    outputFilePtr->cd( outputdir.c_str() );
    outputTree->Write("", BIT(2) );
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc != 9 ){
	std::cerr << "ERROR: need following command line arguments:" << std::endl;
        std::cerr << " - input directory" << std::endl;
        std::cerr << " - sample list" << std::endl;
        std::cerr << " - sample index" << std::endl;
        std::cerr << " - txt input directory" << std::endl;
        std::cerr << " - event selections" << std::endl;
        std::cerr << " - variations" << std::endl;
	std::cerr << " - number of events" << std::endl;
        std::cerr << " - output directory" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& txt_input_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    std::string& variation = argvStr[6];
    std::vector<std::string> variations;
    if( variation!="none" ) variations = stringTools::split(variation,",");
    unsigned long nevents = std::stoul(argvStr[7]);
    std::string& output_directory = argvStr[8];

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - txt input directory: " << txt_input_directory << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - variation: " << variation << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;
    std::cout << "  - output directory: " << output_directory << std::endl;

    // fill the histograms
    closureTestPlot( input_directory, sample_list, sample_index, nevents,
		    txt_input_directory,
		    event_selections, variations, output_directory );

    std::cerr << "###done###" << std::endl;
    return 0;
}
