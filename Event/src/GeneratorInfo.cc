#include "../interface/GeneratorInfo.h"

//include c++ library classes 
#include <stdexcept>


std::map< std::string,unsigned > getWeightIndices( const TreeReader& treeReader ){
    // internal helper function to get scale and PDF weight indices from custom ntuples.
    // hard-coded for now...
    // see https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToPDF
    std::map< std::string,unsigned > resMap;
    // default case for most samples
    resMap["firstScaleIndex"] = 0;
    resMap["numberOfScaleWeights"] = std::min( treeReader._nLheWeights, unsigned(9) );
    resMap["firstPdfIndex"] = 9;
    resMap["numberOfPdfWeights"] = std::min( std::max( treeReader._nLheWeights, unsigned(9) ) - 9, unsigned(100));
    return resMap;
}


// constructor from TreeReader
GeneratorInfo::GeneratorInfo( const TreeReader& treeReader ) :
    _numberOfLheWeights( treeReader._nLheWeights ),
    _numberOfPsWeights( treeReader._nPsWeights ),
    _ttgEventType( treeReader._ttgEventType ),
    _zgEventType( treeReader._zgEventType ),
    _partonLevelHT( treeReader._lheHTIncoming ),
    _numberOfTrueInteractions( treeReader._nTrueInt )
{
    // set combined prefire weights
    if( treeReader.containsPrefire() ){
	_prefireWeight = treeReader._prefireWeight;
	_prefireWeightDown = treeReader._prefireWeightDown;
	_prefireWeightUp = treeReader._prefireWeightUp;
    }
    // set prefire split in components
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
    // get addtional parameters to read the weights correctly
    // note: this is temporary in expectation of a good way to read the lhe weights correctly 
    //       in the ntuplizer instead of simply reading the first min(148,_nLheWeight)
    std::map< std::string,unsigned > paramMap = getWeightIndices(treeReader);
    unsigned firstScaleIndex = paramMap["firstScaleIndex"];
    _numberOfScaleWeights = paramMap["numberOfScaleWeights"];
    unsigned firstPdfIndex = paramMap["firstPdfIndex"];
    _numberOfPdfWeights = paramMap["numberOfPdfWeights"];
    
    // check number of scale weights
    if( _numberOfScaleWeights > maxNumberOfScaleWeights ){
        std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfScaleWeights is " + std::to_string(_numberOfScaleWeights) );
        message.append( " which is larger than " + std::to_string(maxNumberOfScaleWeights) );
        throw std::out_of_range( message );
    }
    // read scale weights
    // for the number and order of the weights:
    // see the ntuplizer: https://github.com/GhentAnalysis/heavyNeutrino/blob/UL_master/multilep/src/LheAnalyzer.cc,
    //   and here: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToPDF.
    // note: make sure to synchronize with Tools/interface/SampleCrossSections!
    _relativeWeight_MuR_1_MuF_1 = treeReader._lheWeight[firstScaleIndex + 0];
    _relativeWeight_MuR_1_MuF_2 = treeReader._lheWeight[firstScaleIndex + 1];
    _relativeWeight_MuR_1_MuF_0p5 = treeReader._lheWeight[firstScaleIndex + 2];
    _relativeWeight_MuR_2_MuF_1 = treeReader._lheWeight[firstScaleIndex + 3];
    _relativeWeight_MuR_2_MuF_2 = treeReader._lheWeight[firstScaleIndex + 4];
    _relativeWeight_MuR_2_MuF_0p5 = treeReader._lheWeight[firstScaleIndex + 5];
    _relativeWeight_MuR_0p5_MuF_1 = treeReader._lheWeight[firstScaleIndex + 6];
    _relativeWeight_MuR_0p5_MuF_2 = treeReader._lheWeight[firstScaleIndex + 7];
    _relativeWeight_MuR_0p5_MuF_0p5 = treeReader._lheWeight[firstScaleIndex + 8];

    // check number of pdf weights
    if( _numberOfPdfWeights > maxNumberOfPdfWeights ){
        std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfPdfWeights is " + std::to_string(_numberOfPdfWeights) );
        message.append( " which is larger than " + std::to_string(maxNumberOfPdfWeights) );
        throw std::out_of_range( message );
    }
    // read pdf weights
    for( unsigned i = 0; i < _numberOfPdfWeights; ++i ){
        _pdfWeights[i] = treeReader._lheWeight[firstPdfIndex + i];
    }

    // check if sample contains more PS weights than maximally allowed
    if( _numberOfPsWeights > maxNumberOfPsWeights ){
	std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
	message.append( " _numberOfPsWeights is " + std::to_string(_numberOfPsWeights) );
	message.append( " which is larger than " + std::to_string(maxNumberOfPsWeights) );
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
    // read PS weights
    // for the number and order of the weights,
    // see the ntuplizer: https://github.com/GhentAnalysis/heavyNeutrino/blob/UL_master/multilep/src/LheAnalyzer.cc,
    // and here: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToPDF.
    // note: make sure to synchronize with Tools/interface/SampleCrossSections!
    _relativeWeight_ISR_InverseSqrt2 = treeReader._psWeight[24];
    _relativeWeight_FSR_InverseSqrt2 = treeReader._psWeight[2];
    _relativeWeight_ISR_Sqrt2 = treeReader._psWeight[25];
    _relativeWeight_FSR_Sqrt2 = treeReader._psWeight[3];
    _relativeWeight_ISR_0p5 = treeReader._psWeight[26];
    _relativeWeight_FSR_0p5 = treeReader._psWeight[4];
    _relativeWeight_ISR_2 = treeReader._psWeight[27];
    _relativeWeight_FSR_2 = treeReader._psWeight[5];
    _relativeWeight_ISR_0p25 = treeReader._psWeight[28];
    _relativeWeight_FSR_0p25 = treeReader._psWeight[6];
    _relativeWeight_ISR_4 = treeReader._psWeight[29];
    _relativeWeight_FSR_4 = treeReader._psWeight[7];
}


// constructor from NanoGenTreeReader
GeneratorInfo::GeneratorInfo( const NanoGenTreeReader& treeReader ) :
    _numberOfScaleWeights( treeReader._nLHEScaleWeight ),
    _numberOfPdfWeights( treeReader._nLHEPdfWeight ),
    _numberOfPsWeights( treeReader._nPSWeight )
{
    // check number of scale weights
    if( _numberOfScaleWeights > maxNumberOfScaleWeights ){
        std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfScaleWeights is " + std::to_string(_numberOfScaleWeights) );
        message.append( " which is larger than " + std::to_string(maxNumberOfScaleWeights) );
        throw std::out_of_range( message );
    }
    // read scale weights
    // for the number and order of the weights:
    // see here: https://cms-nanoaod-integration.web.cern.ch/autoDoc/NanoAODv9/
    // note: make sure to synchronize with Tools/interface/SampleCrossSections!
    _relativeWeight_MuR_1_MuF_1 = treeReader._LHEScaleWeight[4];
    _relativeWeight_MuR_1_MuF_2 = treeReader._LHEScaleWeight[5];
    _relativeWeight_MuR_1_MuF_0p5 = treeReader._LHEScaleWeight[3];
    _relativeWeight_MuR_2_MuF_1 = treeReader._LHEScaleWeight[7];
    _relativeWeight_MuR_2_MuF_2 = treeReader._LHEScaleWeight[8];
    _relativeWeight_MuR_2_MuF_0p5 = treeReader._LHEScaleWeight[6];
    _relativeWeight_MuR_0p5_MuF_1 = treeReader._LHEScaleWeight[1];
    _relativeWeight_MuR_0p5_MuF_2 = treeReader._LHEScaleWeight[2];
    _relativeWeight_MuR_0p5_MuF_0p5 = treeReader._LHEScaleWeight[0];

    // check number of pdf weights
    if( _numberOfPdfWeights > maxNumberOfPdfWeights ){
        std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfPdfWeights is " + std::to_string(_numberOfPdfWeights) );
        message.append( " which is larger than " + std::to_string(maxNumberOfPdfWeights) );
        throw std::out_of_range( message );
    }
    // read pdf weights
    for( unsigned i = 0; i < _numberOfPdfWeights; ++i ){
        _pdfWeights[i] = treeReader._LHEPdfWeight[i];
    }

    // check if sample contains more PS weights than maximally allowed
    if( _numberOfPsWeights > maxNumberOfPsWeights ){
        std::string message = "ERROR in GeneratorInfo::GeneratorInfo:";
        message.append( " _numberOfPsWeights is " + std::to_string(_numberOfPsWeights) );
        message.append( " which is larger than " + std::to_string(maxNumberOfPsWeights) );
        throw std::out_of_range( message );
    }
    // read PS weights
    // for the number and order of the weights,
    // see here: https://cms-nanoaod-integration.web.cern.ch/autoDoc/
    // note: make sure to synchronize with Tools/interface/SampleCrossSections!
    _relativeWeight_ISR_0p5 = treeReader._PSWeight[2];
    _relativeWeight_FSR_0p5 = treeReader._PSWeight[3];
    _relativeWeight_ISR_2 = treeReader._PSWeight[0];
    _relativeWeight_FSR_2 = treeReader._PSWeight[1];
}


double retrieveWeight( const double* array, const unsigned index, 
    const unsigned offset, const unsigned maximumIndex, 
    const std::string& name ){
    if( index >= maximumIndex ){
        std::string maximumIndexStr = std::to_string( maximumIndex );
	std::cout << "WARNING: only " + maximumIndexStr + " " + name + " variations are available";
	std::cout << " and an index larger or equal than " + maximumIndexStr + " is requested;";
	std::cout << " returning relative weight 1 instead..." << std::endl;
	return 1;
    }
    return array[ index + offset ];
}


double GeneratorInfo::relativeWeightPdfVar( const unsigned pdfIndex ) const{
    return retrieveWeight( _pdfWeights, pdfIndex, 0, _numberOfPdfWeights, "pdf" );
}
