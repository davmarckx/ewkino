#include "../Tools/interface/mergeAndRemoveOverlap.h"

// include c++ library classes
#include <string> 
#include <vector>
#include <iostream>

// include other parts of framework
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"


int main( int argc, char* argv[] ){

    // convert all input to std::string format for easier handling
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    // case 1: input and output directory provided on the command line
    // -> merge data files present in input directory (separately for the years)
    if( argc == 3 && !stringTools::stringContains( argvStr[2], ".root" ) ){
        
	// initializations
	const std::string input_directory = argvStr[1];
        const std::string output_directory = argvStr[2];
        const std::vector< std::string > dataIdentifiers = { 
	    "DoubleEG", "DoubleMuon", "MuonEG",
	    "SingleElectron", "SingleMuon", "EGamma",
	    "MET", "JetHT" };
        const std::vector< std::string > presentFiles = systemTools::listFiles( 
	    input_directory, "", ".root" );
	const std::vector< std::string > years = {
	    "Summer16PreVFP", "Summer16PostVFP", "Fall17", "Autumn18" };
	
	// loop over years
        for( const auto& year : years ){
            std::vector< std::string > filesToMerge;
	    // find files to merge
            for( const auto& fileName : presentFiles ){
		// filter on year
                if( !stringTools::stringContains( fileName, year ) ) continue;
                // filter out samples with "gen" (e.g. "genMET")
                if( stringTools::stringContains( fileName, "gen" ) ) continue;
                // make sure file corresponds to data
                bool identified = false;
                for( const auto& id : dataIdentifiers ){
                    if( stringTools::stringContains( fileName, id ) ){
                        identified = true;
                        break;
                    }
                }
                if( identified ){ filesToMerge.push_back( fileName ); }
            }
            
            // submit job to merge files 
            const std::string outputPath = stringTools::formatDirectoryName( 
		output_directory ) + "data_combined_" + year + ".root";
            std::string mergeCommand = "./combinePD " + outputPath;
            for( const auto& input : filesToMerge ){
                mergeCommand += ( " " + input );
            }
            systemTools::submitCommandAsJob( mergeCommand, 
		std::string( "combinePD_" ) + year + ".sh", "169:00:00" );
        }

	// return 0 if no errors occurred
	return 0;
    
    // case 2: list of output and input files similar to hadd structure
    // -> one merging job to merge input files into output file.
    } else if( argc > 2 ){
        std::string outputPath = argvStr[1];
        std::vector< std::string > inputFiles( argvStr.begin() + 2, argvStr.end() );
        mergeAndRemoveOverlap( inputFiles, outputPath );
	return 0;

    } else {
        std::cerr << argc - 1 << " command line arguments given,";
	std::cerr << " while at least 2 are expected." << std::endl;
        std::cerr << "Usage: ./combinePD < output_path >";
	std::cerr << " < space separated list of input files >" << std::endl;
        std::cerr << "OR: ./combinePD < input_directory containing data sample >";
	std::cerr << " < output directory where to put merged samples >" << std::endl;
        return 1;
    }
}
