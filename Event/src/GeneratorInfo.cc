#include "../interface/GeneratorInfo.h"

//include c++ library classes 
#include <stdexcept>


std::map< std::string,unsigned > getWeightIndices( const TreeReader& treeReader ){
    // internal helper function to get QCD scale and PDF weight indices.
    // hard-coded for now...
    // see https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToPDF
    std::map< std::string,unsigned > resMap;
    // default case for most samples
    resMap["firstScaleIndex"] = 0;
    resMap["numberOfScaleVariations"] = std::min( treeReader._nLheWeights, unsigned(9) );
    resMap["firstPdfIndex"] = 9;
    resMap["numberOfPdfVariations"] = std::min( std::max( treeReader._nLheWeights, unsigned(9) ) - 9, unsigned(100));
    return resMap;
}


GeneratorInfo::GeneratorInfo( const TreeReader& treeReader ) :
    _numberOfLheWeights( treeReader._nLheWeights ),
    _numberOfPsWeights( treeReader._nPsWeights ),
    _ttgEventType( treeReader._ttgEventType ),
    _zgEventType( treeReader._zgEventType ),
    _partonLevelHT( treeReader._lheHTIncoming ),
    _numberOfTrueInteractions( treeReader._nTrueInt ),
    _genMetPtr( new GenMet( treeReader ) )
{
    // combined prefire 
    if( treeReader.containsPrefire() ){
	_prefireWeight = treeReader._prefireWeight;
	_prefireWeightDown = treeReader._prefireWeightDown;
	_prefireWeightUp = treeReader._prefireWeightUp;
    }
    // prefire split in components
    if( treeReader.containsPrefireComponents() ){
	_prefireWeightMuon = treeReader._prefireWeightMuon;
        _prefireWeightMuonDown = treeReader._prefireWeightMuonDown;
        _prefireWeightMuonUp = treeReader._prefireWeightMuonUp;
	_prefireWeightECAL = treeReader._prefireWeightECAL;
        _prefireWeightECALDown = treeReader._prefireWeightECALDown;
        _prefireWeightECALUp = treeReader._prefireWeightECALUp;
    }
    // check if the sample contains more LHE weights than maximally allowed
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

    // check if sample contains more PS weights than maximally allowed
    if( _numberOfPsWeights > maxNumberOfPsWeights ){
	std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
	message.append( " _numberOfPsWeights is " + std::to_string(_numberOfPsWeights) );
	message.append( " which is larger than " + std::to_string(maxNumberOfPsWeights) );
	message.append( " (the maximum array size of _psWeights)." );
	throw std::out_of_range( message );
    }
    // check if sample contains less PS weights than expected
    if( _numberOfPsWeights < maxNumberOfPsWeights ){
        std::string message = "WARNING in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfPsWeights is " + std::to_string(_numberOfPsWeights) );
        message.append( " which is smaller than " + std::to_string(maxNumberOfPsWeights) );
        message.append( " (the expected array size of _psWeights)." );
        //std::cerr << message << std::endl;
    }
    for( unsigned i = 0; i < _numberOfPsWeights; ++i ){
        _psWeights[i] = treeReader._psWeight[i];
    }

    // get addtional parameters to read the weights correctly
    // note: this is temporary in expectation of a good way to read the lhe weights correctly 
    //       in the ntuplizer instead of simply reading the first min(148,_nLheWeight)
    std::map< std::string,unsigned > paramMap = getWeightIndices(treeReader);
    _firstScaleIndex = paramMap["firstScaleIndex"];
    _numberOfScaleVariations = paramMap["numberOfScaleVariations"];
    _firstPdfIndex = paramMap["firstPdfIndex"];
    _numberOfPdfVariations = paramMap["numberOfPdfVariations"];

    // printouts for testing
    /*std::cout << "INFO from GeneratorInfo constructor:" << std::endl;
    std::cout << "  number of lhe weights: " << _numberOfLheWeights << std::endl;
    std::cout << "  first scale index: " << _firstScaleIndex << std::endl;
    std::cout << "  number of scale variations: " << _numberOfScaleVariations << std::endl;
    std::cout << "  first pdf index: " << _firstPdfIndex << std::endl;
    std::cout << "  number of pdf variations: " << _numberOfPdfVariations << std::endl;
    std::cout << "  number of ps weights: " << _numberOfPsWeights << std::endl;*/
}

double retrieveWeight( const double* array, const unsigned index, 
    const unsigned offset, const unsigned maximumIndex, 
    const std::string& name ){
    if( index >= maximumIndex ){
        std::string maximumIndexStr = std::to_string( maximumIndex );
        //throw std::out_of_range( "Only " + maximumIndexStr + " " + name + " variations are available, and an index larger or equal than " + maximumIndexStr + " is requested." );
	std::cout << "WARNING: only " + maximumIndexStr + " " + name + " variations are available";
	std::cout << " and an index larger or equal than " + maximumIndexStr + " is requested;";
	std::cout << " returning relative weight 1 instead..." << std::endl;
	return 1;
    }
    return array[ index + offset ];
}


double GeneratorInfo::relativeWeightPdfVar( const unsigned pdfIndex ) const{
    return retrieveWeight( _lheWeights, pdfIndex, _firstPdfIndex, _numberOfPdfVariations, "pdf" );
}


double GeneratorInfo::relativeWeightScaleVar( const unsigned scaleIndex ) const{
    return retrieveWeight( _lheWeights, scaleIndex, _firstScaleIndex, _numberOfScaleVariations, "scale" );
}


double GeneratorInfo::relativeWeightPsVar( const unsigned psIndex ) const{
    return retrieveWeight( _psWeights, psIndex, 0, std::min( _numberOfPsWeights, maxNumberOfPsWeights ), "parton shower" ); 
}
