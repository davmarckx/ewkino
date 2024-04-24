/*
Implementation of a TreeReader class that works on NanoGen format
*/

// Originally meant for processing alternative ttW samples:
// while the full analysis is run on MiniAOD format,
// alternative signal models are produced by the Oviedo group
// in NanoGen format (i.e. NanoAOD with only Gen information).

#ifndef NanoGenTreeReader_H
#define NanoGenTreeReader_H

// include c++ library classes 
#include <fstream>
#include <iostream>
#include <typeinfo>
#include <map>
#include <string>

// include ROOT classes
#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TLorentzVector.h"

// include other parts of the framework
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../constants/luminosities.h"

class Event;

class NanoGenTreeReader {

    public :

        // constructor
        NanoGenTreeReader() = default;
        NanoGenTreeReader( const std::string& sampleListFile,
                    const std::string& sampleDirectory ); 

        // declare leaf types
	// constants
	// note: unsure what values to pick for nanoAOD files...
	// warning: if these maximum array sizes are too small 
	// (compared to the actual array length in the input tree),
	// some memory corruption seems to happen under the radar,
	// which mostly goes unnoticed but causes occasional segmentation violations,
	// or potentially nonsensical values.
        static const unsigned nGenDressedLepton_max = 30;
        static const unsigned nGenJet_max = 30;
	// note: variable types can be found here:
	// https://cms-nanoaod-integration.web.cern.ch/autoDoc/
        // generator weight
	Float_t	    _genWeight;
	// GenDressedLepton collection
	Float_t	    _GenDressedLepton_eta[nGenDressedLepton_max];
	Float_t     _GenDressedLepton_phi[nGenDressedLepton_max];
	Float_t     _GenDressedLepton_pt[nGenDressedLepton_max];
	Float_t     _GenDressedLepton_mass[nGenDressedLepton_max];
	Int_t	    _GenDressedLepton_pdgId[nGenDressedLepton_max];
	UInt_t	    _nGenDressedLepton;
	// GenJet collection
	Float_t     _GenJet_eta[nGenJet_max];
        Float_t     _GenJet_phi[nGenJet_max];
        Float_t     _GenJet_pt[nGenJet_max];
	Float_t     _GenJet_mass[nGenJet_max];
        UChar_t     _GenJet_hadronFlavour[nGenJet_max];
        UInt_t      _nGenJet;
	// GenMET object
	Float_t	    _GenMET_pt;
	Float_t	    _GenMET_phi;
	
        // weight including cross section and lumi scaling 
        double          _scaledWeight;
        double		_lumiScale;

        // set up tree for reading and writing
        void initTree();
        void setOutputTree( TTree* );

        // initialize the next sample
        void initSample( const bool doInitTree = true );
        void initSample( unsigned int sampleIndex,
                         const bool doInitTree = true );
        void initSample( const Sample&, 
			 const bool doInitTree = true );  

        // read sample list from text file
        void readSamples(const std::string& list, const std::string& directory);

        // initialize the current sample directly from a root file
        void initSampleFromFile( const std::string& pathToFile,
				 const bool is2016, 
				 const bool is2016PreVFP, 
				 const bool is2016PostVFP,
				 const bool is2017,
                                 const bool is2018 );
        void initSampleFromFile( const std::string& pathToFile );

        // get entry from Tree, should not be used except for test purposes
        void GetEntry(const Sample&, long unsigned);
        void GetEntry(long unsigned);

        // build event (this will implicitly use GetEntry )
        Event buildEvent( const Sample&, long unsigned ); 
        Event buildEvent( long unsigned ); 

        // check whether specific info is present in current tree
	bool containsGeneratorInfo() const;
	bool containsGenDressedLeptons() const;
	bool containsGenJets() const;
	bool containsGenMET() const;

        // check which year the current sample belongs to
        bool is2016() const;
	bool is2016PreVFP() const;
	bool is2016PostVFP() const;
        bool is2017() const;
        bool is2018() const;
	std::string getYearString() const;

        // check whether the current sample is data or MC
	// (normally all NanoGen samples are MC, but keep just in case for syntax)
        bool isData() const;
        bool isMC() const;

        // access number of samples and current sample
        const Sample& currentSample() const{ return *_currentSamplePtr; }
        const Sample* currentSamplePtr() const{ return _currentSamplePtr.get(); }
        std::vector< Sample >::size_type numberOfSamples() const{ return samples.size(); }
        std::vector< Sample > sampleVector() const{ return samples; }

        // access current file and tree 
        std::shared_ptr< TFile > currentFilePtr(){ return _currentFilePtr; }

        // get object from current file 
        TObject* getFromCurrentFile( const std::string& name ) const;

        // get list of histograms stored in current file
        std::vector< std::shared_ptr< TH1 > > getHistogramsFromCurrentFile() const;

	// get number of entries
        unsigned long numberOfEntries() const;

    private:

        // list of samples to loop over 
        std::vector< Sample > samples;

        // current sample
        std::shared_ptr< const Sample > _currentSamplePtr;

        // TFile associated to current sample
        std::shared_ptr< TFile > _currentFilePtr;

        // TTree associated to current sample 
        TTree* _currentTreePtr = nullptr;

        // check whether current sample is initialized, throw an error if it is not 
        void checkCurrentSample() const;

        // check whether current Tree is initialized, throw an error if it is not 
        void checkCurrentTree() const;

        // check whether current File is initialized, throw an error if it is not
        void checkCurrentFile() const;

        // current index in samples vector
        int currentSampleIndex = -1;

        // general function to read a list of samples
        void readSamples(const std::string&, const std::string&, std::vector<Sample>&);

        // list of branches
	// generator weight
	TBranch*    b__genWeight;
	// GenDressedLepton collection
	TBranch*    b__GenDressedLepton_eta;
        TBranch*    b__GenDressedLepton_phi;
        TBranch*    b__GenDressedLepton_pt;
	TBranch*    b__GenDressedLepton_mass;
        TBranch*    b__GenDressedLepton_pdgId;
        TBranch*    b__nGenDressedLepton;
	// GenJet collection
        TBranch*    b__GenJet_eta;
        TBranch*    b__GenJet_phi;
        TBranch*    b__GenJet_pt;
	TBranch*    b__GenJet_mass;
        TBranch*    b__GenJet_hadronFlavour;
        TBranch*    b__nGenJet;
	// GenMET object
	TBranch*    b__GenMET_pt;
	TBranch*    b__GenMET_phi;
};

#endif
