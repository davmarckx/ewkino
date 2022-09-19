/*
multi category bdt reader
*/

#ifndef CategorizedBDTReader_H
#define CategorizedBDTReader_H

//include other parts of code 
#include "../interface/BDTReader.h"
#include "../interface/Category.h"

//include c++ library functions
#include <memory>
#include <vector>
#include <map>
#include <string>

//include ROOT classes

class CategorizedBDTReader{
     public:
        CategorizedBDTReader(const std::string&, const std::string&, const std::shared_ptr<Category>&, const std::map< std::string, float>&);
        CategorizedBDTReader(const std::string&, const std::string&, const Category&, const std::map < std::string, float >&);
        CategorizedBDTReader(const std::string&, const std::string&, const std::vector < std::vector < std::string > >&, const std::map < std::string, float >&);

        //get the BDT output for this category's BDT 
        float computeBDT(const std::vector< size_t > &, const std::map < std::string, float>& );

    private:
        std::shared_ptr<Category> category;
        std::vector<BDTReader> bdtReaders;
        std::string methodName;

        //get name of method (xml file) for given category
        std::string mvaMethodName(const size_t) const;

        //get the BDT output for this category's BDT 
        float computeBDT(const size_t categoryIndex, const std::map < std::string, float>& );
};
#endif
