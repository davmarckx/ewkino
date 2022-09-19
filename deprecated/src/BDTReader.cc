#include "../interface/BDTReader.h"

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::shared_ptr< std::map < std::string, float > >& varMap):
    variableMap(varMap), methodName(bdtName)
{
    reader.reset( new TMVA::Reader("!Color:!Silent") );

    //set all variables
    addVariables();

    //book method 
    reader->BookMVA(methodName, xmlFileName); 
}

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::map < std::string, float >& varMap):
    BDTReader(bdtName, xmlFileName, std::make_shared< std::map < std::string, float> >(varMap) ) {} 

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::vector<std::string>& varsToUse){
    std::map < std::string, float> varMap;
    //safety check that every element in the variables to use is in variableMap
    for(auto& varName : varsToUse){
        varMap[varName] = 0.;
    }
    *this = BDTReader(bdtName, xmlFileName, varMap);
}

void BDTReader::addVariables() const{
    for(auto& variable : *variableMap){
        reader->AddVariable( (const TString&) variable.first, &( (*variableMap)[variable.first] ) );
    }
}

float BDTReader::computeBDT(const std::map < std::string, float>& varMap){
    for(auto it = varMap.cbegin(); it != varMap.cend(); ++it){
        (*variableMap)[it->first] = it->second;
    }
    //retrieve bdt output and return
    return ( reader->EvaluateMVA(methodName) );
}
