
//include c++ library classes 
#include <string>

//include other parts of framework
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/SusyScan.h"
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/Sample.h"


bool plotsExist( const std::string& baseDirectory, const std::string& year, const std::string& region, const std::string& modelName, const double massSplitting ){

    //convert deltaM to string and cut off digits after the comma
    std::string deltaM = std::to_string( massSplitting );
    deltaM = stringTools::split( deltaM, "." ).front();
        
    //check if the required neural network logarithmic plot is present
    std::string expectedDirectory = stringTools::formatDirectoryName( baseDirectory ) + year + "/" + region + "/" + modelName + "/deltaM_" + deltaM;
    std::string plotName = ( "neuralNet_" + region + "_" + modelName + "_" + deltaM + "_" + year + "_log.pdf" );
    std::string plotPath = stringTools::formatDirectoryName( expectedDirectory ) + plotName;
    
    return systemTools::fileExists( plotPath );
}


bool datatacardsExist( const std::string& baseDirectory, const std::string& year, const std::string& modelName, const double massSplitting ){
	std::string deltaM = std::to_string( massSplitting );
	deltaM = stringTools::split( deltaM, "." ).front();
    std::string expectedDirectory = stringTools::formatDirectoryName( baseDirectory ) + year + "/" + modelName + "/" + deltaM;

	//if the directory exist check that there are nonempty files
    if( systemTools::directoryExists( expectedDirectory ) ){
		std::vector< std::string > fileList = systemTools::listFiles( expectedDirectory, "neural", "txt" );
        if( !fileList.empty() ){
            return true;
        }
	}
	return false;
}


std::vector< unsigned > getMassSplittings( const std::string& modelName, const std::string& year, const std::string& sampleDirectoryPath ){
    analysisTools::checkYearString( year );

    std::string sampleListFilePath = ( "sampleLists/samples_" + modelName + "_" + year + ".txt" );
    auto sampleList = readSampleList( sampleListFilePath, sampleDirectoryPath );

    //use sampleList to make a SusyScan containing all the available mass splittings 
    SusyScan susyScan;
    for( const auto& sample : sampleList ){
        if( sample.isNewPhysicsSignal() ){
            susyScan.addScan( sample );
        }
    }
    return susyScan.massSplittings();
}


//void resubmitMissingMassSplittingsSearchRegions( const std::string& datacardDirectory, const std::string& plotDirectory, const std::string& modelName, const std::string& year, const std::string& sampleDirectoryPath ){
void resubmitMissingMassSplittingsSearchRegions( const std::string& datacardDirectory, const std::string& plotDirectory, const std::string& year, const std::string& modelName, const std::string& sampleDirectoryPath ){

    auto massSplittingList = getMassSplittings( modelName, year, sampleDirectoryPath );
    for( auto massSplitting : massSplittingList ){

		//if either the plots or datacards correspoding to this mass splitting are not present, relaunch the job
        bool plots_exist = plotsExist( plotDirectory, year, "SRA", modelName, massSplitting );
        bool datacards_exist = datatacardsExist( datacardDirectory, year, modelName, massSplitting );

        if( !( plots_exist && datacards_exist ) ){
			std::string deltaM = stringTools::split( std::to_string( massSplitting ), "." ).front();

            //THIS PART MUST BE GENERALIZED SO IT WORKS FOR OTHER EXECUTABLES
			std::string commandString = "./Ewkino_3lOSSF " + modelName + " " + deltaM + " " + year;
			std::string scriptName = "ewkino_" + modelName + "_" + deltaM + "_" + year + "_retry.sh";
			systemTools::submitCommandAsJob( commandString, scriptName, "100:00:00", "", 2 );	
		}
    }
}


void resubmitMissingMassSplittingsControlRegions( const std::string& plotDirectory, const std::string& year, const std::string& modelName, const std::string& controlRegion, const std::string& sampleDirectoryPath ){

    auto massSplittingList = getMassSplittings( modelName, year, sampleDirectoryPath );
    for( auto massSplitting : massSplittingList ){
        
        //if the plots corresponding to this mass splitting are not present, relaunch the job
        bool plots_exist = plotsExist( plotDirectory,  year, controlRegion, modelName, massSplitting );
        if( ! plots_exist ){
			std::string deltaM = stringTools::split( std::to_string( massSplitting ), "." ).front();

            //THIS PART MUST BE GENERALIZED SO IT WORKS FOR OTHER EXECUTABLES
            std::string commandString = "./controlRegions " + modelName + " " + deltaM + " " + year + " " + controlRegion;
            std::string scriptName = controlRegion + "_" + modelName + "_" + deltaM + "_" + year + ".sh";
            systemTools::submitCommandAsJob( commandString, scriptName, "100:00:00", "", 2 );
        }
    }
}



int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    const std::string sampleDirectoryPath = "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino/";

    if( argc == 1 ){
        const std::vector< std::string > years = { "2016", "2017", "2018" };
        const std::vector< std::string > models = { "TChiWZ", "TChiSlepSnu_x0p95", "TChiSlepSnu_x0p5", "TChiSlepSnu_x0p05" };
        const std::vector< std::string > controlRegions = { "WZ", "TTZ", "NP", "XGamma" };

        //resubmit both control- and search region plots
        if( argc == 1 ){
            for( const auto& year : years ){
                for( const auto& model : models ){
                    resubmitMissingMassSplittingsSearchRegions( "datacards/", "plots/ewkino/", year, model, sampleDirectoryPath );
                }
            }
            for( const auto& year : years ){
                for( const auto& model : models ){
                    for( const auto& controlRegion : controlRegions ){
                        resubmitMissingMassSplittingsControlRegions( "plots/ewkino/", year, model, controlRegion, sampleDirectoryPath );
                    }
                }
            }
        }
    } else {
        return 1;
    }
    return 0;
}
