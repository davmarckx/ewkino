#include "../interface/TreeReader.h"

//include c++ library classes 
#include <fstream>
#include <iostream>
#include <typeinfo>

//include other parts of analysis framework
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../Event/interface/Event.h"
#include "../../constants/luminosities.h"

// constructor //

TreeReader::TreeReader( const std::string& sampleListFile, const std::string& sampleDirectory ){
    readSamples( sampleListFile, sampleDirectory );
}


// functions for sample reading //

void TreeReader::readSamples( const std::string& list, 
			      const std::string& directory, 
			      std::vector<Sample>& sampleVector ){

    //clean current sample list 
    sampleVector.clear();

    //read list of samples from file
    sampleVector = readSampleList(list, directory);

    //print sample information
    for(auto& sample : sampleVector){
        std::cout << "sample: " << sample << std::endl;
    }
}


void TreeReader::readSamples( const std::string& list, const std::string& directory ){
    readSamples( list, directory, this->samples );
}


void TreeReader::readSamples2016( const std::string& list, const std::string& directory ){
    // note: not yet updated to UL files, use with caution
    std::cout << "########################################" << std::endl;
    std::cout << "         2016 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples( list, directory, this->samples2016 );

    //add the 2016 samples to the total sample list 
    this->samples.insert( samples.end(), samples2016.begin(), samples2016.end() );

    //check for errors
    checkSampleEraConsistency();
}


void TreeReader::readSamples2017( const std::string& list, const std::string& directory ){
    // note: not yet updated to UL files, use with caution
    std::cout << "########################################" << std::endl;
    std::cout << "         2017 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples( list, directory, this->samples2017 );

    //add the 2017 samples to the total sample list
    this->samples.insert( samples.end(), samples2017.begin(), samples2017.end() );

    //check for errors 
    checkSampleEraConsistency();
}


void TreeReader::readSamples2018( const std::string& list, const std::string& directory ){
    // note: not yet updated to UL files, use with caution
    std::cout << "########################################" << std::endl;
    std::cout << "         2018 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples( list, directory, this->samples2018 );

    //add the 2018 samples to the total sample list
    this->samples.insert( samples.end(), samples2018.begin(), samples2018.end() );

    //check for errors 
    checkSampleEraConsistency();
}


// functions for initializing maps of branches rather than hard-coded names //

std::pair< std::map< std::string, bool >, std::map< std::string, TBranch* > > buildBranchMap( 
	TTree* treePtr, 
	const std::vector< std::string> nameIdentifiers, 
	const std::string& antiIdentifier = "" ){
    // build a map of branches from a given tree
    // all branches whose name contains nameIdentifier and not antiIdentifier will be added
    // the return type is a pair of two objects:
    // - map of branch name to boolean 
    //   (e.g. usable for triggers)
    //   (values are set to false everywhere)
    // - map of branch name to branch pointer
    //   (e.g. usable for many branches with similar names)
    //   (branch pointers are set to nullptr everywhere)
    std::map< std::string, bool > decisionMap;
    std::map< std::string, TBranch* > branchMap;
    TObjArray* branch_list = treePtr->GetListOfBranches();
    for( const auto& branchPtr : *branch_list ){
        std::string branchName = branchPtr->GetName();
	bool select = true;
	for( std::string nameIdentifier: nameIdentifiers ){
	    if( !stringTools::stringContains( branchName, nameIdentifier ) ) select = false;
	}
	if( !select ) continue;
        if( antiIdentifier != "" 
	    && stringTools::stringContains( branchName, antiIdentifier ) ) continue;
	decisionMap[ branchName ] = false;
        branchMap[ branchName ] = nullptr;
    }
    return { decisionMap, branchMap };
}


void TreeReader::initializeTriggerMap( TTree* treePtr ){
    auto triggerMaps = buildBranchMap( treePtr, {"HLT"}, "prescale" );
    _triggerMap = triggerMaps.first;
    b__triggerMap = triggerMaps.second;
}


void TreeReader::initializeMetFilterMap( TTree* treePtr ){
    //WARNING: Currently one MET filter contains 'updated' rather than 'Flag' in the name. 
    //If this changes, make sure to modify the code here!
    auto filterMaps = buildBranchMap( treePtr, {"Flag"} );
    auto filterMaps_part2 = buildBranchMap( treePtr , {"updated"} );
    filterMaps.first.insert( filterMaps_part2.first.cbegin(), filterMaps_part2.first.cend() );
    filterMaps.second.insert( filterMaps_part2.second.cbegin(), filterMaps_part2.second.cend() );

    _MetFilterMap = filterMaps.first;
    b__MetFilterMap = filterMaps.second;
}


void TreeReader::initializeJecSourcesMaps( TTree* treePtr ){
    // initialize jetPt branches
    b__jetPt_JECSourcesUp = buildBranchMap( treePtr, {"_jetPt_","_JECSourcesUp"}).second;
    for( auto mapEl: b__jetPt_JECSourcesUp ){ _jetPt_JECSourcesUp[mapEl.first]; }
    b__jetPt_JECSourcesDown = buildBranchMap( treePtr, {"_jetPt","_JECSourcesDown"}).second;
    for( auto mapEl: b__jetPt_JECSourcesDown ){ _jetPt_JECSourcesDown[mapEl.first]; }
    // initialize jetSmearedPt branches
    b__jetSmearedPt_JECSourcesUp = buildBranchMap( treePtr, 
	{"_jetSmearedPt","JECSourcesUp"}).second;
    for( auto mapEl: b__jetSmearedPt_JECSourcesUp ){ 
	_jetSmearedPt_JECSourcesUp[mapEl.first]; 
    }
    b__jetSmearedPt_JECSourcesDown = buildBranchMap( treePtr, 
	{"_jetSmearedPt","JECSourcesDown"}).second;
    for( auto mapEl: b__jetSmearedPt_JECSourcesDown ){ 
	_jetSmearedPt_JECSourcesDown[mapEl.first]; 
    }
    // initialize met x branches
    b__corrMETx_JECSourcesUp = buildBranchMap( treePtr, {"_corrMETx","_JECSourcesUp"}).second;
    for( auto mapEl: b__corrMETx_JECSourcesUp ){ _corrMETx_JECSourcesUp[mapEl.first]; }
    b__corrMETx_JECSourcesDown = buildBranchMap( treePtr, {"_corrMETx","_JECSourcesDown"}).second;
    for( auto mapEl: b__corrMETx_JECSourcesDown ){ _corrMETx_JECSourcesDown[mapEl.first]; }
    // initialize met y branches
    b__corrMETy_JECSourcesUp = buildBranchMap( treePtr, {"_corrMETy","_JECSourcesUp"}).second;
    for( auto mapEl: b__corrMETy_JECSourcesUp ){ _corrMETy_JECSourcesUp[mapEl.first]; }
    b__corrMETy_JECSourcesDown = buildBranchMap( treePtr, {"_corrMETy","_JECSourcesDown"}).second;
    for( auto mapEl: b__corrMETy_JECSourcesDown ){ _corrMETy_JECSourcesDown[mapEl.first]; }
}


void TreeReader::initializeJecSourcesGroupedMaps( TTree* treePtr ){
    // initialize jetPt branches
    b__jetPt_JECGroupedUp = buildBranchMap( treePtr, {"_jetPt_","_JECGroupedUp"}).second;
    for( auto mapEl: b__jetPt_JECGroupedUp ){ _jetPt_JECGroupedUp[mapEl.first]; }
    b__jetPt_JECGroupedDown = buildBranchMap( treePtr, {"_jetPt","_JECGroupedDown"}).second;
    for( auto mapEl: b__jetPt_JECGroupedDown ){ _jetPt_JECGroupedDown[mapEl.first]; }
    // initialize jetSmearedPt branches
    b__jetSmearedPt_JECGroupedUp = buildBranchMap( treePtr, 
	{"_jetSmearedPt","JECGroupedUp"}).second;
    for( auto mapEl: b__jetSmearedPt_JECGroupedUp ){ 
	_jetSmearedPt_JECGroupedUp[mapEl.first]; 
    }
    b__jetSmearedPt_JECGroupedDown = buildBranchMap( treePtr, 
	{"_jetSmearedPt","JECGroupedDown"}).second;
    for( auto mapEl: b__jetSmearedPt_JECGroupedDown ){ 
	_jetSmearedPt_JECGroupedDown[mapEl.first]; 
    }
    // initialize met x branches
    b__corrMETx_JECGroupedUp = buildBranchMap( treePtr, {"_corrMETx","_JECGroupedUp"}).second;
    for( auto mapEl: b__corrMETx_JECGroupedUp ){ _corrMETx_JECGroupedUp[mapEl.first]; }
    b__corrMETx_JECGroupedDown = buildBranchMap( treePtr, {"_corrMETx","_JECGroupedDown"}).second;
    for( auto mapEl: b__corrMETx_JECGroupedDown ){ _corrMETx_JECGroupedDown[mapEl.first]; }
    // initialize met y branches
    b__corrMETy_JECGroupedUp = buildBranchMap( treePtr, {"_corrMETy","_JECGroupedUp"}).second;
    for( auto mapEl: b__corrMETy_JECGroupedUp ){ _corrMETy_JECGroupedUp[mapEl.first]; }
    b__corrMETy_JECGroupedDown = buildBranchMap( treePtr, {"_corrMETy","_JECGroupedDown"}).second;
    for( auto mapEl: b__corrMETy_JECGroupedDown ){ _corrMETy_JECGroupedDown[mapEl.first]; }
}


// functions to find if a tree has branches with certain types of info //

bool treeHasBranchWithName( TTree* treePtr, const std::string& nameToFind ){
    TObjArray* branch_list = treePtr->GetListOfBranches();
    for( const auto& branchPtr : *branch_list ){
	std::string branchName = branchPtr->GetName();
	    if( stringTools::stringContains( branchName, nameToFind ) ){
		return true;
	    }
    }
    return false;
}


bool TreeReader::containsTauInfo() const{
    return treeHasBranchWithName( _currentTreePtr, "_tau" );
}


bool TreeReader::containsGeneratorInfo() const{
    // to do: find a more suitable check than only checking the _weight branch;
    //        but more difficult than in other cases since the relevant branch names
    //         have little in common...
    return treeHasBranchWithName( _currentTreePtr, "_weight");
}


bool TreeReader::containsGenParticles() const{
    return treeHasBranchWithName( _currentTreePtr, "_gen_" );
}


bool TreeReader::containsParticleLevel() const{
    return treeHasBranchWithName( _currentTreePtr, "_pl_" );
}


bool TreeReader::containsPrefire() const{
    return treeHasBranchWithName( _currentTreePtr, "_prefireWeight" );
}


bool TreeReader::containsPrefireComponents() const{
    return ( treeHasBranchWithName( _currentTreePtr, "_prefireWeightMuon")
	     && treeHasBranchWithName( _currentTreePtr, "_prefireWeightECAL") );
}


bool TreeReader::containsSusyMassInfo() const{
    return treeHasBranchWithName( _currentTreePtr, "_mChi" );
}


bool TreeReader::containsTriggerInfo( const std::string& triggerPath ) const{
    return treeHasBranchWithName( _currentTreePtr, triggerPath );
}


bool TreeReader::containsRefTriggerInfo() const{
    return containsTriggerInfo("passTrigger_ref");
}


bool TreeReader::isData() const{
    if( _currentSamplePtr ) return _currentSamplePtr->isData();
    else return !containsGeneratorInfo();
}


bool TreeReader::isMC() const{
    return !isData();
}


void TreeReader::checkCurrentSample() const{
    if( !_currentSamplePtr ){
        throw std::domain_error( "pointer to current Sample is nullptr." );
    }
}


void TreeReader::checkCurrentTree() const{
    if( !_currentTreePtr ){
        throw std::domain_error( "pointer to current TTree is nullptr." );
    }
}


void TreeReader::checkCurrentFile() const{
    if( !_currentFilePtr ){
	throw std::domain_error( "pointer to current TFile is nullptr." );
    }
}


bool TreeReader::is2016() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016();
}


bool TreeReader::is2016PreVFP() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016PreVFP();
}


bool TreeReader::is2016PostVFP() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016PostVFP();
}


bool TreeReader::is2017() const{
    checkCurrentSample();
    return _currentSamplePtr->is2017();
}


bool TreeReader::is2018() const{
    checkCurrentSample();
    return _currentSamplePtr->is2018();
}


std::string TreeReader::getYearString() const{
    if( is2016PreVFP() ) return "2016PreVFP";
    else if( is2016PostVFP() ) return "2016PostVFP";
    else if( is2016() ) return "2016";
    else if( is2017() ) return "2017";
    else if( is2018() ) return "2018";
    else{
	std::string msg = "ERROR in TreeReader::getYearString:";
	msg += " no valid year string could be returned as all year checks failed.";
	throw std::runtime_error(msg);
    }
}


bool TreeReader::isSMSignal() const{
    checkCurrentSample();
    return _currentSamplePtr->isSMSignal();
}


bool TreeReader::isNewPhysicsSignal() const{
    checkCurrentSample();
    return _currentSamplePtr->isNewPhysicsSignal();
}


long unsigned TreeReader::numberOfEntries() const{
    checkCurrentTree();
    return _currentTreePtr->GetEntries();
}


void TreeReader::initSample( const Sample& samp, 
			     const bool doInitTree, 
			     const bool doInitHCounter ){ 

    //update current sample
    // old comment from Willem:
    // "I wonder if the extra copy can be avoided here, 
    // its however hard if we want to keep the functionality of reading the sample vector, 
    // and also having the function initSampleFromFile. 
    // It's not clear how we can make a new sample in one of them 
    // and refer to an existing one in the other. 
    // It can be done with a static Sample in 'initSampleFromFile', 
    // but this makes the entire TreeReader class unthreadsafe, 
    // so no parallel sample processing in one process can be done"
    _currentSamplePtr = std::make_shared< Sample >( samp );
    _currentFilePtr = samp.filePtr();

    // old comment from Willem:
    // "Warning: this pointer is overwritten, but it is not a memory leak. 
    // ROOT is dirty and deletes the previous tree upon closure of the TFile it belongs to.
    // The previous TFile is closed by the std::shared_ptr destructor, 
    // implicitly called above when opening a new TFile."
    _currentTreePtr = (TTree*) _currentFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" );
    // temporary addition to read Gianny's files for charge misid. measurement,
    // where the tree is stored in the root directory...
    if( !_currentTreePtr ) _currentTreePtr = (TTree*) _currentFilePtr->Get( "blackJackAndHookersTree" );
    checkCurrentTree();
    if( doInitTree) initTree();
    if( !samp.isData() && doInitHCounter ){

        //read sum of simulated event weights
        TH1D* hCounter = new TH1D( "hCounter", "Events counter", 1, 0, 1 );
        _currentFilePtr->cd( "blackJackAndHookers" );
        hCounter->Read( "hCounter" ); 
        double sumSimulatedEventWeights = hCounter->GetBinContent(1);
        delete hCounter;

        //event weights set with lumi depending on sample's era 
        double dataLumi;
        if( is2016() ){ dataLumi = lumi::lumi2016; } 
	else if( is2016PreVFP() ){ dataLumi = lumi::lumi2016PreVFP; }
	else if( is2016PostVFP() ){ dataLumi = lumi::lumi2016PostVFP; }
        else if( is2017() ){ dataLumi = lumi::lumi2017; } 
	else { dataLumi = lumi::lumi2018; }
        scale = samp.xSec()*dataLumi*1000 / sumSimulatedEventWeights;
    }

    //check whether current sample is a SUSY sample
    _isSusy = containsSusyMassInfo();
}


//initialize the next sample in the list
void TreeReader::initSample( const bool doInitTree,
                             const bool doInitHCounter ){
    initSample( samples[ ++currentSampleIndex ], doInitTree, doInitHCounter );
}


//initialize the current Sample directly from a root file, this is used when skimming
void TreeReader::initSampleFromFile( const std::string& pathToFile, 
				     const bool is2016, 
				     const bool is2016PreVFP,
				     const bool is2016PostVFP,
				     const bool is2017, 
				     const bool is2018, 
				     const bool resetTriggersAndFilters ){

    // check if file exists 
    if( !systemTools::fileExists( pathToFile ) ){
        throw std::invalid_argument( "File '" + pathToFile + "' does not exist." );
    }

    _currentFilePtr = std::shared_ptr< TFile >( new TFile( pathToFile.c_str() ) );

    // check year
    if( !(is2016 || is2016PreVFP || is2016PostVFP || is2017 || is2018 ) ){
	std::string msg = "ERROR in TreeReader::initSampleFromFile:";
	msg += " no valid year was given for sample ";
	msg += pathToFile;
	throw std::runtime_error(msg);
    }

    // old comment from Willem:
    // "Warning: this pointer is overwritten, but it is not a memory leak. 
    // ROOT is dirty and deletes the previous tree upon closure of the TFile it belongs to.
    // The previous TFile is closed by the std::shared_ptr destructor, 
    // implicitly called above when opening a new TFile."
    _currentTreePtr = (TTree*) _currentFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" );
    checkCurrentTree();

    // make a new sample, and make sure the pointer remains valid
    // old comment from Willem:
    // "new is no option here since this would also require a destructor for the class, 
    // which does not work for the other initSample case"
    _currentSamplePtr = std::make_shared< Sample >( pathToFile, is2016, is2016PreVFP,
			    is2016PostVFP, is2017, is2018, isData() );

    //initialize tree
    initTree( resetTriggersAndFilters );

    //check whether current sample is a SUSY sample
    _isSusy = containsSusyMassInfo();

    //set scale so weights don't become 0 when building the event
    scale = 1.;
}


//automatically determine whether sample is 2017 or 2018 from file name 
void TreeReader::initSampleFromFile( const std::string& pathToFile, 
				     const bool resetTriggersAndFilters ){
    bool is2016 = analysisTools::fileIs2016( pathToFile );
    bool is2016PreVFP = analysisTools::fileIs2016PreVFP( pathToFile );
    bool is2016PostVFP = analysisTools::fileIs2016PostVFP( pathToFile );
    bool is2017 = analysisTools::fileIs2017( pathToFile );
    bool is2018 = analysisTools::fileIs2018( pathToFile );
    initSampleFromFile( pathToFile, is2016, is2016PreVFP, is2016PostVFP, is2017, is2018, 
			resetTriggersAndFilters );
}


void TreeReader::GetEntry( const Sample& samp, long unsigned entry ){
    checkCurrentTree();

    _currentTreePtr->GetEntry( entry );

    //Set up correct event weight
    if( !samp.isData() ){
        _scaledWeight = _weight*scale;
    } else{
        _scaledWeight = 1;
    }
}


//use the currently initialized sample when running in serial
void TreeReader::GetEntry( long unsigned entry ){
    GetEntry( *_currentSamplePtr, entry );
}


Event TreeReader::buildEvent( const Sample& samp, long unsigned entry, 
	const bool readIndividualTriggers, const bool readIndividualMetFilters,
	const bool readAllJECVariations, const bool readGroupedJECVariations,
	const bool readParticleLevel ){
    GetEntry( samp, entry );
    return Event( *this, readIndividualTriggers, readIndividualMetFilters,
			readAllJECVariations, readGroupedJECVariations,
			readParticleLevel );
}


Event TreeReader::buildEvent( long unsigned entry, 
	const bool readIndividualTriggers, const bool readIndividualMetFilters,
	const bool readAllJECVariations, const bool readGroupedJECVariations,
	const bool readParticleLevel ){
    GetEntry( entry );
    return Event( *this, readIndividualTriggers, readIndividualMetFilters,
			readAllJECVariations, readGroupedJECVariations,
			readParticleLevel );
}


template< typename T > void setMapBranchAddresses( TTree* treePtr, 
	std::map< std::string, T >& variableMap, 
	std::map< std::string, TBranch* > branchMap ){
    for( const auto& variable : variableMap ){
        treePtr->SetBranchAddress( variable.first.c_str(), 
	    &variableMap[ variable.first ], 
	    &branchMap[ variable.first ] );
    }
}


template< typename T> void setMapOutputBranches( TTree* treePtr, 
			    std::map< std::string, T >& variableMap,
			    std::string branchDataType ){
    // note: branchDataType should be e.g. "/O" for boolean, "[nJets]/D" for an array of doubles.
    // mind the slash!
    for( const auto& variable : variableMap ){
        treePtr->Branch( variable.first.c_str(), 
	    &variableMap[ variable.first ], 
	    ( variable.first + branchDataType ).c_str() );
    }    
}


void TreeReader::initTree( const bool resetTriggersAndFilters ){

    // Set branch addresses and branch pointers
    checkCurrentTree();

    _currentTreePtr->SetMakeClass(1);

    _currentTreePtr->SetBranchAddress("_runNb", &_runNb, &b__runNb);
    _currentTreePtr->SetBranchAddress("_lumiBlock", &_lumiBlock, &b__lumiBlock);
    _currentTreePtr->SetBranchAddress("_eventNb", &_eventNb, &b__eventNb);
    _currentTreePtr->SetBranchAddress("_nVertex", &_nVertex, &b__nVertex);    

    _currentTreePtr->SetBranchAddress("_passTrigger_e", &_passTrigger_e, &b__passTrigger_e);
    _currentTreePtr->SetBranchAddress("_passTrigger_ee", &_passTrigger_ee, &b__passTrigger_ee);
    _currentTreePtr->SetBranchAddress("_passTrigger_eee", &_passTrigger_eee, &b__passTrigger_eee);
    _currentTreePtr->SetBranchAddress("_passTrigger_em", &_passTrigger_em, &b__passTrigger_em);
    _currentTreePtr->SetBranchAddress("_passTrigger_m", &_passTrigger_m, &b__passTrigger_m);
    _currentTreePtr->SetBranchAddress("_passTrigger_eem", &_passTrigger_eem, &b__passTrigger_eem);
    _currentTreePtr->SetBranchAddress("_passTrigger_mm", &_passTrigger_mm, &b__passTrigger_mm);
    _currentTreePtr->SetBranchAddress("_passTrigger_emm", &_passTrigger_emm, &b__passTrigger_emm);
    _currentTreePtr->SetBranchAddress("_passTrigger_mmm", &_passTrigger_mmm, &b__passTrigger_mmm);
    _currentTreePtr->SetBranchAddress("_passTrigger_et", &_passTrigger_et, &b__passTrigger_et);
    _currentTreePtr->SetBranchAddress("_passTrigger_mt", &_passTrigger_mt, &b__passTrigger_mt);
    _currentTreePtr->SetBranchAddress("_passTrigger_FR", &_passTrigger_FR, &b__passTrigger_FR);
    _currentTreePtr->SetBranchAddress("_passTrigger_FR_iso", &_passTrigger_FR_iso, &b__passTrigger_FR_iso);
    if( !containsRefTriggerInfo() ){
        std::string msg = "WARNING: input tree does not seem to contain reference trigger bool;";
        msg.append( " will not read this trigger!" );
        std::cerr << msg << std::endl;
    } else{
	_currentTreePtr->SetBranchAddress("_passTrigger_ref", &_passTrigger_ref, &b__passTrigger_ref);
    }
    _currentTreePtr->SetBranchAddress("_passMETFilters", &_passMETFilters, &b__passMETFilters);

    _currentTreePtr->SetBranchAddress("_nL", &_nL, &b__nL);
    _currentTreePtr->SetBranchAddress("_nMu", &_nMu, &b__nMu);
    _currentTreePtr->SetBranchAddress("_nEle", &_nEle, &b__nEle);
    _currentTreePtr->SetBranchAddress("_nLight", &_nLight, &b__nLight);
    _currentTreePtr->SetBranchAddress("_nTau", &_nTau, &b__nTau);
    _currentTreePtr->SetBranchAddress("_lPt", _lPt, &b__lPt);
    _currentTreePtr->SetBranchAddress("_lPtCorr", _lPtCorr, &b__lPtCorr);
    _currentTreePtr->SetBranchAddress("_lPtScaleUp", _lPtScaleUp, &b__lPtScaleUp);
    _currentTreePtr->SetBranchAddress("_lPtScaleDown", _lPtScaleDown, &b__lPtScaleDown);
    _currentTreePtr->SetBranchAddress("_lPtResUp", _lPtResUp, &b__lPtResUp);
    _currentTreePtr->SetBranchAddress("_lPtResDown", _lPtResDown, &b__lPtResDown);
    _currentTreePtr->SetBranchAddress("_lEta", _lEta, &b__lEta);
    _currentTreePtr->SetBranchAddress("_lEtaSC", _lEtaSC, &b__lEtaSC);
    _currentTreePtr->SetBranchAddress("_lPhi", _lPhi, &b__lPhi);
    _currentTreePtr->SetBranchAddress("_lE", _lE, &b__lE);
    _currentTreePtr->SetBranchAddress("_lECorr", _lECorr, &b__lECorr);
    _currentTreePtr->SetBranchAddress("_lEScaleUp", _lEScaleUp, &b__lEScaleUp);
    _currentTreePtr->SetBranchAddress("_lEScaleDown", _lEScaleDown, &b__lEScaleDown);
    _currentTreePtr->SetBranchAddress("_lEResUp", _lEResUp, &b__lEResUp);
    _currentTreePtr->SetBranchAddress("_lEResDown", _lEResDown, &b__lEResDown);
    _currentTreePtr->SetBranchAddress("_lFlavor", _lFlavor, &b__lFlavor);
    _currentTreePtr->SetBranchAddress("_lCharge", _lCharge, &b__lCharge);
    _currentTreePtr->SetBranchAddress("_dxy", _dxy, &b__dxy);
    _currentTreePtr->SetBranchAddress("_dz", _dz, &b__dz);
    _currentTreePtr->SetBranchAddress("_3dIP", _3dIP, &b__3dIP);
    _currentTreePtr->SetBranchAddress("_3dIPSig", _3dIPSig, &b__3dIPSig);
    _currentTreePtr->SetBranchAddress("_lElectronSummer16MvaGP", _lElectronSummer16MvaGP, &b__lElectronSummer16MvaGP);
    _currentTreePtr->SetBranchAddress("_lElectronSummer16MvaHZZ", _lElectronSummer16MvaHZZ, &b__lElectronSummer16MvaHZZ);
    _currentTreePtr->SetBranchAddress("_lElectronMvaFall17Iso", _lElectronMvaFall17Iso, &b__lElectronMvaFall17Iso);
    _currentTreePtr->SetBranchAddress("_lElectronMvaFall17NoIso", _lElectronMvaFall17NoIso, &b__lElectronMvaFall17NoIso);
    _currentTreePtr->SetBranchAddress("_lElectronPassMVAFall17NoIsoWPLoose", _lElectronPassMVAFall17NoIsoWPLoose, &b__lElectronPassMVAFall17NoIsoWPLoose);
    _currentTreePtr->SetBranchAddress("_lElectronPassMVAFall17NoIsoWP90", _lElectronPassMVAFall17NoIsoWP90, &b__lElectronPassMVAFall17NoIsoWP90);
    _currentTreePtr->SetBranchAddress("_lElectronPassMVAFall17NoIsoWP80", _lElectronPassMVAFall17NoIsoWP80, &b__lElectronPassMVAFall17NoIsoWP80);
    _currentTreePtr->SetBranchAddress("_lElectronPassEmu", _lElectronPassEmu, &b__lElectronPassEmu);
    _currentTreePtr->SetBranchAddress("_lElectronPassConvVeto", _lElectronPassConvVeto, &b__lElectronPassConvVeto);
    _currentTreePtr->SetBranchAddress("_lElectronChargeConst", _lElectronChargeConst, &b__lElectronChargeConst);
    _currentTreePtr->SetBranchAddress("_lElectronMissingHits", _lElectronMissingHits, &b__lElectronMissingHits);
    _currentTreePtr->SetBranchAddress("_lElectronEInvMinusPInv", _lElectronEInvMinusPInv, &b__lElectronEInvMinusPInv);
    _currentTreePtr->SetBranchAddress("_lElectronHOverE", _lElectronHOverE, &b__lElectronHOverE);
    _currentTreePtr->SetBranchAddress("_lElectronSigmaIetaIeta", _lElectronSigmaIetaIeta, &b__lElectronSigmaIetaIeta);
    _currentTreePtr->SetBranchAddress("_leptonMvaTTH", _leptonMvaTTH, &b__leptonMvaTTH);
    _currentTreePtr->SetBranchAddress("_leptonMvatZq", _leptonMvatZq, &b__leptonMvatZq);
    _currentTreePtr->SetBranchAddress("_leptonMvaTOP", _leptonMvaTOP, &b__leptonMvaTOP);
    _currentTreePtr->SetBranchAddress("_leptonMvaTOPUL", _leptonMvaTOPUL, &b__leptonMvaTOPUL);
    _currentTreePtr->SetBranchAddress("_leptonMvaTOPv2UL", _leptonMvaTOPv2UL, &b__leptonMvaTOPv2UL);
    _currentTreePtr->SetBranchAddress("_lPOGVeto", _lPOGVeto, &b__lPOGVeto);
    _currentTreePtr->SetBranchAddress("_lPOGLoose", _lPOGLoose, &b__lPOGLoose);
    _currentTreePtr->SetBranchAddress("_lPOGMedium", _lPOGMedium, &b__lPOGMedium);
    _currentTreePtr->SetBranchAddress("_lPOGTight", _lPOGTight, &b__lPOGTight);
    _currentTreePtr->SetBranchAddress("_relIso", _relIso, &b__relIso);
    _currentTreePtr->SetBranchAddress("_relIso0p4", _relIso0p4, &b__relIso0p4);
    _currentTreePtr->SetBranchAddress("_relIso0p4MuDeltaBeta", _relIso0p4MuDeltaBeta, &b__relIso0p4MuDeltaBeta);
    _currentTreePtr->SetBranchAddress("_miniIso", _miniIso, &b__miniIso);
    _currentTreePtr->SetBranchAddress("_miniIsoCharged", _miniIsoCharged, &b__miniIsoCharged);
    _currentTreePtr->SetBranchAddress("_ptRel", _ptRel, &b__ptRel);
    _currentTreePtr->SetBranchAddress("_ptRatio", _ptRatio, &b__ptRatio);
    _currentTreePtr->SetBranchAddress("_closestJetCsvV2", _closestJetCsvV2, &b__closestJetCsvV2);
    _currentTreePtr->SetBranchAddress("_closestJetDeepCsv_b", _closestJetDeepCsv_b, &b__closestJetDeepCsv_b);
    _currentTreePtr->SetBranchAddress("_closestJetDeepCsv_bb", _closestJetDeepCsv_bb, &b__closestJetDeepCsv_bb);
    _currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_b", _closestJetDeepFlavor_b, &b__closestJetDeepFlavor_b);
    _currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_bb", _closestJetDeepFlavor_bb, &b__closestJetDeepFlavor_bb);
    _currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_lepb", _closestJetDeepFlavor_lepb, &b__closestJetDeepFlavor_lepb);
    _currentTreePtr->SetBranchAddress("_selectedTrackMult", _selectedTrackMult, &b__selectedTrackMult);
    _currentTreePtr->SetBranchAddress("_lMuonSegComp", _lMuonSegComp, &b__lMuonSegComp);
    _currentTreePtr->SetBranchAddress("_lMuonTrackPt", _lMuonTrackPt, &b__lMuonTrackPt);
    _currentTreePtr->SetBranchAddress("_lMuonTrackPtErr", _lMuonTrackPtErr, &b__lMuonTrackPtErr);

    _currentTreePtr->SetBranchAddress("_nJets", &_nJets, &b__nJets);
    _currentTreePtr->SetBranchAddress("_jetPt", _jetPt, &b__jetPt);
    _currentTreePtr->SetBranchAddress("_jetSmearedPt", _jetSmearedPt, &b__jetSmearedPt);
    _currentTreePtr->SetBranchAddress("_jetSmearedPt_JECDown", _jetSmearedPt_JECDown, &b__jetSmearedPt_JECDown);
    _currentTreePtr->SetBranchAddress("_jetSmearedPt_JECUp", _jetSmearedPt_JECUp, &b__jetSmearedPt_JECUp);
    _currentTreePtr->SetBranchAddress("_jetSmearedPt_JERDown", _jetSmearedPt_JERDown, &b__jetSmearedPt_JERDown);
    _currentTreePtr->SetBranchAddress("_jetSmearedPt_JERUp", _jetSmearedPt_JERUp, &b__jetSmearedPt_JERUp);
    _currentTreePtr->SetBranchAddress("_jetPt_JECUp", _jetPt_JECUp, &b__jetPt_JECUp);
    _currentTreePtr->SetBranchAddress("_jetPt_JECDown", _jetPt_JECDown, &b__jetPt_JECDown);
    _currentTreePtr->SetBranchAddress("_jetEta", _jetEta, &b__jetEta);
    _currentTreePtr->SetBranchAddress("_jetPhi", _jetPhi, &b__jetPhi);
    _currentTreePtr->SetBranchAddress("_jetE", _jetE, &b__jetE);
    _currentTreePtr->SetBranchAddress("_jetPt_Uncorrected",_jetPt_Uncorrected, &b__jetPt_Uncorrected);
    _currentTreePtr->SetBranchAddress("_jetPt_L1", _jetPt_L1, &b__jetPt_L1);
    _currentTreePtr->SetBranchAddress("_jetPt_L2", _jetPt_L2, &b__jetPt_L2);
    _currentTreePtr->SetBranchAddress("_jetPt_L3", _jetPt_L3, &b__jetPt_L3);
    _currentTreePtr->SetBranchAddress("_jetCsvV2", _jetCsvV2, &b__jetCsvV2);
    _currentTreePtr->SetBranchAddress("_jetDeepCsv_udsg", _jetDeepCsv_udsg, &b__jetDeepCsv_udsg);
    _currentTreePtr->SetBranchAddress("_jetDeepCsv_b", _jetDeepCsv_b, &b__jetDeepCsv_b);
    _currentTreePtr->SetBranchAddress("_jetDeepCsv_c", _jetDeepCsv_c, &b__jetDeepCsv_c);
    _currentTreePtr->SetBranchAddress("_jetDeepCsv_bb", _jetDeepCsv_bb, &b__jetDeepCsv_bb);
    _currentTreePtr->SetBranchAddress("_jetDeepFlavor_b", _jetDeepFlavor_b, &b__jetDeepFlavor_b);
    _currentTreePtr->SetBranchAddress("_jetDeepFlavor_bb", _jetDeepFlavor_bb, &b__jetDeepFlavor_bb);
    _currentTreePtr->SetBranchAddress("_jetDeepFlavor_lepb", _jetDeepFlavor_lepb, &b__jetDeepFlavor_lepb);
    _currentTreePtr->SetBranchAddress("_jetHadronFlavor", _jetHadronFlavor, &b__jetHadronFlavor);
    _currentTreePtr->SetBranchAddress("_jetIsTight", _jetIsTight, &b__jetIsTight);
    _currentTreePtr->SetBranchAddress("_jetIsTightLepVeto", _jetIsTightLepVeto, &b__jetIsTightLepVeto);
    _currentTreePtr->SetBranchAddress("_jetNeutralHadronFraction", _jetNeutralHadronFraction, &b__jetNeutralHadronFraction);
    _currentTreePtr->SetBranchAddress("_jetChargedHadronFraction", _jetChargedHadronFraction, &b__jetChargedHadronFraction);
    _currentTreePtr->SetBranchAddress("_jetNeutralEmFraction", _jetNeutralEmFraction, &b__jetNeutralEmFraction);
    _currentTreePtr->SetBranchAddress("_jetChargedEmFraction", _jetChargedEmFraction, &b__jetChargedEmFraction);
    _currentTreePtr->SetBranchAddress("_jetHFHadronFraction", _jetHFHadronFraction, &b__jetHFHadronFraction);
    _currentTreePtr->SetBranchAddress("_jetHFEmFraction", _jetHFEmFraction, &b__jetHFEmFraction);

    _currentTreePtr->SetBranchAddress("_met", &_met, &b__met);
    _currentTreePtr->SetBranchAddress("_met_JECDown", &_met_JECDown, &b__met_JECDown);
    _currentTreePtr->SetBranchAddress("_met_JECUp", &_met_JECUp, &b__met_JECUp);
    _currentTreePtr->SetBranchAddress("_met_UnclDown", &_met_UnclDown, &b__met_UnclDown);
    _currentTreePtr->SetBranchAddress("_met_UnclUp", &_met_UnclUp, &b__met_UnclUp);
    _currentTreePtr->SetBranchAddress("_metPhi", &_metPhi, &b__metPhi);
    _currentTreePtr->SetBranchAddress("_metPhi_JECDown", &_metPhi_JECDown, &b__metPhi_JECDown);
    _currentTreePtr->SetBranchAddress("_metPhi_JECUp", &_metPhi_JECUp, &b__metPhi_JECUp);
    _currentTreePtr->SetBranchAddress("_metPhi_UnclDown", &_metPhi_UnclDown, &b__metPhi_UnclDown);
    _currentTreePtr->SetBranchAddress("_metPhi_UnclUp", &_metPhi_UnclUp, &b__metPhi_UnclUp);
    _currentTreePtr->SetBranchAddress("_metSignificance", &_metSignificance, &b__metSignificance);

    if( !containsTauInfo() ){
	std::string msg = "WARNING: input tree does not seem to contain tau info;";
	msg.append( " will not read tau branches!" );
	std::cerr << msg << std::endl;
    } else{
	_currentTreePtr->SetBranchAddress("_tauDecayMode", _tauDecayMode, &b__tauDecayMode);
        _currentTreePtr->SetBranchAddress("_decayModeFinding", _decayModeFinding, &b__decayModeFinding);
	_currentTreePtr->SetBranchAddress("_decayModeFindingNew", _decayModeFindingNew, &b__decayModeFindingNew);
	_currentTreePtr->SetBranchAddress("_tauMuonVetoLoose", _tauMuonVetoLoose, &b__tauMuonVetoLoose);
	_currentTreePtr->SetBranchAddress("_tauMuonVetoTight", _tauMuonVetoTight, &b__tauMuonVetoTight);
	_currentTreePtr->SetBranchAddress("_tauEleVetoVLoose", _tauEleVetoVLoose, &b__tauEleVetoVLoose);
	_currentTreePtr->SetBranchAddress("_tauEleVetoLoose", _tauEleVetoLoose, &b__tauEleVetoLoose);
	_currentTreePtr->SetBranchAddress("_tauEleVetoMedium", _tauEleVetoMedium, &b__tauEleVetoMedium);
	_currentTreePtr->SetBranchAddress("_tauEleVetoTight", _tauEleVetoTight, &b__tauEleVetoTight);
	_currentTreePtr->SetBranchAddress("_tauEleVetoVTight", _tauEleVetoVTight, &b__tauEleVetoVTight);
	_currentTreePtr->SetBranchAddress("_tauPOGVLoose2015", _tauPOGVLoose2015, &b__tauPOGVLoose2015);
	_currentTreePtr->SetBranchAddress("_tauPOGLoose2015", _tauPOGLoose2015, &b__tauPOGLoose2015);
	_currentTreePtr->SetBranchAddress("_tauPOGMedium2015", _tauPOGMedium2015, &b__tauPOGMedium2015);
	_currentTreePtr->SetBranchAddress("_tauPOGTight2015", _tauPOGTight2015, &b__tauPOGTight2015);
	_currentTreePtr->SetBranchAddress("_tauPOGVTight2015", _tauPOGVTight2015, &b__tauPOGVTight2015);
	_currentTreePtr->SetBranchAddress("_tauVLooseMvaNew2015", _tauVLooseMvaNew2015, &b__tauVLooseMvaNew2015);
	_currentTreePtr->SetBranchAddress("_tauLooseMvaNew2015", _tauLooseMvaNew2015, &b__tauLooseMvaNew2015);
	_currentTreePtr->SetBranchAddress("_tauMediumMvaNew2015", _tauMediumMvaNew2015, &b__tauMediumMvaNew2015);
	_currentTreePtr->SetBranchAddress("_tauTightMvaNew2015", _tauTightMvaNew2015, &b__tauTightMvaNew2015);
	_currentTreePtr->SetBranchAddress("_tauVTightMvaNew2015", _tauVTightMvaNew2015, &b__tauVTightMvaNew2015);
	_currentTreePtr->SetBranchAddress("_tauPOGVVLoose2017v2", _tauPOGVVLoose2017v2, &b__tauPOGVVLoose2017v2);
	_currentTreePtr->SetBranchAddress("_tauPOGVTight2017v2", _tauPOGVTight2017v2, &b__tauPOGVTight2017v2);
	_currentTreePtr->SetBranchAddress("_tauPOGVVTight2017v2", _tauPOGVVTight2017v2, &b__tauPOGVVTight2017v2);
	_currentTreePtr->SetBranchAddress("_tauVLooseMvaNew2017v2", _tauVLooseMvaNew2017v2, &b__tauVLooseMvaNew2017v2);
	_currentTreePtr->SetBranchAddress("_tauLooseMvaNew2017v2", _tauLooseMvaNew2017v2, &b__tauLooseMvaNew2017v2);
	_currentTreePtr->SetBranchAddress("_tauMediumMvaNew2017v2", _tauMediumMvaNew2017v2, &b__tauMediumMvaNew2017v2);
	_currentTreePtr->SetBranchAddress("_tauTightMvaNew2017v2", _tauTightMvaNew2017v2, &b__tauTightMvaNew2017v2);
	_currentTreePtr->SetBranchAddress("_tauVTightMvaNew2017v2", _tauVTightMvaNew2017v2, &b__tauVTightMvaNew2017v2);
    }
    
    if( isMC() ){
	if( !containsGeneratorInfo() ){
	    std::string msg = "WARNING: input tree does not seem to contain generator info;";
	    msg.append( " will not read generator info branches!" );
	    std::cerr << msg << std::endl;
	} else{
	    _currentTreePtr->SetBranchAddress("_weight", &_weight, &b__weight);
	    _currentTreePtr->SetBranchAddress("_nLheWeights", &_nLheWeights, &b__nLheWeights);
	    _currentTreePtr->SetBranchAddress("_lheWeight", _lheWeight, &b__lheWeight);
	    _currentTreePtr->SetBranchAddress("_nPsWeights", &_nPsWeights, &b__nPsWeights);
	    _currentTreePtr->SetBranchAddress("_psWeight", _psWeight, &b__psWeight);
	    _currentTreePtr->SetBranchAddress("_nTrueInt", &_nTrueInt, &b__nTrueInt);
	    _currentTreePtr->SetBranchAddress("_lheHTIncoming", &_lheHTIncoming, &b__lheHTIncoming);
	    _currentTreePtr->SetBranchAddress("_lIsPrompt", _lIsPrompt, &b__lIsPrompt);
	    _currentTreePtr->SetBranchAddress("_lMatchPdgId", _lMatchPdgId, &b__lMatchPdgId);
	    _currentTreePtr->SetBranchAddress("_lMatchCharge", _lMatchCharge, &b__lMatchCharge);
	    _currentTreePtr->SetBranchAddress("_lMomPdgId",  _lMomPdgId, &b__lMomPdgId);
	    _currentTreePtr->SetBranchAddress("_lProvenance", _lProvenance, &b__lProvenance);
	    _currentTreePtr->SetBranchAddress("_lProvenanceCompressed", _lProvenanceCompressed, &b__lProvenanceCompressed);
	    _currentTreePtr->SetBranchAddress("_lProvenanceConversion", _lProvenanceConversion, &b__lProvenanceConversion);
	    _currentTreePtr->SetBranchAddress("_ttgEventType", &_ttgEventType, &b__ttgEventType);
	    _currentTreePtr->SetBranchAddress("_zgEventType", &_zgEventType, &b__zgEventType);
	}

	if( !containsGenParticles() ){
	    std::string msg = "WARNING: input tree does not seem to contain gen particle info;";
	    msg.append( " will not read gen particle branches!" );
	    std::cerr << msg << std::endl;
	} else{
	    _currentTreePtr->SetBranchAddress("_gen_met", &_gen_met, &b__gen_met);
	    _currentTreePtr->SetBranchAddress("_gen_metPhi", &_gen_metPhi, &b__gen_metPhi);
	    _currentTreePtr->SetBranchAddress("_gen_nL", &_gen_nL, &b__gen_nL);
	    _currentTreePtr->SetBranchAddress("_gen_lPt", _gen_lPt, &b__gen_lPt);
	    _currentTreePtr->SetBranchAddress("_gen_lEta", _gen_lEta, &b__gen_lEta);
	    _currentTreePtr->SetBranchAddress("_gen_lPhi", _gen_lPhi, &b__gen_lPhi);
	    _currentTreePtr->SetBranchAddress("_gen_lE", _gen_lE, &b__gen_lE);
	    _currentTreePtr->SetBranchAddress("_gen_lFlavor", _gen_lFlavor, &b__gen_lFlavor);
	    _currentTreePtr->SetBranchAddress("_gen_lCharge", _gen_lCharge, &b__gen_lCharge);
	    _currentTreePtr->SetBranchAddress("_gen_lMomPdg", _gen_lMomPdg, &b__gen_lMomPdg);
	    _currentTreePtr->SetBranchAddress("_gen_lIsPrompt", _gen_lIsPrompt, &b__gen_lIsPrompt);
	}

	if( !containsParticleLevel() ){
	    std::string msg = "WARNING: input tree does not seem to contain particle level info;";
            msg.append( " will not read particle level branches!" );
            std::cerr << msg << std::endl;
	} else{
	    _currentTreePtr->SetBranchAddress("_pl_met", &_pl_met, &b__pl_met);
	    _currentTreePtr->SetBranchAddress("_pl_metPhi", &_pl_metPhi, &b__pl_metPhi);
	    _currentTreePtr->SetBranchAddress("_pl_nL", &_pl_nL, &b__pl_nL);
	    _currentTreePtr->SetBranchAddress("_pl_lPt", _pl_lPt, &b__pl_lPt);
	    _currentTreePtr->SetBranchAddress("_pl_lEta", _pl_lEta, &b__pl_lEta);
	    _currentTreePtr->SetBranchAddress("_pl_lPhi", _pl_lPhi, &b__pl_lPhi);
	    _currentTreePtr->SetBranchAddress("_pl_lE", _pl_lE, &b__pl_lE);
	    _currentTreePtr->SetBranchAddress("_pl_lFlavor", _pl_lFlavor, &b__pl_lFlavor);
	    _currentTreePtr->SetBranchAddress("_pl_lCharge", _pl_lCharge, &b__pl_lCharge);
	    _currentTreePtr->SetBranchAddress("_pl_nJets", &_pl_nJets, &b__pl_nJets);
	    _currentTreePtr->SetBranchAddress("_pl_jetPt", _pl_jetPt, &b__pl_jetPt);
	    _currentTreePtr->SetBranchAddress("_pl_jetEta", _pl_jetEta, &b__pl_jetEta);
	    _currentTreePtr->SetBranchAddress("_pl_jetPhi", _pl_jetPhi, &b__pl_jetPhi);
	    _currentTreePtr->SetBranchAddress("_pl_jetE", _pl_jetE, &b__pl_jetE);
	    _currentTreePtr->SetBranchAddress("_pl_jetHadronFlavor", _pl_jetHadronFlavor, &b__pl_jetHadronFlavor);	
	}

	if( !containsPrefire() ){
	    std::string msg = "WARNING: input tree does not seem to contain prefire info;";
	    msg.append( " will not read prefire branches!" );
	    std::cerr << msg << std::endl;
	} else{
	    _currentTreePtr->SetBranchAddress("_prefireWeight", &_prefireWeight, &b__prefireWeight);
	    _currentTreePtr->SetBranchAddress("_prefireWeightDown", &_prefireWeightDown, &b__prefireWeightDown);
	    _currentTreePtr->SetBranchAddress("_prefireWeightUp", &_prefireWeightUp, &b__prefireWeightUp);
	}

	if( !containsPrefireComponents() ){
	    std::string msg = "WARNING: input tree does not seem to contain prefire component info;";
	    msg.append( " will not read prefire component branches!" );
	    std::cerr << msg << std::endl;
	} else{
	    _currentTreePtr->SetBranchAddress("_prefireWeightMuon", &_prefireWeightMuon, &b__prefireWeightMuon);
	    _currentTreePtr->SetBranchAddress("_prefireWeightMuonDown", &_prefireWeightMuonDown, &b__prefireWeightMuonDown);
	    _currentTreePtr->SetBranchAddress("_prefireWeightMuonUp", &_prefireWeightMuonUp, &b__prefireWeightMuonUp);
	    _currentTreePtr->SetBranchAddress("_prefireWeightECAL", &_prefireWeightECAL, &b__prefireWeightECAL);
	    _currentTreePtr->SetBranchAddress("_prefireWeightECALDown", &_prefireWeightECALDown, &b__prefireWeightECALDown);
	    _currentTreePtr->SetBranchAddress("_prefireWeightECALUp", &_prefireWeightECALUp, &b__prefireWeightECALUp);
	}
    }

    //add all individually stored triggers 
    // always reset triggers instead of rare case of combining primary datasets 
    // to prevent invalidating addresses set by setOutputTree
    if( resetTriggersAndFilters || _triggerMap.empty() ){
        initializeTriggerMap( _currentTreePtr );
    }
    setMapBranchAddresses( _currentTreePtr, _triggerMap, b__triggerMap );

    //add all individually stored MET filters
    // always reset filters instead of rare case of combining primary datasets 
    // to prevent invalidating addresses set by setOutputTree
    if( resetTriggersAndFilters || _MetFilterMap.empty() ){
        initializeMetFilterMap( _currentTreePtr );
    }
    setMapBranchAddresses( _currentTreePtr, _MetFilterMap, b__MetFilterMap );

    // add split JEC uncertainties
    initializeJecSourcesMaps( _currentTreePtr );
    setMapBranchAddresses( _currentTreePtr, _jetPt_JECSourcesUp, b__jetPt_JECSourcesUp );
    setMapBranchAddresses( _currentTreePtr, _jetPt_JECSourcesDown, b__jetPt_JECSourcesDown );
    setMapBranchAddresses( _currentTreePtr, _jetSmearedPt_JECSourcesUp, b__jetSmearedPt_JECSourcesUp );
    setMapBranchAddresses( _currentTreePtr, _jetSmearedPt_JECSourcesDown, b__jetSmearedPt_JECSourcesDown );
    setMapBranchAddresses( _currentTreePtr, _corrMETx_JECSourcesUp, b__corrMETx_JECSourcesUp );
    setMapBranchAddresses( _currentTreePtr, _corrMETx_JECSourcesDown, b__corrMETx_JECSourcesDown );
    setMapBranchAddresses( _currentTreePtr, _corrMETy_JECSourcesUp, b__corrMETy_JECSourcesUp );
    setMapBranchAddresses( _currentTreePtr, _corrMETy_JECSourcesDown, b__corrMETy_JECSourcesDown );
    initializeJecSourcesGroupedMaps( _currentTreePtr );
    setMapBranchAddresses( _currentTreePtr, _jetPt_JECGroupedUp, b__jetPt_JECGroupedUp );
    setMapBranchAddresses( _currentTreePtr, _jetPt_JECGroupedDown, b__jetPt_JECGroupedDown );
    setMapBranchAddresses( _currentTreePtr, _jetSmearedPt_JECGroupedUp, b__jetSmearedPt_JECGroupedUp );
    setMapBranchAddresses( _currentTreePtr, _jetSmearedPt_JECGroupedDown, b__jetSmearedPt_JECGroupedDown );
    setMapBranchAddresses( _currentTreePtr, _corrMETx_JECGroupedUp, b__corrMETx_JECGroupedUp );
    setMapBranchAddresses( _currentTreePtr, _corrMETx_JECGroupedDown, b__corrMETx_JECGroupedDown );
    setMapBranchAddresses( _currentTreePtr, _corrMETy_JECGroupedUp, b__corrMETy_JECGroupedUp );
    setMapBranchAddresses( _currentTreePtr, _corrMETy_JECGroupedDown, b__corrMETy_JECGroupedDown );
}


void TreeReader::setOutputTree( TTree* outputTree, 
				bool includeJECSources, 
				bool includeJECGrouped,
				bool includeTauInfo,
				bool includeGeneratorInfo,
				bool includeGenParticles,
				bool includePrefire,
				bool includePrefireComponents,
                                bool includeParticleLevel ){
    outputTree->Branch("_runNb",                        &_runNb,                        "_runNb/l");
    outputTree->Branch("_lumiBlock",                    &_lumiBlock,                    "_lumiBlock/l");
    outputTree->Branch("_eventNb",                      &_eventNb,                      "_eventNb/l");
    outputTree->Branch("_nVertex",                      &_nVertex,                      "_nVertex/i");

    outputTree->Branch("_passTrigger_e", &_passTrigger_e, "_passTrigger_e/O");
    outputTree->Branch("_passTrigger_ee", &_passTrigger_ee, "_passTrigger_ee/O");
    outputTree->Branch("_passTrigger_eee", &_passTrigger_eee, "_passTrigger_eee/O");
    outputTree->Branch("_passTrigger_em", &_passTrigger_em, "_passTrigger_em/O");
    outputTree->Branch("_passTrigger_m", &_passTrigger_m, "_passTrigger_m/O");
    outputTree->Branch("_passTrigger_eem", &_passTrigger_eem, "_passTrigger_eem/O");
    outputTree->Branch("_passTrigger_mm", &_passTrigger_mm, "_passTrigger_mm/O");
    outputTree->Branch("_passTrigger_emm", &_passTrigger_emm, "_passTrigger_emm/O");
    outputTree->Branch("_passTrigger_mmm", &_passTrigger_mmm, "_passTrigger_mmm/O");
    outputTree->Branch("_passTrigger_et", &_passTrigger_et, "_passTrigger_et/O");
    outputTree->Branch("_passTrigger_mt", &_passTrigger_mt, "_passTrigger_mt/O");
    outputTree->Branch("_passTrigger_FR", &_passTrigger_FR, "_passTrigger_FR/O");
    outputTree->Branch("_passTrigger_FR_iso", &_passTrigger_FR_iso, "_passTrigger_FR_iso/O");
    outputTree->Branch("_passTrigger_ref", &_passTrigger_ref, "_passTrigger_ref/O");
    outputTree->Branch("_passMETFilters", &_passMETFilters, "_passMETFilters/O");

    outputTree->Branch("_nL",                           &_nL,                           "_nL/i");
    outputTree->Branch("_nMu",                          &_nMu,                          "_nMu/i");
    outputTree->Branch("_nEle",                         &_nEle,                         "_nEle/i");
    outputTree->Branch("_nLight",                       &_nLight,                       "_nLight/i");
    outputTree->Branch("_nTau",                         &_nTau,                         "_nTau/i");
    outputTree->Branch("_lPt",                          &_lPt,                          "_lPt[_nL]/D");
    outputTree->Branch("_lPtCorr",                      &_lPtCorr,                      "_lPtCorr[_nLight]/D");
    outputTree->Branch("_lPtScaleUp",			&_lPtScaleUp,                   "_lPtScaleUp[_nLight]/D");
    outputTree->Branch("_lPtScaleDown",                 &_lPtScaleDown,                 "_lPtScaleDown[_nLight]/D");
    outputTree->Branch("_lPtResUp",                     &_lPtResUp,                     "_lPtResUp[_nLight]/D");
    outputTree->Branch("_lPtResDown",                   &_lPtResDown,                   "_lPtResDown[_nLight]/D");
    outputTree->Branch("_lEta",                         &_lEta,                         "_lEta[_nL]/D");
    outputTree->Branch("_lEtaSC",                       &_lEtaSC,                       "_lEtaSC[_nLight]/D");
    outputTree->Branch("_lPhi",                         &_lPhi,                         "_lPhi[_nL]/D");
    outputTree->Branch("_lE",                           &_lE,                           "_lE[_nL]/D");
    outputTree->Branch("_lECorr",                       &_lECorr,                       "_lECorr[_nLight]/D");
    outputTree->Branch("_lEScaleUp",                    &_lEScaleUp,                    "_lEScaleUp[_nLight]/D");
    outputTree->Branch("_lEScaleDown",                  &_lEScaleDown,                  "_lEScaleDown[_nLight]/D");
    outputTree->Branch("_lEResUp",                      &_lEResUp,                      "_lEResUp[_nLight]/D");
    outputTree->Branch("_lEResDown",                    &_lEResDown,                    "_lEResDown[_nLight]/D");
    outputTree->Branch("_lFlavor",                      &_lFlavor,                      "_lFlavor[_nL]/i");
    outputTree->Branch("_lCharge",                      &_lCharge,                      "_lCharge[_nL]/I");
    outputTree->Branch("_dxy",                          &_dxy,                          "_dxy[_nL]/D");
    outputTree->Branch("_dz",                           &_dz,                           "_dz[_nL]/D");
    outputTree->Branch("_3dIP",                         &_3dIP,                         "_3dIP[_nL]/D");
    outputTree->Branch("_3dIPSig",                      &_3dIPSig,                      "_3dIPSig[_nL]/D");
    outputTree->Branch("_lElectronSummer16MvaGP",       &_lElectronSummer16MvaGP,       "_lElectronSummer16MvaGP[_nLight]/F");
    outputTree->Branch("_lElectronSummer16MvaHZZ",      &_lElectronSummer16MvaHZZ,      "_lElectronSummer16MvaHZZ[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17Iso",        &_lElectronMvaFall17Iso,        "_lElectronMvaFall17Iso[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17NoIso",      &_lElectronMvaFall17NoIso,      "_lElectronMvaFall17NoIso[_nLight]/F");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWPLoose", &_lElectronPassMVAFall17NoIsoWPLoose, "_lElectronPassMVAFall17NoIsoWPLoose[_nLight]/O");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWP90", &_lElectronPassMVAFall17NoIsoWP90, "_lElectronPassMVAFall17NoIsoWP90[_nLight]/O");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWP80", &_lElectronPassMVAFall17NoIsoWP80, "_lElectronPassMVAFall17NoIsoWP80[_nLight]/O");
    outputTree->Branch("_lElectronPassEmu",             &_lElectronPassEmu,             "_lElectronPassEmu[_nLight]/O");
    outputTree->Branch("_lElectronPassConvVeto",        &_lElectronPassConvVeto,        "_lElectronPassConvVeto[_nLight]/O");
    outputTree->Branch("_lElectronChargeConst",         &_lElectronChargeConst,         "_lElectronChargeConst[_nLight]/O");
    outputTree->Branch("_lElectronMissingHits",         &_lElectronMissingHits,         "_lElectronMissingHits[_nLight]/i");
    outputTree->Branch("_lElectronEInvMinusPInv",       &_lElectronEInvMinusPInv,       "_lElectronEInvMinusPInv[_nLight]/D");
    outputTree->Branch("_lElectronHOverE",              &_lElectronHOverE,              "_lElectronHOverE[_nLight]/D");
    outputTree->Branch("_lElectronSigmaIetaIeta",       &_lElectronSigmaIetaIeta,       "_lElectronSigmaIetaIeta[_nLight]/D");
    outputTree->Branch("_leptonMvaTTH",                 &_leptonMvaTTH,                 "_leptonMvaTTH[_nLight]/D");
    outputTree->Branch("_leptonMvatZq",                 &_leptonMvatZq,                 "_leptonMvatZq[_nLight]/D");
    outputTree->Branch("_leptonMvaTOP",                 &_leptonMvaTOP,                 "_leptonMvaTOP[_nLight]/D");
    outputTree->Branch("_leptonMvaTOPUL",		&_leptonMvaTOPUL,		"_leptonMvaTOPUL[_nLight]/D");
    outputTree->Branch("_leptonMvaTOPv2UL",             &_leptonMvaTOPv2UL,		"_leptonMvaTOPv2UL[_nLight]/D");
    outputTree->Branch("_lPOGVeto",                     &_lPOGVeto,                     "_lPOGVeto[_nL]/O");
    outputTree->Branch("_lPOGLoose",                    &_lPOGLoose,                    "_lPOGLoose[_nL]/O");
    outputTree->Branch("_lPOGMedium",                   &_lPOGMedium,                   "_lPOGMedium[_nL]/O");
    outputTree->Branch("_lPOGTight",                    &_lPOGTight,                    "_lPOGTight[_nL]/O");
    outputTree->Branch("_relIso",                       &_relIso,                       "_relIso[_nLight]/D");
    outputTree->Branch("_relIso0p4",                    &_relIso0p4,                    "_relIso0p4[_nLight]/D");
    outputTree->Branch("_relIso0p4MuDeltaBeta",         &_relIso0p4MuDeltaBeta,         "_relIso0p4MuDeltaBeta[_nMu]/D");
    outputTree->Branch("_miniIso",                      &_miniIso,                      "_miniIso[_nLight]/D");
    outputTree->Branch("_miniIsoCharged",               &_miniIsoCharged,               "_miniIsoCharged[_nLight]/D");
    outputTree->Branch("_ptRel",                        &_ptRel,                        "_ptRel[_nLight]/D");
    outputTree->Branch("_ptRatio",                      &_ptRatio,                      "_ptRatio[_nLight]/D");
    outputTree->Branch("_closestJetCsvV2",              &_closestJetCsvV2,              "_closestJetCsvV2[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_b",          &_closestJetDeepCsv_b,          "_closestJetDeepCsv_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_bb",         &_closestJetDeepCsv_bb,         "_closestJetDeepCsv_bb[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_b",       &_closestJetDeepFlavor_b,       "_closestJetDeepFlavor_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_bb",      &_closestJetDeepFlavor_bb,      "_closestJetDeepFlavor_bb[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_lepb",    &_closestJetDeepFlavor_lepb,    "_closestJetDeepFlavor_lepb[_nLight]/D");
    outputTree->Branch("_selectedTrackMult",            &_selectedTrackMult,            "_selectedTrackMult[_nLight]/i");
    outputTree->Branch("_lMuonSegComp",                 &_lMuonSegComp,                 "_lMuonSegComp[_nMu]/D");
    outputTree->Branch("_lMuonTrackPt",                 &_lMuonTrackPt,                 "_lMuonTrackPt[_nMu]/D");
    outputTree->Branch("_lMuonTrackPtErr",              &_lMuonTrackPtErr,              "_lMuonTrackPtErr[_nMu]/D");

    outputTree->Branch("_nJets",                     &_nJets,                    "_nJets/i");
    outputTree->Branch("_jetPt",                     &_jetPt,                    "_jetPt[_nJets]/D");
    outputTree->Branch("_jetPt_JECUp",               &_jetPt_JECUp,              "_jetPt_JECUp[_nJets]/D");
    outputTree->Branch("_jetPt_JECDown",             &_jetPt_JECDown,            "_jetPt_JECDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt",              &_jetSmearedPt,             "_jetSmearedPt[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JECDown",      &_jetSmearedPt_JECDown,     "_jetSmearedPt_JECDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JECUp",        &_jetSmearedPt_JECUp,       "_jetSmearedPt_JECUp[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JERDown",      &_jetSmearedPt_JERDown,     "_jetSmearedPt_JERDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JERUp",        &_jetSmearedPt_JERUp,       "_jetSmearedPt_JERUp[_nJets]/D");
    outputTree->Branch("_jetEta",                    &_jetEta,                   "_jetEta[_nJets]/D");
    outputTree->Branch("_jetPhi",                    &_jetPhi,                   "_jetPhi[_nJets]/D");
    outputTree->Branch("_jetE",                      &_jetE,                     "_jetE[_nJets]/D");
    outputTree->Branch("_jetPt_Uncorrected",         &_jetPt_Uncorrected,        "_jetPt_Uncorrected[_nJets]/D");
    outputTree->Branch("_jetPt_L1",                  &_jetPt_L1,                 "_jetPt_L1[_nJets]/D");
    outputTree->Branch("_jetPt_L2",                  &_jetPt_L2,                 "_jetPt_L2[_nJets]/D");
    outputTree->Branch("_jetPt_L3",                  &_jetPt_L3,                 "_jetPt_L3[_nJets]/D");
    outputTree->Branch("_jetCsvV2",                  &_jetCsvV2,                 "_jetCsvV2[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_udsg",           &_jetDeepCsv_udsg,          "_jetDeepCsv_udsg[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_b",              &_jetDeepCsv_b,             "_jetDeepCsv_b[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_c",              &_jetDeepCsv_c,             "_jetDeepCsv_c[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_bb",             &_jetDeepCsv_bb,            "_jetDeepCsv_bb[_nJets]/D");
    outputTree->Branch("_jetDeepFlavor_b",           &_jetDeepFlavor_b,          "_jetDeepFlavor_b[_nJets]/D");
    outputTree->Branch("_jetDeepFlavor_bb",          &_jetDeepFlavor_bb,         "_jetDeepFlavor_bb[_nJets]/D");
    outputTree->Branch("_jetDeepFlavor_lepb",        &_jetDeepFlavor_lepb,       "_jetDeepFlavor_lepb[_nJets]/D");
    outputTree->Branch("_jetHadronFlavor",           &_jetHadronFlavor,          "_jetHadronFlavor[_nJets]/i");
    outputTree->Branch("_jetIsTight",                &_jetIsTight,               "_jetIsTight[_nJets]/O");
    outputTree->Branch("_jetIsTightLepVeto",         &_jetIsTightLepVeto,        "_jetIsTightLepVeto[_nJets]/O");
    outputTree->Branch("_jetNeutralHadronFraction",  &_jetNeutralHadronFraction, "_jetNeutralHadronFraction[_nJets]/D");
    outputTree->Branch("_jetChargedHadronFraction",  &_jetChargedHadronFraction, "_jetChargedHadronFraction[_nJets]/D");
    outputTree->Branch("_jetNeutralEmFraction",      &_jetNeutralEmFraction,     "_jetNeutralEmFraction[_nJets]/D");
    outputTree->Branch("_jetChargedEmFraction",      &_jetChargedEmFraction,     "_jetChargedEmFraction[_nJets]/D");
    outputTree->Branch("_jetHFHadronFraction",       &_jetHFHadronFraction,      "_jetHFHadronFraction[_nJets]/D");
    outputTree->Branch("_jetHFEmFraction",           &_jetHFEmFraction,          "_jetHFEmFraction[_nJets]/D");

    outputTree->Branch("_met",                          &_met,                          "_met/D");
    outputTree->Branch("_met_JECDown",                  &_met_JECDown,                  "_met_JECDown/D");
    outputTree->Branch("_met_JECUp",                    &_met_JECUp,                    "_met_JECUp/D");
    outputTree->Branch("_met_UnclDown",                 &_met_UnclDown,                 "_met_UnclDown/D");
    outputTree->Branch("_met_UnclUp",                   &_met_UnclUp,                   "_met_UnclUp/D");
    outputTree->Branch("_metPhi",                       &_metPhi,                       "_metPhi/D");
    outputTree->Branch("_metPhi_JECDown",               &_metPhi_JECDown,               "_metPhi_JECDown/D");
    outputTree->Branch("_metPhi_JECUp",                 &_metPhi_JECUp,                 "_metPhi_JECUp/D");
    outputTree->Branch("_metPhi_UnclDown",              &_metPhi_UnclDown,              "_metPhi_UnclDown/D");
    outputTree->Branch("_metPhi_UnclUp",                &_metPhi_UnclUp,                "_metPhi_UnclUp/D");
    outputTree->Branch("_metSignificance",              &_metSignificance,              "_metSignificance/D");

    if( includeTauInfo ){
        if( !containsTauInfo() ){
            std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include tau info in output tree,");
            msg.append(" but there appear to be no tau info branches in the input tree;");
            msg.append(" will skip writing tau info to output tree!");
            std::cerr << msg << std::endl;
        }
        else{
            outputTree->Branch("_tauDecayMode",                 &_tauDecayMode,                 "_tauDecayMode[_nL]/i");
            outputTree->Branch("_decayModeFinding",             &_decayModeFinding,             "_decayModeFinding[_nL]/O");
            outputTree->Branch("_decayModeFindingNew",          &_decayModeFindingNew,          "_decayModeFindingNew[_nL]/O");
            outputTree->Branch("_tauPOGVLoose2015",             &_tauPOGVLoose2015,             "_tauPOGVLoose2015[_nL]/O");
            outputTree->Branch("_tauPOGLoose2015",              &_tauPOGLoose2015,              "_tauPOGLoose2015[_nL]/O");
            outputTree->Branch("_tauPOGMedium2015",             &_tauPOGMedium2015,             "_tauPOGMedium2015[_nL]/O");
            outputTree->Branch("_tauPOGTight2015",              &_tauPOGTight2015,              "_tauPOGTight2015[_nL]/O");
            outputTree->Branch("_tauPOGVTight2015",             &_tauPOGVTight2015,             "_tauPOGVTight2015[_nL]/O");
            outputTree->Branch("_tauVLooseMvaNew2015",          &_tauVLooseMvaNew2015,          "_tauVLooseMvaNew2015[_nL]/O");
            outputTree->Branch("_tauLooseMvaNew2015",           &_tauLooseMvaNew2015,           "_tauLooseMvaNew2015[_nL]/O");
            outputTree->Branch("_tauMediumMvaNew2015",          &_tauMediumMvaNew2015,          "_tauMediumMvaNew2015[_nL]/O");
            outputTree->Branch("_tauTightMvaNew2015",           &_tauTightMvaNew2015,           "_tauTightMvaNew2015[_nL]/O");
            outputTree->Branch("_tauVTightMvaNew2015",          &_tauVTightMvaNew2015,          "_tauVTightMvaNew2015[_nL]/O");
            outputTree->Branch("_tauPOGVVLoose2017v2",          &_tauPOGVVLoose2017v2,          "_tauPOGVVLoose2017v2[_nL]/O");
            outputTree->Branch("_tauPOGVTight2017v2",           &_tauPOGVTight2017v2,           "_tauPOGVTight2017v2[_nL]/O");
            outputTree->Branch("_tauPOGVVTight2017v2",          &_tauPOGVVTight2017v2,          "_tauPOGVVTight2017v2[_nL]/O");
            outputTree->Branch("_tauVLooseMvaNew2017v2",        &_tauVLooseMvaNew2017v2,        "_tauVLooseMvaNew2017v2[_nL]/O");
            outputTree->Branch("_tauLooseMvaNew2017v2",         &_tauLooseMvaNew2017v2,         "_tauLooseMvaNew2017v2[_nL]/O");
            outputTree->Branch("_tauMediumMvaNew2017v2",        &_tauMediumMvaNew2017v2,        "_tauMediumMvaNew2017v2[_nL]/O");
            outputTree->Branch("_tauTightMvaNew2017v2",         &_tauTightMvaNew2017v2,         "_tauTightMvaNew2017v2[_nL]/O");
            outputTree->Branch("_tauVTightMvaNew2017v2",        &_tauVTightMvaNew2017v2,        "_tauVTightMvaNew2017v2[_nL]/O");
            outputTree->Branch("_tauMuonVetoLoose",             &_tauMuonVetoLoose,             "_tauMuonVetoLoose[_nL]/O");
            outputTree->Branch("_tauMuonVetoTight",             &_tauMuonVetoTight,             "_tauMuonVetoTight[_nL]/O");
            outputTree->Branch("_tauEleVetoVLoose",             &_tauEleVetoVLoose,             "_tauEleVetoVLoose[_nL]/O");
            outputTree->Branch("_tauEleVetoLoose",              &_tauEleVetoLoose,              "_tauEleVetoLoose[_nL]/O");
            outputTree->Branch("_tauEleVetoMedium",             &_tauEleVetoMedium,             "_tauEleVetoMedium[_nL]/O");
            outputTree->Branch("_tauEleVetoTight",              &_tauEleVetoTight,              "_tauEleVetoTight[_nL]/O");
            outputTree->Branch("_tauEleVetoVTight",             &_tauEleVetoVTight,             "_tauEleVetoVTight[_nL]/O");
        }
    }

    if( isMC() && includeGeneratorInfo ){
	if( !containsGeneratorInfo() ){
            std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include generator info in output tree,");
            msg.append(" but there appear to be no generator info branches in the input tree;");
            msg.append(" will skip writing generator info to output tree!");
            std::cerr << msg << std::endl;
        }
	else{
	    outputTree->Branch("_weight",                    &_weight,                    "_weight/D");
	    outputTree->Branch("_nLheWeights",               &_nLheWeights,               "_nLheWeights/i");
	    outputTree->Branch("_lheWeight",                 &_lheWeight,                 "_lheWeight[_nLheWeights]/D");
	    outputTree->Branch("_nPsWeights",                &_nPsWeights,                "_nPsWeights/i");
	    outputTree->Branch("_psWeight",                  &_psWeight,                  "_psWeight[_nPsWeights]/D");
	    outputTree->Branch("_nTrueInt",                  &_nTrueInt,                  "_nTrueInt/F");
	    outputTree->Branch("_lheHTIncoming",             &_lheHTIncoming,             "_lheHTIncoming/D");
	    outputTree->Branch("_lIsPrompt",                 &_lIsPrompt,                 "_lIsPrompt[_nL]/O");
	    outputTree->Branch("_lMatchPdgId",               &_lMatchPdgId,               "_lMatchPdgId[_nL]/I");
	    outputTree->Branch("_lMatchCharge",              &_lMatchCharge,              "_lMatchCharge[_nL]/I");
	    outputTree->Branch("_lMomPdgId",                 &_lMomPdgId,                 "_lMomPdgId[_nL]/I");
	    outputTree->Branch("_lProvenance",               &_lProvenance,               "_lProvenance[_nL]/i");
	    outputTree->Branch("_lProvenanceCompressed",     &_lProvenanceCompressed,     "_lProvenanceCompressed[_nL]/i");
	    outputTree->Branch("_lProvenanceConversion",     &_lProvenanceConversion,     "_lProvenanceConversion[_nL]/i");
	    outputTree->Branch("_ttgEventType",              &_ttgEventType,              "_ttgEventType/i");
	    outputTree->Branch("_zgEventType",               &_zgEventType,               "_zgEventType/i");
	}
    }

    if( isMC() && includeGenParticles){
	if( !containsGenParticles() ){
            std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include gen particles in output tree,");
            msg.append(" but there appear to be no gen particle branches in the input tree;");
            msg.append(" will skip writing gen particles to output tree!");
            std::cerr << msg << std::endl;
        }
	else{
	    outputTree->Branch("_gen_met",                   &_gen_met,                   "_gen_met/D");
	    outputTree->Branch("_gen_metPhi",                &_gen_metPhi,                "_gen_metPhi/D");
	    outputTree->Branch("_gen_nL",                    &_gen_nL,                    "_gen_nL/i");
	    outputTree->Branch("_gen_lPt",                   &_gen_lPt,                   "_gen_lPt[_gen_nL]/D");
	    outputTree->Branch("_gen_lEta",                  &_gen_lEta,                  "_gen_lEta[_gen_nL]/D");
	    outputTree->Branch("_gen_lPhi",                  &_gen_lPhi,                  "_gen_lPhi[_gen_nL]/D");
	    outputTree->Branch("_gen_lE",                    &_gen_lE,                    "_gen_lE[_gen_nL]/D");
	    outputTree->Branch("_gen_lFlavor",               &_gen_lFlavor,               "_gen_lFlavor[_gen_nL]/i");
	    outputTree->Branch("_gen_lCharge",               &_gen_lCharge,               "_gen_lCharge[_gen_nL]/I");
	    outputTree->Branch("_gen_lMomPdg",               &_gen_lMomPdg,               "_gen_lMomPdg[_gen_nL]/I");
	    outputTree->Branch("_gen_lIsPrompt",             &_gen_lIsPrompt,             "_gen_lIsPrompt[_gen_nL]/O");
	}
    }

    if( isMC() && includeParticleLevel){
        if( !containsParticleLevel() ){
            std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include particle level in output tree,");
            msg.append(" but there appear to be no particle level branches in the input tree;");
            msg.append(" will skip writing particles level to output tree!");
            std::cerr << msg << std::endl;
        }
        else{
	    outputTree->Branch("_pl_met", &b__pl_met, "_pl_met/D");
            outputTree->Branch("_pl_metPhi", &b__pl_metPhi, "_pl_metPhi/D");
            outputTree->Branch("_pl_nL", &b__pl_nL, "_pl_nL/i");
            outputTree->Branch("_pl_lPt", &b__pl_lPt, "_pl_lPt[_pl_nL]/D");
            outputTree->Branch("_pl_lEta", &b__pl_lEta, "_pl_lEta[_pl_nL]/D");
            outputTree->Branch("_pl_lPhi", &b__pl_lPhi, "_pl_lPhi[_pl_nL]/D");
            outputTree->Branch("_pl_lE", &b__pl_lE, "_pl_lE[_pl_nL]/D");
            outputTree->Branch("_pl_lFlavor", &b__pl_lFlavor, "_pl_lFlavor[_pl_nL]/i");
            outputTree->Branch("_pl_lCharge", &b__pl_lCharge, "_pl_lCharge[_pl_nL]/I");
            outputTree->Branch("_pl_nJets", &b__pl_nJets, "_pl_nJets/i");
            outputTree->Branch("_pl_jetPt", &b__pl_jetPt, "_pl_jetPt[_pl_nJets]/D");
            outputTree->Branch("_pl_jetEta", &b__pl_jetEta, "_pl_jetEta[_pl_nJets]/D");
            outputTree->Branch("_pl_jetPhi", &b__pl_jetPhi, "_pl_jetPhi[_pl_nJets]/D");
            outputTree->Branch("_pl_jetE", &b__pl_jetE, "_pl_jetE[_pl_nJets]/D");
            outputTree->Branch("_pl_jetHadronFlavor", &b__pl_jetHadronFlavor, "pl_jetHadronFlavor[_pl_nJets]/i");
	}
    }

    if( isMC() && includePrefire ){
	if( !containsPrefire() ){
	    std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include prefire weights in output tree,");
            msg.append(" but there appear to be no prefire weight branches in the input tree;");
            msg.append(" will skip writing prefire weights to output tree!");
            std::cerr << msg << std::endl;
	}
	else{
	    outputTree->Branch("_prefireWeight",             &_prefireWeight,             "_prefireWeight/F");
	    outputTree->Branch("_prefireWeightUp",           &_prefireWeightUp,           "_prefireWeightUp/F");
	    outputTree->Branch("_prefireWeightDown",         &_prefireWeightDown,         "_prefireWeightDown/F"); 
	}
    }

    if( isMC() && includePrefireComponents ){
        if( !containsPrefireComponents() ){
            std::string msg = "WARNING in TreeReader.setOutputTree:";
            msg.append(" requested to include prefire component weights in output tree,");
            msg.append(" but there appear to be no prefire component weight branches in the input tree;");
            msg.append(" will skip writing prefire component weights to output tree!");
            std::cerr << msg << std::endl;
        }
        else{
            outputTree->Branch("_prefireWeightMuon",             &_prefireWeightMuon,             "_prefireWeightMuon/F");
            outputTree->Branch("_prefireWeightMuonUp",           &_prefireWeightMuonUp,           "_prefireWeightMuonUp/F");
            outputTree->Branch("_prefireWeightMuonDown",         &_prefireWeightMuonDown,         "_prefireWeightMuonDown/F");
	    outputTree->Branch("_prefireWeightECAL",             &_prefireWeightECAL,             "_prefireWeightECAL/F");
            outputTree->Branch("_prefireWeightECALUp",           &_prefireWeightECALUp,           "_prefireWeightECALUp/F");
            outputTree->Branch("_prefireWeightECALDown",         &_prefireWeightECALDown,         "_prefireWeightECALDown/F");
        }
    }

    if( containsSusyMassInfo() ){
	outputTree->Branch("_mChi1", &_mChi1, "_mChi1/D");
	outputTree->Branch("_mChi2", &_mChi2, "_mChi2/D");
    }

    //write individual trigger decisions to output tree 
    setMapOutputBranches( outputTree, _triggerMap, "/O" );

    //write individual MET filters to output tree
    setMapOutputBranches( outputTree, _MetFilterMap, "/O" );

    // write split JEC uncertainties to output tree
    if( includeJECSources ){
	setMapOutputBranches( outputTree, _jetPt_JECSourcesUp, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetPt_JECSourcesDown, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetSmearedPt_JECSourcesUp, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetSmearedPt_JECSourcesDown, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _corrMETx_JECSourcesUp, "/D");
        setMapOutputBranches( outputTree, _corrMETx_JECSourcesDown, "/D");
        setMapOutputBranches( outputTree, _corrMETy_JECSourcesUp, "/D");
        setMapOutputBranches( outputTree, _corrMETy_JECSourcesDown, "/D");
    }
    if( includeJECGrouped ){
	setMapOutputBranches( outputTree, _jetPt_JECGroupedUp, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetPt_JECGroupedDown, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetSmearedPt_JECGroupedUp, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _jetSmearedPt_JECGroupedDown, "[_nJets]/D" );
	setMapOutputBranches( outputTree, _corrMETx_JECGroupedUp, "/D");
	setMapOutputBranches( outputTree, _corrMETx_JECGroupedDown, "/D");
	setMapOutputBranches( outputTree, _corrMETy_JECGroupedUp, "/D");
	setMapOutputBranches( outputTree, _corrMETy_JECGroupedDown, "/D");
    }
}


//get object from current file 
TObject* TreeReader::getFromCurrentFile( const std::string& name ) const{
    checkCurrentFile();
    return _currentFilePtr->Get( name.c_str() );
}


//Get list of histograms stored in current file
std::vector< std::shared_ptr< TH1 > > TreeReader::getHistogramsFromCurrentFile() const{

    checkCurrentFile();

    //vector containing all histograms in current file
    std::vector< std::shared_ptr< TH1 > > histogramVector;

    //loop over keys in blackJackAndHookers directory
    // old comment from Willem:
    // "this directory gets implicitly deleted by root when the current root file gets deleted. 
    // This can NOT be a shared_ptr since this directory will also make the root file inaccessible 
    // upon deletion (DIRTY ROOT!!!)"
    TDirectory* dir = (TDirectory*) _currentFilePtr->Get("blackJackAndHookers");
    // old comment from Willem:
    // "this is not a memory leak since this object will implicitly be deleted 
    // when 'dir' gets deleted (DIRTY ROOT!!!)"
    TList* keyList = dir->GetListOfKeys();

    for( const auto objectPtr : *keyList ){

	//try if a dynamic_cast to a histogram works to check if object is histogram
	TH1* histPtr = dynamic_cast< TH1* >( dir->Get( objectPtr->GetName() ) );
	if( histPtr ){
            //make sure histograms don't get deleted by root upon deletion of TDirectory above
            histPtr->SetDirectory( gROOT );
	    histogramVector.emplace_back( histPtr );
	}
    }
    return histogramVector;
}


void TreeReader::removeBSMSignalSamples(){
    // remove some samples from the SampleVector of this TreeReader
    // WARNING: the SampleVector is modified in-place.
    // WARNING: might be analysis-specific (for ewkino), check isNewPhysicsSignal()
    for( auto it = samples.begin(); it != samples.end(); ){
        if( it->isNewPhysicsSignal() ){
            it = samples.erase( it );
        } else ++it;
    }
}


void TreeReader::keepOnlySignalsWithName( const std::string& signalName ){
    // remove some samples from the SampleVector of this TreeReader
    // WARNING: the SampleVector is modified in-place.
    // WARNING: might be analysis-specific (for ewkino), check isNewPhysicsSignal()
    for( auto it = samples.begin(); it != samples.end(); ){
        if( it->isNewPhysicsSignal() && it->processName() != signalName ){
            it = samples.erase( it );
        } else ++it;
    }
}
