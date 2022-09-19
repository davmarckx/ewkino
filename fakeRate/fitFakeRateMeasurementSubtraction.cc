// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// include framework
#include "interface/fakeRateTools.h"
#include "../plotting/interface/plotCode.h"
#include "../plotting/interface/tdrStyle.h"
#include "../Tools/interface/systemTools.h"


int main( int argc, char* argv[] ){

    // read command-line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 6 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 5 are needed."<<std::endl;
        std::cerr<<"usage: ./fitFakeRateMeasurementSubtraction use_mT flavour year";
	std::cerr<<" doInputPlot doInputSave"<<std::endl;
        return 1;
    }
    std::string year = argvStr[3];
    std::string flavor = argvStr[2];
    const bool use_mT = (argvStr[1]=="True" || argvStr[1]=="true");
    const bool doInputPlot = (argvStr[4]=="True" || argvStr[4]=="true");
    const bool doInputSave = (argvStr[5]=="True" || argvStr[5]=="true");

    // initializations
    const double maxFitValue = 40; // 40
    setTDRStyle();

    // read input file
    std::string instanceName = flavor + "_" + year + "_" + ( use_mT ? "mT" : "met" );
    std::string file_name = "fakeRateMeasurement_data_" + instanceName + "_histograms.root";
    TFile* measurement_filePtr = TFile::Open( file_name.c_str() );

    // produce fake rate map
    std::shared_ptr< TH2D > frMap = fakeRate::produceFakeRateMap_cut( measurement_filePtr, 
					maxFitValue, doInputPlot, doInputSave );
    measurement_filePtr->Close();

    // plot and save the fake rate map
    systemTools::makeDirectory("fakeRateMaps");
    std::string mapName = "fakeRateMaps/fakeRateMap_data_"+instanceName;
    std::string title = "Fake rate map for " + year + " " + flavor + "s";
    plot2DHistogram( frMap.get(), (mapName+".pdf").c_str(), title.c_str(), "colztexte", 1.5 );
    TFile* writeFile = TFile::Open( (mapName+".root" ).c_str(), "RECREATE" );
    frMap->Write( ("fakeRate_" + flavor + "_" + year ).c_str() );
    writeFile->Close();
    return 0;
}
