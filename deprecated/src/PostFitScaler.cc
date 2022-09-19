#include "../interface/PostFitScaler.h"

//include c++ library classes 
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cmath>

//include other parts of code 
#include "../interface/systemTools.h"

PostFitScaler::PostFitScaler( const std::string& inputFileName ){

	//read input file
	std::ifstream fileStream( inputFileName );   

	//read each line of the input file 
	std::string line;

	//skip first line 
	std::getline( fileStream, line ); 

    //check whether txt file has the correct format, it should atleast have four columns
    if( systemTools::numberOfColumnsInFile( inputFileName ) < 4 ){
        std::cerr << "Error in PostFitScaler constructor : input txt file has less than 4 columns and can not be processed. Yields will be left empty!" << std::endl;
        return;
    } 

	while( std::getline( fileStream, line ) ){
		
        //values to be read
        std::string channel, process;
        double preFitYield, postFitYield; 

		//split line and read each part 
		std::istringstream lineStream( line );
        lineStream >> channel >> process >> preFitYield >> postFitYield; 
    
        postAndPreFitYields[process].push_back( {preFitYield, postFitYield} );
        
	}
}

std::pair< std::string, size_t > PostFitScaler::findProcessAndBin( const double preFitYield ) const{
    std::string process;
    size_t bin;

    //find minimum difference between all stored pre fit yields and the argument passed 
    double minDifference = std::numeric_limits<double>::max();
    
    for( const auto& mapEntry : postAndPreFitYields ){
        const auto& yieldVector = mapEntry.second;
        for( size_t b = 0; b < yieldVector.size(); ++b ){
            double diff = fabs( preFitYield - yieldVector[b].first );
            if( diff < minDifference ){                    
                minDifference = diff;
                bin = b;
                process = mapEntry.first;
            }
        }
    }

    return {process, bin};
}

size_t PostFitScaler::findBinForProcess( const std::string& process, const double preFitYield) const{

    //find yields for given process 
    const auto& mapIt = postAndPreFitYields.find( process );

    //make sure the process is present in the input file
    if( mapIt == postAndPreFitYields.cend() ){
        std::cerr << "Error in PostFitScaler::findBinForProcess : " << process << " does not correspond to any process read from the input file!" << std::endl;
    }

    //find minimum difference between all stored pre fit yields for this process and the argument passed 
    double minDifference = std::numeric_limits<double>::max();

    size_t bin;

    const auto& yieldVector = mapIt->second;
    for( size_t b = 0; b < yieldVector.size(); ++b ){
        double diff = fabs( preFitYield - yieldVector[b].first );
        if( diff < minDifference ){
            minDifference = diff;
            bin = b;
        }
    }
    return bin;
}

double PostFitScaler::postFitYield( const size_t bin, const std::string& process) const{
    const auto& mapIt = postAndPreFitYields.find( process );
    const auto& yieldVector = mapIt->second;
    const auto& yieldPair = yieldVector[ bin ];
    return yieldPair.second;

}

double PostFitScaler::postFitYield( const double preFitYield ) const{
    std::pair< std::string, size_t> processAndBin = findProcessAndBin( preFitYield );
    return postFitYield( processAndBin.second, processAndBin.first);
}

double PostFitScaler::postFitScaling( const double preFitYield ) const{
    double newYield = postFitYield( preFitYield );
    return ( newYield / preFitYield );
}

double PostFitScaler::postFitYield( const std::string& process, const double preFitYield) const{
    size_t bin = findBinForProcess( process, preFitYield );
    return postFitYield( bin, process); 
}

double PostFitScaler::postFitScaling( const std::string& process, const double preFitYield) const{
    double newYield = postFitYield( process, preFitYield );
    return ( newYield / preFitYield );
}
