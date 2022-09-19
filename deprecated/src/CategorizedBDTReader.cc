/*
implementation of Categorized BDT reader
*/

#include "../interface/CategorizedBDTReader.h"
#include "../interface/analysisTools.h"


CategorizedBDTReader::CategorizedBDTReader(const std::string& bdtName, const std::string& xmlFileDirectory, const std::shared_ptr< Category >& cat, const std::map < std::string, float>& varMap):
	category(cat), methodName(bdtName)
{
	//map shared ptr based on varMap to avoid copies
	std::shared_ptr< std::map < std::string, float> > varMapPtr = std::make_shared< std::map < std::string, float > >( varMap);
	for(size_t c = 0; c < category->size(); ++c){
		bdtReaders.push_back( BDTReader(bdtName, tools::formatedDirectoryName(xmlFileDirectory) + mvaMethodName(c) + ".xml" ,varMapPtr) );
	}
}
	
std::string CategorizedBDTReader::mvaMethodName(const size_t categoryIndex) const{
    return ( category->name(categoryIndex) + "_" + methodName );
}

//compute mva value for given category's mva
float CategorizedBDTReader::computeBDT(const size_t categoryIndex, const std::map < std::string, float>& varMap){
    return bdtReaders[categoryIndex].computeBDT(varMap);
}

float CategorizedBDTReader::computeBDT(const std::vector<size_t>& categoryIndices, const std::map < std::string, float>& varMap){
    size_t categoryIndex = category->getIndex(categoryIndices);
    return computeBDT(categoryIndex, varMap);
}
