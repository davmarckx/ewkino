// Investigate magnitude of pdf normalization variations

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"

// include other parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/SampleCrossSections.h"


void fillPdfNormHistogram(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputFile ){

    // initialize TreeReader from input file
    std::cout << "initializing TreeReader and setting to sample n. " << sampleIndex << "..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(unsigned int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // initializations
    std::shared_ptr< SampleCrossSections > xsecs;
    unsigned numberOfScaleVariations = 0;
    unsigned numberOfPdfVariations = 0;
    std::vector< double > qcdScalesXSecRatios;
    std::vector<double> pdfXSecRatios;

    // build first event to get number of variations
    std::cout << "building event..." << std::endl;
    Event event = treeReader.buildEvent(0);
    numberOfScaleVariations = event.generatorInfo().numberOfScaleVariations();
    numberOfPdfVariations = event.generatorInfo().numberOfPdfVariations();

    // initialize histograms
    std::shared_ptr<TH1D> qcdScalesHist;
    qcdScalesHist = std::make_shared<TH1D>(
	"qcdScales", "qcdScales", numberOfScaleVariations, -0.5, numberOfScaleVariations-0.5);
    qcdScalesHist->SetDirectory(0);
    qcdScalesHist->Sumw2();
    std::shared_ptr<TH1D> pdfHist;
    pdfHist = std::make_shared<TH1D>(
        "pdf", "pdf", numberOfPdfVariations, -0.5, numberOfPdfVariations-0.5);
    pdfHist->SetDirectory(0);
    pdfHist->Sumw2();
    
    // build modified cross-section collection
    xsecs = std::make_shared<SampleCrossSections>( treeReader.currentSample() );

    // read QCD scale norm variations 
    std::cout << "finding available QCD scale variations..." << std::endl;
    if( numberOfScaleVariations==9 ){
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5() );
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_2() );
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1() );
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_1() );
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_2() );
	qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5() );
	for(unsigned int i=0; i<numberOfScaleVariations; i++){
	    qcdScalesHist->SetBinContent(i+1, qcdScalesXSecRatios[i]);
	    qcdScalesHist->SetBinError(i+1, 0);
	}
    }

    // read PDF norm variations
    std::cout << "finding available PDF variations..." << std::endl;
    for(unsigned i=0; i<numberOfPdfVariations; i++){ 
	pdfXSecRatios.push_back( xsecs.get()->crossSectionRatio_pdfVar(i) );
    }
    for(unsigned int i=0; i<numberOfPdfVariations; i++){
        pdfHist->SetBinContent(i+1, pdfXSecRatios[i]);
        pdfHist->SetBinError(i+1, 0);
    }

    // write output file
    TFile* outputFilePtr = TFile::Open( outputFile.c_str() , "RECREATE" );
    qcdScalesHist->Write();
    pdfHist->Write();
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 4;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: pdfnorm.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input directory" << std::endl;
	std::cerr << "sample list" << std::endl;
	std::cerr << "sample index" << std::endl;
	std::cerr << "output file" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_file = argvStr[4];

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output file: " << output_file << std::endl;

    // fill the histograms
    fillPdfNormHistogram( input_directory, sample_list, sample_index, output_file );

    std::cerr << "###done###" << std::endl;
    return 0;
}
