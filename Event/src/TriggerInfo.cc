#include "../interface/TriggerInfo.h"

//include c++ library classes
#include <iostream>
#include <stdexcept>

//include other parts of framework
#include "../../Tools/interface/stringTools.h"


//remove leading _ from trigger and filter names 
std::string cleanName( const std::string& name ){
    std::string ret( name );
    if( stringTools::stringStartsWith( name, "_" ) ){
        ret.erase(0, 1);
    }
    return ret;
}


TriggerInfo::TriggerInfo( const TreeReader& treeReader, const bool readIndividualTriggers, const bool readIndividualMetFilters ) :
    _passTriggers_e( treeReader._passTrigger_e ),
    _passTriggers_m( treeReader._passTrigger_m ),
    _passTriggers_ee( treeReader._passTrigger_ee ),
    _passTriggers_em( treeReader._passTrigger_em ),
    _passTriggers_et( treeReader._passTrigger_et ),
    _passTriggers_mm( treeReader._passTrigger_mm ),
    _passTriggers_mt( treeReader._passTrigger_mt ),
    _passTriggers_eee( treeReader._passTrigger_eee ),
    _passTriggers_eem( treeReader._passTrigger_eem ),
    _passTriggers_emm( treeReader._passTrigger_emm ),
    _passTriggers_mmm( treeReader._passTrigger_mmm ),
    _passTriggers_FR( treeReader._passTrigger_FR ),
    _passTriggers_FR_iso( treeReader._passTrigger_FR_iso ),
    _passMetFilters( treeReader._passMETFilters )
{
    if( treeReader.containsRefTriggerInfo() ){
	_passTriggers_ref = treeReader._passTrigger_ref;
    }
    if( readIndividualTriggers ){
        for( const auto& trigger : treeReader._triggerMap ){
            individualTriggerMap.insert( { cleanName( trigger.first ), trigger.second } );
        }
    }
    if( readIndividualMetFilters ){
        for( const auto& filter : treeReader._MetFilterMap ){
            individualMetFilterMap.insert( { cleanName( filter.first ), filter.second } );
        }   
    }
}


bool passTriggerOrFilter( const std::map< std::string, bool >& decisionMap,  const std::string& name ){
    auto decisionIt = decisionMap.find( name );

    //throw error if non-existing trigger or MET filter is requested
    if( decisionIt == decisionMap.cend() ){
        throw std::invalid_argument( "Requested trigger or MET filter '" + name + "' does not exist." );
    } else {
        return (*decisionIt).second;
    }
}


bool TriggerInfo::passTrigger( const std::string& triggerName ) const{
    return passTriggerOrFilter( individualTriggerMap, triggerName );
}


bool TriggerInfo::passMetFilter( const std::string& filterName ) const{
    return passTriggerOrFilter( individualMetFilterMap, filterName );
}


void printAvailableInfo( const std::map< std::string, bool >& decisionMap, const std::string& decisionType ){
    std::cout << "Available " << decisionType << " :\n";
    for( const auto& decision : decisionMap ){
        std::cout << decision.first << "\n";
    }
}


void TriggerInfo::printAvailableIndividualTriggers() const{
    printAvailableInfo( individualTriggerMap, "triggers" );
}


void TriggerInfo::printAvailableMetFilters() const{
    printAvailableInfo( individualMetFilterMap, "MET filters");
}
