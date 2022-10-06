#include "../interface/CombinedReweighter.h"

// include c++ library classes
#include <stdexcept>


// adding and removing reweighters

void CombinedReweighter::addReweighter( 
    const std::string& name, 
    const std::shared_ptr< Reweighter >& reweighter ){
    // add a reweighter
    reweighterMap[ name ] = reweighter;
    reweighterVector.push_back( reweighter );
}

std::map< std::string, std::shared_ptr< Reweighter > >::const_iterator findAndCheckReweighter( 
    const std::string& name, 
    const std::map< std::string, std::shared_ptr< Reweighter > >& reweighterMap ){
    // check if a reweighter with a requested name is present in a string-to-reweighter map
    auto it = reweighterMap.find( name );
    if( it == reweighterMap.cend() ){
	std::string msg = "ERROR in CombinedReweighter: requested to access Reweighter '" + name + "',";
	msg.append( " but no Reweighter of that name is present." );
        throw std::invalid_argument( msg );
    }
    return it;
}

void CombinedReweighter::eraseReweighter( const std::string& name ){
    // delete a reweighter
    auto mapIt = findAndCheckReweighter( name, reweighterMap );
    // use the address to delete the corresponding element in the vector
    Reweighter* address = mapIt->second.get();
    // remove from map
    reweighterMap.erase( mapIt );
    // remove from vector
    for( auto vecIt = reweighterVector.begin(); vecIt != reweighterVector.end(); ++vecIt ){
        if( vecIt->get() == address ){
            reweighterVector.erase( vecIt );
            // break is needed to avoid problems with invalid iterators after calling erase
            break;
        }
    }
}


// get a specific reweighter

const Reweighter* CombinedReweighter::operator[]( const std::string& name ) const{
    auto it = findAndCheckReweighter( name, reweighterMap );
    return it->second.get();
}


// get weights

double CombinedReweighter::totalWeight( const Event& event ) const{
    // get the total weight as multiplication of individual weights
    double weight = 1.;
    for( const auto& r : reweighterVector ){
        weight *= r->weight( event );
    }
    return weight;
}

double CombinedReweighter::weightUp( const Event& event ) const{
    // get the total up-varied weight as multiplication of individual up-varied weights
    // note: this only makes sense if all the reweighters are fully correlated!
    double weight = 1.;
    for( const auto& r : reweighterVector ){
        weight *= r->weightUp( event );
    }
    return weight;
}

double CombinedReweighter::weightDown( const Event& event ) const{
    // get the total down-varied weight as multiplication of individual down-varied weights
    // note: this only makes sense if all the reweighters are fully correlated!
    double weight = 1.;
    for( const auto& r : reweighterVector ){
        weight *= r->weightDown( event );
    }
    return weight;
}

double CombinedReweighter::weightWithout( const Event& event, const std::string& reweighter ) const{
    // helper function for weightUp and weightDown.
    // get the total weight divided by the weight of one of the reweighters
    double weight = totalWeight(event);
    double rweight = (*this)[reweighter]->weight(event);
    if( std::abs(rweight)<1e-12 ){
        std::string msg = "ERROR in CombinedReweighter: nominal weight for reweighter";
        msg.append( " '"+reweighter+"' is zero!" );
        throw std::runtime_error( msg );
    }
    return weight/rweight;
}

double CombinedReweighter::weightUp( const Event& event, const std::string& reweighter ) const{
    // get the total weight where one of the reweighters is varied upwards
    double weight = weightWithout( event, reweighter );
    weight *= (*this)[reweighter]->weightUp(event);
    return weight;
}

double CombinedReweighter::weightDown( const Event& event, const std::string& reweighter ) const{
    // get the total weight where one of the reweighters is varied downwards
    double weight = weightWithout( event, reweighter );
    weight *= (*this)[reweighter]->weightDown(event);
    return weight;
}
