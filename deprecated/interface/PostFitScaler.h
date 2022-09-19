/*
Class that extracts post-fit normalizations from Combine's output and retrieves the scaling 
*/

#ifndef PostFitScaler_H
#define PostFitScaler_H

//include c++ library classes 
#include <map>
#include <vector>
#include <string>
#include <utility>

class PostFitScaler{
    public: 
        PostFitScaler( const std::string& inputFileName );

        double postFitYield( const double preFitYield ) const;
        double postFitScaling( const double preFitYield ) const;
        double postFitYield( const std::string& process, const double preFitYield ) const;
        double postFitScaling( const std::string& process, const double preFitYield ) const;

    private:
        std::map< std::string, std::vector< std::pair < double, double  > > > postAndPreFitYields;

        //retrieve which process and bin corresponds to a certain pre fit yield
        std::pair< std::string, size_t> findProcessAndBin( const double preFitYield ) const;

        //retrieve bin corresponding to pre fit yield for given process
        size_t findBinForProcess( const std::string& process, const double preFitYield ) const;

        //return postFitYield given process and vector index
        double postFitYield( const size_t bin, const std::string& process) const;
};
#endif
