#include "../interface/GeneratorInfo.h"

//include c++ library classes 
#include <stdexcept>



GeneratorInfo::GeneratorInfo( const TreeReader& treeReader ) :
    _numberOfLheWeights( treeReader._nLheWeights ),
    _numberOfPsWeights( treeReader._nPsWeights ),
    _ttgEventType( treeReader._ttgEventType ),
    _zgEventType( treeReader._zgEventType ),
    _partonLevelHT( treeReader._lheHTIncoming ),
    _numberOfTrueInteractions( treeReader._nTrueInt ),
    _genMetPtr( new GenMet( treeReader ) )
{ 
    if( treeReader.containsPrefire() ){
	_prefireWeight = treeReader._prefireWeight;
	_prefireWeightDown = treeReader._prefireWeightDown;
	_prefireWeightUp = treeReader._prefireWeightUp;
    }
    if( treeReader.containsPrefireComponents() ){
	_prefireWeightMuon = treeReader._prefireWeightMuon;
        _prefireWeightMuonDown = treeReader._prefireWeightMuonDown;
        _prefireWeightMuonUp = treeReader._prefireWeightMuonUp;
	_prefireWeightECAL = treeReader._prefireWeightECAL;
        _prefireWeightECALDown = treeReader._prefireWeightECALDown;
        _prefireWeightECALUp = treeReader._prefireWeightECALUp;
    }
    if( _numberOfLheWeights > maxNumberOfLheWeights ){
	std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
	message.append( " _numberOfLheWeights is " + std::to_string(_numberOfLheWeights) );
	message.append( " which is larger than " + std::to_string(maxNumberOfLheWeights) );
	message.append( " (the maximum array size of _lheWeights)." );
	throw std::out_of_range( message );
    }
    for( unsigned i = 0; i < _numberOfLheWeights; ++i  ){
        _lheWeights[i] = treeReader._lheWeight[i];
    }

    if( _numberOfPsWeights > maxNumberOfPsWeights ){
	std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
	message.append( " _numberOfPsWeights is " + std::to_string(_numberOfPsWeights) );
	message.append( " which is larger than " + std::to_string(maxNumberOfPsWeights) );
	message.append( " (the maximum array size of _psWeights)." );
	throw std::out_of_range( message );
    }
    for( unsigned i = 0; i < _numberOfPsWeights; ++i ){
        _psWeights[i] = treeReader._psWeight[i];
    }
}


double retrieveWeight( const double* array, const unsigned index, const unsigned offset, const unsigned maximumIndex, const std::string& name ){
    if( index >= maximumIndex ){
        std::string maximumIndexStr = std::to_string( maximumIndex );
        throw std::out_of_range( "Only " + maximumIndexStr + " " + name + " variations are available, and an index larger or equal than " + maximumIndexStr + " is requested." );
    }
    return array[ index + offset ];
}


double GeneratorInfo::relativeWeightPdfVar( const unsigned pdfIndex ) const{
    return retrieveWeight( _lheWeights, pdfIndex, 9, std::min( std::max( _numberOfLheWeights, unsigned(9) ) - 9, unsigned(100) ), "pdf" );
}


double GeneratorInfo::relativeWeightScaleVar( const unsigned scaleIndex ) const{
    return retrieveWeight( _lheWeights, scaleIndex, 0, std::min( _numberOfLheWeights, unsigned(9) ), "scale" );
}


double GeneratorInfo::relativeWeightPsVar( const unsigned psIndex ) const{
    return retrieveWeight( _psWeights, psIndex, 0, std::min( _numberOfPsWeights, unsigned(14) ), "parton shower" ); 
}
