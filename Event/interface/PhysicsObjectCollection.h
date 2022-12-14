#ifndef PhysicsObjectCollection_H
#define PhysicsObjectCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>



template< typename ObjectType > class PhysicsObjectCollection {

    public:
        using collection_type = std::vector< std::shared_ptr< ObjectType > >;
        using iterator = typename collection_type::iterator;
        using const_iterator = typename collection_type::const_iterator;
        using value_type = typename collection_type::value_type;
        using size_type = typename collection_type::size_type;

        PhysicsObjectCollection() {}
        ~PhysicsObjectCollection() = default;

        void push_back( const ObjectType& );
        void push_back( ObjectType&& );

        iterator begin(){ return collection.begin(); }
        const_iterator begin() const{ return collection.cbegin(); }
        const_iterator cbegin() const{ return collection.cbegin(); }
        iterator end(){ return collection.end(); }
        const_iterator end() const{ return collection.cend(); }
        const_iterator cend() const{ return collection.cend(); }

        size_type size() const{ return collection.size(); }

        ObjectType& operator[]( const size_type index ){ return *collection[index]; }
        const ObjectType& operator[]( const size_type index ) const{ return *collection[index]; }
        
        template< typename func > void sortByAttribute( const func& f );
        void sortByPt(){ return sortByAttribute( [](const std::shared_ptr< ObjectType >& lhs, const std::shared_ptr< ObjectType >& rhs){ return lhs->pt() > rhs->pt(); } ); }

        PhysicsObject objectSum() const;
        double mass() const;
        double scalarPtSum() const;

        // apply a user specified selection
        void selectObjects( bool (&passSelection)( const ObjectType& ) );

        // return a vector of all possible object pairs
        std::vector< std::pair< std::shared_ptr< ObjectType >, std::shared_ptr< ObjectType > > > pairCollection() const;

    protected:
        PhysicsObjectCollection( const collection_type& col ) : collection( col ) {}

        // select objects passing a threshold and remove the others 
        void selectObjects( bool (ObjectType::*passSelection)() const );

        template< typename IteratorType > IteratorType erase( IteratorType );

        // count the number of objects satisfying given criterion
        size_type count( bool (ObjectType::*passSelection)() const ) const;

        // check if a pair satisfies a given criterion
	bool hasPairWithRequirement( bool (*)(const ObjectType&, const ObjectType&) ) const;

    private:
        collection_type collection;
};



template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::push_back( const ObjectType& physicsObject ){
    collection.push_back( std::shared_ptr< ObjectType >( physicsObject.clone() ) );
}


template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::push_back( ObjectType&& physicsObject ){
    collection.push_back( std::shared_ptr< ObjectType >( std::move( physicsObject ).clone() ) );
}


template< typename ObjectType > template< typename func > void PhysicsObjectCollection< ObjectType >::sortByAttribute( const func& f ){
    std::sort( begin(), end(), f );
}


template< typename ObjectType > template< typename IteratorType > IteratorType PhysicsObjectCollection< ObjectType >::erase( IteratorType it ){
    return collection.erase( it );
} 


template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::selectObjects( bool (ObjectType::*passSelection)() const ){
    for( const_iterator it = cbegin(); it != cend(); ){
        if( !( (**it).*passSelection)() ){
            it = erase( it );
        } else {
            ++it;
        }
    }
}


template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::selectObjects( bool (&passSelection)( const ObjectType& ) ){
    for( const_iterator it = cbegin(); it != cend(); ){
        if( !passSelection( **it ) ){
            it = erase( it );
        } else {
            ++it;
        }
    }
}


template< typename ObjectType > typename PhysicsObjectCollection< ObjectType >::size_type PhysicsObjectCollection< ObjectType >::count( bool (ObjectType::*passSelection)() const ) const{
    size_type counter = 0;
    for( auto& objectPtr : *this ){
        if( ( (*objectPtr).*passSelection)() ){
            ++counter;
        }
    }
    return counter;
}


template< typename ObjectType > PhysicsObject PhysicsObjectCollection< ObjectType >::objectSum() const{
    PhysicsObject totalSystem( 0, 0, 0, 0, 
	(*this)[0].is2016(), (*this)[0].is2016PreVFP(), (*this)[0].is2016PostVFP(), 
	(*this)[0].is2017(), (*this)[0].is2018() );
    for( const auto& objectPtr : *this ){
        totalSystem += *objectPtr;
    }
    return totalSystem;
}


template< typename ObjectType > double PhysicsObjectCollection< ObjectType >::mass() const{
    return objectSum().mass();
}


template< typename ObjectType > double PhysicsObjectCollection< ObjectType >::scalarPtSum() const{
    double ptSum = 0;
    for( const auto& object : *this ){
        ptSum += object->pt();
    }
    return ptSum;
}


template< typename ObjectType > std::vector< std::pair< std::shared_ptr< ObjectType >, std::shared_ptr< ObjectType > > > PhysicsObjectCollection< ObjectType >::pairCollection() const{
    std::vector< std::pair< std::shared_ptr< ObjectType >, std::shared_ptr< ObjectType > > > pairVector;
    if( size() < 2 ){
        return pairVector;
    }
    for( const_iterator it1 = cbegin(); it1 != ( cend() - 1 ); ++it1 ){
        for( const_iterator it2 = it1 + 1; it2 != cend(); ++it2 ){
            pairVector.push_back( { *it1, *it2 } );
        }
    }
    return pairVector;
}


template< typename ObjectType > bool PhysicsObjectCollection< ObjectType >::hasPairWithRequirement( 
    bool (*satisfyPairRequirement)(const ObjectType& lhs, const ObjectType& rhs) ) const{
    std::vector< std::pair< std::shared_ptr< ObjectType >, std::shared_ptr< ObjectType > > > allPairs;
    allPairs = pairCollection();
    for( auto& pair : allPairs ){
        if( (*satisfyPairRequirement)(*pair.first, *pair.second) ){
            return true;
        }
    }
    return false;
}


#endif
