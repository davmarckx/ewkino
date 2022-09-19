/*
class to facilitate extraction of bdt output from xml files
*/

#ifndef BDTReader_H
#define BDTReader_H

//include other parts of code 

//include c++ library functions
#include <memory>
#include <vector>
#include <map>
#include <string>

//include ROOT classes
#include "TMVA/Reader.h"

class BDTReader{
    public:
        BDTReader(const std::string&, const std::string&, const std::shared_ptr < std::map < std::string, float> >&); 
        BDTReader(const std::string&, const std::string&, const std::map < std::string, float>&); 
        BDTReader(const std::string&, const std::string&, const std::vector<std::string>&);

        //get the BDT output for this category's BDT 
        float computeBDT(const std::map < std::string, float>& );

    private:
        std::shared_ptr< TMVA::Reader > reader;
        std::shared_ptr< std::map < std::string, float> > variableMap;
        std::string methodName;

        //add variables to bdt 
        void addVariables() const;
};
#endif 
