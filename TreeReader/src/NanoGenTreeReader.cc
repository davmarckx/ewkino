#include "../interface/NanoGenTreeReader.h"
#include "../../Event/interface/Event.h"


// constructor //

NanoGenTreeReader::NanoGenTreeReader( const std::string& sampleListFile, const std::string& sampleDirectory ){
    readSamples( sampleListFile, sampleDirectory );
}


// functions for sample reading //

void NanoGenTreeReader::readSamples( const std::string& list, 
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


void NanoGenTreeReader::readSamples( const std::string& list, const std::string& directory ){
    readSamples( list, directory, this->samples );
}


// functions to find if a tree has branches with certain types of info //

bool nanoTreeHasBranchWithName( TTree* treePtr, const std::string& nameToFind ){
    TObjArray* branch_list = treePtr->GetListOfBranches();
    for( const auto& branchPtr : *branch_list ){
	std::string branchName = branchPtr->GetName();
	    if( stringTools::stringContains( branchName, nameToFind ) ){
		return true;
	    }
    }
    return false;
}


bool NanoGenTreeReader::containsGeneratorInfo() const{
    return nanoTreeHasBranchWithName( _currentTreePtr, "genWeight");
}


bool NanoGenTreeReader::containsGenDressedLeptons() const{
    return nanoTreeHasBranchWithName( _currentTreePtr, "nGenDressedLepton");
}


bool NanoGenTreeReader::containsGenJets() const{
    return nanoTreeHasBranchWithName( _currentTreePtr, "nGenJet" );
}


bool NanoGenTreeReader::containsGenMET() const{
    return nanoTreeHasBranchWithName( _currentTreePtr, "GenMET_pt" );
}


bool NanoGenTreeReader::isData() const{
    if( _currentSamplePtr ) return _currentSamplePtr->isData();
    else return !containsGeneratorInfo();
}


bool NanoGenTreeReader::isMC() const{
    return !isData();
}


void NanoGenTreeReader::checkCurrentSample() const{
    if( !_currentSamplePtr ){
        throw std::domain_error( "pointer to current Sample is nullptr." );
    }
}


void NanoGenTreeReader::checkCurrentTree() const{
    if( !_currentTreePtr ){
        throw std::domain_error( "pointer to current TTree is nullptr." );
    }
}


void NanoGenTreeReader::checkCurrentFile() const{
    if( !_currentFilePtr ){
	throw std::domain_error( "pointer to current TFile is nullptr." );
    }
}


bool NanoGenTreeReader::is2016() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016();
}


bool NanoGenTreeReader::is2016PreVFP() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016PreVFP();
}


bool NanoGenTreeReader::is2016PostVFP() const{
    checkCurrentSample();
    return _currentSamplePtr->is2016PostVFP();
}


bool NanoGenTreeReader::is2017() const{
    checkCurrentSample();
    return _currentSamplePtr->is2017();
}


bool NanoGenTreeReader::is2018() const{
    checkCurrentSample();
    return _currentSamplePtr->is2018();
}


std::string NanoGenTreeReader::getYearString() const{
    if( is2016PreVFP() ) return "2016PreVFP";
    else if( is2016PostVFP() ) return "2016PostVFP";
    else if( is2016() ) return "2016";
    else if( is2017() ) return "2017";
    else if( is2018() ) return "2018";
    else{
	std::string msg = "ERROR in NanoGenTreeReader::getYearString:";
	msg += " no valid year string could be returned as all year checks failed.";
	throw std::runtime_error(msg);
    }
}


long unsigned NanoGenTreeReader::numberOfEntries() const{
    checkCurrentTree();
    return _currentTreePtr->GetEntries();
}


void NanoGenTreeReader::initSample( const Sample& samp, const bool doInitTree ){ 
    // set current Sample, File and Tree pointers
    _currentSamplePtr = std::make_shared< Sample >( samp );
    _currentFilePtr = samp.filePtr();
    // note: when initializing multiple samples in series,
    // the currentTreePtr below gets overwritten by the new tree.
    // normally, this implies a memory leak, but in this case it is fine,
    // since ROOT implicitly deletes the previous tree once the file is closed
    // (which is again implicitly done when opening the new file).
    _currentTreePtr = (TTree*) _currentFilePtr->Get( "Events" );
    checkCurrentTree();
    // initialize the input tree
    if( doInitTree ) initTree();
    // printouts for testing
    std::cout << "INFO: NanoGenTreeReader initialized with sample: ";
    std::cout << _currentSamplePtr.get() << std::endl;
    if( !samp.isData() ){
        // get luminosity 
        double lumi;
        if( is2016() ){ lumi = lumi::lumi2016; } 
	else if( is2016PreVFP() ){ lumi = lumi::lumi2016PreVFP; }
	else if( is2016PostVFP() ){ lumi = lumi::lumi2016PostVFP; }
        else if( is2017() ){ lumi = lumi::lumi2017; } 
	else if( is2018() ){ lumi = lumi::lumi2018; }
	else{
	    std::string msg = "ERROR in NanoGenTreeReader::initSample:";
	    msg += " no valid year could be identified!";
	    throw std::runtime_error(msg);
	}
	// get cross-section
	double xsection = samp.xSec();
	// get sum of gen weights
	// note: the sum of gen weights is stored in a branch called genEventSumw.
	//       this branch may contain multiple entries, e.g. when files are merged,
	//	 so we need to take the sum over all entries in this branch!
	TTree* runTreePtr = (TTree*) _currentFilePtr->Get( "Runs" );
	if( !runTreePtr ){
	    std::string msg = "ERROR in NanoGenTreeReader::initSample:";
	    msg += " tree with sum-of-gen-weight information requested";
	    msg += " but does not seem to exist.";
	    throw std::runtime_error(msg);
	}
	_sumGenWeights = 0;
	_numGenEvents = 0;
	Double_t genEventSumw = 0;
	Long64_t genEventCount = 0;
	runTreePtr->SetBranchAddress("genEventSumw", &genEventSumw);
	runTreePtr->SetBranchAddress("genEventCount", &genEventCount);
	for( long int entry=0; entry < runTreePtr->GetEntries(); entry++ ){
	    runTreePtr->GetEntry(entry);
	    _sumGenWeights += genEventSumw;
	    _numGenEvents += genEventCount;
	}
	// calculate the scale
        _weightScale = xsection*lumi*1000 / _sumGenWeights;
	// printouts for testing
	std::cout << "INFO NanoGenTreeReader found following sample paramters:" << std::endl;
	std::cout << " - lumi: " << lumi << std::endl;
	std::cout << " - xsec: " << xsection << std::endl;
	std::cout << " - sumweights: " << _sumGenWeights << std::endl;
	// get sum of scale and PDF varied weights
	// note: the branch LHEScaleSumw contains, for each index i,
	//       the sum of genEventWeight * LHEScaleWeight[i], divided by genEventSumw
	//       (see https://cms-nanoaod-integration.web.cern.ch/autoDoc/).
	//       to correctly handle the case of multiple entries in this branch,
	//	 we need to multiply each entry by its genEventSumw, and divide by the total
	//       after adding all entries!
	//	 the same holds for LHEPdfSumw
	Double_t LHEScaleSumw[nLHEScaleWeight_max];
	UInt_t nLHEScaleSumw;
	Double_t LHEPdfSumw[nLHEPdfWeight_max];
	UInt_t nLHEPdfSumw;
	runTreePtr->SetBranchAddress("LHEScaleSumw", LHEScaleSumw);
	runTreePtr->SetBranchAddress("nLHEScaleSumw", &nLHEScaleSumw);
	runTreePtr->SetBranchAddress("LHEPdfSumw", LHEPdfSumw);
	runTreePtr->SetBranchAddress("nLHEPdfSumw", &nLHEPdfSumw);
	for( long int entry=0; entry < runTreePtr->GetEntries(); entry++ ){
            runTreePtr->GetEntry(entry);
	    if( entry==0 ){
		_nSumLHEScaleWeights = nLHEScaleSumw;
		_nSumLHEPdfWeights = nLHEPdfSumw;
	    }
	    // check consistency of counters (should be the same for all entries)
            if( entry>0 && nLHEScaleSumw!=_nSumLHEScaleWeights){
		std::string msg = "ERROR in NanoGenTreeReader::initSample:";
		msg += " inconsistent number of LHE scale weights in Runs tree.";
		throw std::runtime_error(msg);
	    }
            if( entry>0 && nLHEPdfSumw!=_nSumLHEPdfWeights){
                std::string msg = "ERROR in NanoGenTreeReader::initSample:";
                msg += " inconsistent number of LHE PDF weights in Runs tree.";
                throw std::runtime_error(msg);
            }
	    // add this entry to the total
	    for( unsigned int idx=0; idx<_nSumLHEScaleWeights; idx++ ){
		_sumLHEScaleWeights[idx] += LHEScaleSumw[idx] * genEventSumw;
	    }
	    for( unsigned int idx=0; idx<_nSumLHEPdfWeights; idx++ ){
                _sumLHEPdfWeights[idx] += LHEPdfSumw[idx] * genEventSumw;
            }
        }
	// divide by the total sum of nominal gen weights
	for( unsigned int idx=0; idx<_nSumLHEScaleWeights; idx++ ){
            _sumLHEScaleWeights[idx] /= _sumGenWeights;
        }
        for( unsigned int idx=0; idx<_nSumLHEPdfWeights; idx++ ){
            _sumLHEPdfWeights[idx] /= _sumGenWeights;
        }
    }
}


// initialize a sample at a given index in the list
void NanoGenTreeReader::initSample( unsigned int sampleIndex,
			     const bool doInitTree ){
    currentSampleIndex = sampleIndex;
    initSample( samples[ sampleIndex ], doInitTree );
}

// initialize the next sample in the list
void NanoGenTreeReader::initSample( const bool doInitTree ){
    initSample( samples[ ++currentSampleIndex ], doInitTree );
}


// initialize the current Sample directly from a root file, this is used when skimming
void NanoGenTreeReader::initSampleFromFile( const std::string& pathToFile, 
				     const bool is2016, 
				     const bool is2016PreVFP,
				     const bool is2016PostVFP,
				     const bool is2017, 
				     const bool is2018 ){

    // check if file exists 
    if( !systemTools::fileExists( pathToFile ) ){
	std::string msg = "ERROR in NanoGenTreeReader.initSampleFromFile:";
	msg += " file '" + pathToFile + "' does not exist.";
        throw std::invalid_argument(msg);
	// (one might suppress this error for files read from DAS)
    }

    // check year
    if( !(is2016 || is2016PreVFP || is2016PostVFP || is2017 || is2018 ) ){
        std::string msg = "ERROR in NanoGenTreeReader::initSampleFromFile:";
        msg += " no valid year was given for sample ";
        msg += pathToFile;
        throw std::runtime_error(msg);
    }

    // open file
    std::cout << "INFO in NanoGenTreeReader::initSampleFromFile: opening " << pathToFile << std::endl;
    _currentFilePtr = std::shared_ptr< TFile >( TFile::Open( pathToFile.c_str(), "READ" ) );

    // set tree pointer
    // note: when initializing multiple samples in series,
    // the currentTreePtr below gets overwritten by the new tree.
    // normally, this implies a memory leak, but in this case it is fine,
    // since ROOT implicitly deletes the previous tree once the file is closed
    // (which is again implicitly done when opening the new file).
    _currentTreePtr = (TTree*) _currentFilePtr->Get( "Events" );
    checkCurrentTree();

    // make a new sample, and make sure the pointer remains valid
    _currentSamplePtr = std::make_shared< Sample >( pathToFile, is2016, is2016PreVFP,
			    is2016PostVFP, is2017, is2018, isData() );

    // initialize tree
    initTree();

    // set scale to default so weights do not become 0 when building the event
    _weightScale = 1.;
}


// automatically determine whether sample is 2017 or 2018 from file name 
void NanoGenTreeReader::initSampleFromFile( const std::string& pathToFile ){
    bool is2016 = analysisTools::fileIs2016( pathToFile );
    bool is2016PreVFP = analysisTools::fileIs2016PreVFP( pathToFile );
    bool is2016PostVFP = analysisTools::fileIs2016PostVFP( pathToFile );
    bool is2017 = analysisTools::fileIs2017( pathToFile );
    bool is2018 = analysisTools::fileIs2018( pathToFile );
    initSampleFromFile( pathToFile, is2016, is2016PreVFP, is2016PostVFP, is2017, is2018 );
}


void NanoGenTreeReader::GetEntry( const Sample& samp, long unsigned entry ){
    checkCurrentTree();
    _currentTreePtr->GetEntry( entry );
    // do some checks on array lengths
    if( _nGenDressedLepton > nGenDressedLepton_max ){
	std::string msg = "WARNING in NanoGenTreeReader::GetEntry:";
	msg += " nGenDressedLepton is " + std::to_string(_nGenDressedLepton);
	msg += " while nGenDressedLepton_max is; " + std::to_string(nGenDressedLepton_max);
	msg += " will ignore objects with index >= " +std::to_string(nGenDressedLepton_max);
	std::cerr << msg << std::endl;
	_nGenDressedLepton = nGenDressedLepton_max;
    }
    if( _nGenJet > nGenJet_max ){
        std::string msg = "WARNING in NanoGenTreeReader::GetEntry:";
        msg += " nGenJet is " + std::to_string(_nGenJet);
        msg += " while nGenJet_max is; " + std::to_string(nGenJet_max);
        msg += " will ignore objects with index >= " +std::to_string(nGenJet_max);
	std::cerr << msg << std::endl;
        _nGenJet = nGenJet_max;
    }
    // set correctly scaled weight
    if( !samp.isData() ) _scaledWeight = _genWeight*_weightScale;
    else _scaledWeight = 1;
}


// use the currently initialized sample when running in serial
void NanoGenTreeReader::GetEntry( long unsigned entry ){
    GetEntry( *_currentSamplePtr, entry );
}


Event NanoGenTreeReader::buildEvent( const Sample& samp, 
	long unsigned entry ){
    GetEntry( samp, entry );
    return Event( *this );
}


Event NanoGenTreeReader::buildEvent( long unsigned entry ){
    GetEntry( entry );
    return Event( *this );
}


void NanoGenTreeReader::initTree(){

    checkCurrentTree();
    _currentTreePtr->SetMakeClass(1);

    // generator weight
    if( !containsGeneratorInfo() ){
	std::string msg = "WARNING in NanoGenTreeReader.initTree:";
        msg.append(" no generator info branches found in the input tree!");
	std::cerr << msg << std::endl;
    } else{
	_currentTreePtr->SetBranchAddress("genWeight", &_genWeight, &b__genWeight);
	_currentTreePtr->SetBranchAddress("LHEScaleWeight", _LHEScaleWeight, &b__LHEScaleWeight);
	_currentTreePtr->SetBranchAddress("nLHEScaleWeight", &_nLHEScaleWeight, &b__nLHEScaleWeight);
	_currentTreePtr->SetBranchAddress("LHEPdfWeight", _LHEPdfWeight, &b__LHEPdfWeight);
        _currentTreePtr->SetBranchAddress("nLHEPdfWeight", &_nLHEPdfWeight, &b__nLHEPdfWeight);
	_currentTreePtr->SetBranchAddress("PSWeight", _PSWeight, &b__PSWeight);
        _currentTreePtr->SetBranchAddress("nPSWeight", &_nPSWeight, &b__nPSWeight);
    }
    // GenDressedLeptons
    if( !containsGenDressedLeptons() ){
	std::string msg = "WARNING in NanoGenTreeReader.initTree:";
        msg.append(" no GenDressedLepton branches found in the input tree!");
        std::cerr << msg << std::endl;
    } else{
	_currentTreePtr->SetBranchAddress("nGenDressedLepton", &_nGenDressedLepton, &b__nGenDressedLepton);
	_currentTreePtr->SetBranchAddress("GenDressedLepton_pt", _GenDressedLepton_pt, &b__GenDressedLepton_pt);
	_currentTreePtr->SetBranchAddress("GenDressedLepton_mass", _GenDressedLepton_mass, &b__GenDressedLepton_mass);
	_currentTreePtr->SetBranchAddress("GenDressedLepton_eta", _GenDressedLepton_eta, &b__GenDressedLepton_eta);
	_currentTreePtr->SetBranchAddress("GenDressedLepton_phi", _GenDressedLepton_phi, &b__GenDressedLepton_phi);
	_currentTreePtr->SetBranchAddress("GenDressedLepton_pdgId", _GenDressedLepton_pdgId, &b__GenDressedLepton_pdgId);
    }
    // GenJets
    if( !containsGenJets() ){
	std::string msg = "WARNING in NanoGenTreeReader.initTree:";
        msg.append(" no GenJet branches found in the input tree!");
        std::cerr << msg << std::endl;
    } else{
        _currentTreePtr->SetBranchAddress("nGenJet", &_nGenJet, &b__nGenJet);
        _currentTreePtr->SetBranchAddress("GenJet_pt", _GenJet_pt, &b__GenJet_pt);
	_currentTreePtr->SetBranchAddress("GenJet_mass", _GenJet_mass, &b__GenJet_mass);
        _currentTreePtr->SetBranchAddress("GenJet_eta", _GenJet_eta, &b__GenJet_eta);
        _currentTreePtr->SetBranchAddress("GenJet_phi", _GenJet_phi, &b__GenJet_phi);
        _currentTreePtr->SetBranchAddress("GenJet_hadronFlavour", _GenJet_hadronFlavour, &b__GenJet_hadronFlavour);
    }
    // GenMET
    if( !containsGenMET() ){
        std::string msg = "WARNING in NanoGenTreeReader.initTree:";
        msg.append(" no GenMET branches found in the input tree!");
        std::cerr << msg << std::endl;
    } else{
        _currentTreePtr->SetBranchAddress("GenMET_pt", &_GenMET_pt, &b__GenMET_pt);
        _currentTreePtr->SetBranchAddress("GenMET_phi", &_GenMET_phi, &b__GenMET_phi);
    }
}


void NanoGenTreeReader::setOutputTree( TTree* outputTree ){
    // generator weight
    if( !containsGeneratorInfo() ){
            std::string msg = "WARNING in NanoGenTreeReader.setOutputTree:";
            msg.append(" requested to include generator info in output tree,");
            msg.append(" but there appear to be no generator info branches in the input tree;");
            msg.append(" will skip writing generator info to output tree!");
            std::cerr << msg << std::endl;
    }
    else{
            outputTree->Branch("genWeight", &_genWeight, "_genWeight/F");
    }
    // GenDressedLeptons
    if( !containsGenDressedLeptons() ){
            std::string msg = "WARNING in NanoGenTreeReader.setOutputTree:";
            msg.append(" requested to include GenDressedLeptons in output tree,");
            msg.append(" but there appear to be no GenDressedLepton branches in the input tree;");
            msg.append(" will skip writing GenDressedLeptons to output tree!");
            std::cerr << msg << std::endl;
    }
    else{
	    outputTree->Branch("nGenDressedLepton", &_nGenDressedLepton, "_nGenDressedLepton/i");
	    outputTree->Branch("GenDressedLepton_pt", &_GenDressedLepton_pt, "_GenDressedLepton_pt[_nGenDressedLepton]/F");
	    outputTree->Branch("GenDressedLepton_mass", &_GenDressedLepton_mass, "_GenDressedLepton_mass[_nGenDressedLepton]/F");
	    outputTree->Branch("GenDressedLepton_eta", &_GenDressedLepton_eta, "_GenDressedLepton_eta[_nGenDressedLepton]/F");
	    outputTree->Branch("GenDressedLepton_phi", &_GenDressedLepton_phi, "_GenDressedLepton_phi[_nGenDressedLepton]/F");
	    outputTree->Branch("GenDressedLepton_pdgId", &_GenDressedLepton_pdgId, "_GenDressedLepton_pdgId[_nGenDressedLepton]/I");
    }
    // GenJets
    if( !containsGenJets() ){
            std::string msg = "WARNING in NanoGenTreeReader.setOutputTree:";
            msg.append(" requested to include GenJets in output tree,");
            msg.append(" but there appear to be no GenJet branches in the input tree;");
            msg.append(" will skip writing GenJets to output tree!");
            std::cerr << msg << std::endl;
    }
    else{
            outputTree->Branch("nGenJet", &_nGenJet, "_nGenJet/i");
            outputTree->Branch("GenJet_pt", &_GenJet_pt, "_GenJet_pt[_nGenJet]/F");
	    outputTree->Branch("GenJet_mass", &_GenJet_mass, "_GenJet_mass[_nGenJet]/F");
            outputTree->Branch("GenJet_eta", &_GenJet_eta, "_GenJet_eta[_nGenJet]/F");
            outputTree->Branch("GenJet_phi", &_GenJet_phi, "_GenJet_phi[_nGenJet]/F");
            outputTree->Branch("GenJet_hadronFlavour", &_GenJet_hadronFlavour, "_GenJet_hadronFlavour[_nGenJet]/b");
    }
}


// get object from current file 
TObject* NanoGenTreeReader::getFromCurrentFile( const std::string& name ) const{
    checkCurrentFile();
    return _currentFilePtr->Get( name.c_str() );
}


// get list of histograms stored in current file
std::vector< std::shared_ptr< TH1 > > NanoGenTreeReader::getHistogramsFromCurrentFile() const{
    checkCurrentFile();
    // vector containing all histograms in current file
    std::vector< std::shared_ptr< TH1 > > histogramVector;
    // loop over keys in top directory
    TDirectory* dir = (TDirectory*) _currentFilePtr->Get("");
    TList* keyList = dir->GetListOfKeys();
    for( const auto objectPtr : *keyList ){
	//try if a dynamic_cast to a histogram works to check if object is histogram
	TH1* histPtr = dynamic_cast< TH1* >( dir->Get( objectPtr->GetName() ) );
	if( histPtr ){
            histPtr->SetDirectory( gROOT );
	    histogramVector.emplace_back( histPtr );
	}
    }
    return histogramVector;
}
