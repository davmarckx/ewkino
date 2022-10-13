#ifndef CombinedReweighter_H
#define CombinedReweighter_H

//include c++ library classes
#include <map>
#include <memory>
#include <string>

//include other parts of framework
#include "Reweighter.h"



class CombinedReweighter{

    public:
        CombinedReweighter() = default;

	// add or remove reweighters
        void addReweighter( const std::string&, const std::shared_ptr< Reweighter >& );
        void eraseReweighter( const std::string& );

	// get reweighter
        const Reweighter* operator[]( const std::string& ) const;

	// get weights
        double totalWeight( const Event& ) const;
	double weight( const Event& event ) const { return totalWeight(event); }
	// ( weight(event) is just a convenient alias for totalWeight(event) )
	double weightUp( const Event& event ) const;
	double weightDown( const Event& event ) const;
	double singleWeight( const Event& event, const std::string& reweighter ) const;
	double singleWeightUp( const Event& event, const std::string& reweighter ) const;
	double singleWeightDown( const Event& event, const std::string& reweighter ) const;
	double weightWithout( const Event& event, const std::string& reweighter ) const;
	double weightUp( const Event& event, const std::string& reweighter ) const;
	double weightDown( const Event& event, const std::string& reweighter ) const;

    private:
        std::map< std::string, std::shared_ptr< Reweighter > > reweighterMap;
        std::vector< std::shared_ptr< Reweighter > > reweighterVector;
};


#endif
