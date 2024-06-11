#ifndef Event_H
#define Event_H

//include c++ library classes 
#include <utility>

//include other parts of framework
#include "LeptonCollection.h"
#include "JetCollection.h"
#include "GeneratorInfo.h"
#include "EFTInfo.h"
#include "TriggerInfo.h"
#include "JetInfo.h"
#include "EventTags.h"
#include "SusyMassInfo.h"
#include "LeptonParticleLevelCollection.h"
#include "JetParticleLevelCollection.h"
#include "../../objects/interface/Met.h"
#include "../../objects/interface/MetParticleLevel.h"
#include "../../objects/interface/PhysicsObject.h"


class TreeReader;
class NanoGenTreeReader;
class MuonCollection;
class ElectronCollection;
class TauCollection;
class Met;
class TriggerInfo;
class JetInfo;
class GeneratorInfo;
class EFTInfo;
class Sample;


class Event{

    public:
	// constructor from TreeReader
        Event( const TreeReader&, 
		const bool readIndividualTriggers = false, 
		const bool readIndividualMetFilters = false,
		const bool readAllJECVariations = false,
		const bool readGroupedJECVariations = false,
                const bool readParticleLevel = false,
                const bool readEFTCoefficients = false );
	// constructor from NanoGenTreeReader
	Event( const NanoGenTreeReader& );
	// other constructors
        Event( const Event& );
        Event( Event&& ) noexcept;

        Event& operator=( const Event& );
        Event& operator=( Event&& ) noexcept; 

        ~Event();
        

        LeptonCollection& leptonCollection() const{ return *_leptonCollectionPtr; }
        JetCollection& jetCollection() const{ return *_jetCollectionPtr; }
        Met& met() const{ return *_metPtr; }
        TriggerInfo& triggerInfo() const{ return *_triggerInfoPtr; }
	JetInfo& jetInfo() const{ return *_jetInfoPtr; }
        EventTags& eventTags() const{ return *_eventTagsPtr; }
        GeneratorInfo& generatorInfo() const;
        EFTInfo& eftInfo() const;
        SusyMassInfo& susyMassInfo() const;
	LeptonParticleLevelCollection& leptonParticleLevelCollection() const;
	JetParticleLevelCollection& jetParticleLevelCollection() const;
	MetParticleLevel& metParticleLevel() const;

	// return jet collection and met with varied JEC/JER/Uncl uncertainties
	JetCollection getJetCollection( const std::string& variation ) const{ 
	    return (*_jetCollectionPtr).getVariedJetCollection( variation); }
	Met getMet( const std::string& variation, JetCollection jets ) const{
	    return (*_metPtr).getVariedMet( variation, jets); }

        Lepton& lepton( const LeptonCollection::size_type leptonIndex ) const{ 
	    return (*_leptonCollectionPtr)[ leptonIndex ]; }
        Jet& jet( const JetCollection::size_type jetIndex ) const{ 
	    return (*_jetCollectionPtr)[ jetIndex ]; }


        void sortLeptonsByPt( bool useConeCorrectedPt=true ) const{ 
	    _leptonCollectionPtr->sortByPt( useConeCorrectedPt ); }
        void sortJetsByPt() const{ _jetCollectionPtr->sortByPt(); }

        unsigned numberOfVertices() const{ return _numberOfVertices; }
        double weight() const{ return _weight; }
	double genWeight() const{ return _genWeight; }
	double lumiScale() const{ return _lumiScale; }

        double HT() const{ return _jetCollectionPtr->scalarPtSum(); }
        double LT() const{ return _leptonCollectionPtr->scalarPtSum(); }
        double metPt() const{ return _metPtr->pt(); }
        
        // jet selection and cleaning
        void selectGoodJets() const{ _jetCollectionPtr->selectGoodJets(); }
        void cleanJetsFromLooseLeptons( const double coneSize = 0.4 ) const{ 
	    _jetCollectionPtr->cleanJetsFromLooseLeptons( *_leptonCollectionPtr, coneSize ); }
        void cleanJetsFromFOLeptons( const double coneSize = 0.4 ) const{ 
	    _jetCollectionPtr->cleanJetsFromFOLeptons( *_leptonCollectionPtr, coneSize ); }
        void cleanJetsFromTightLeptons( const double coneSize = 0.4 ) const{ 
	    _jetCollectionPtr->cleanJetsFromTightLeptons( *_leptonCollectionPtr, coneSize ); }

        // user specified jet selection
        void selectJets( bool (&passSelection)( const Jet& ) ){ _jetCollectionPtr->selectObjects( passSelection ); }

	// particle level jet selection and cleaning
	void selectGoodParticleLevelJets() const;
	void cleanParticleLevelJetsFromLeptons( const double coneSize = 0.4 ) const;
        void cleanParticleLevelLeptonsFromJets( const double coneSize = 0.4 ) const;

        // b-tag collections
        JetCollection looseBTagCollection() const{ return _jetCollectionPtr->looseBTagCollection(); }
        JetCollection mediumBTagCollection() const{ return _jetCollectionPtr->mediumBTagCollection(); }
        JetCollection tightBTagCollection() const{ return _jetCollectionPtr->tightBTagCollection(); }

        // lepton selection and cleaning
        void selectLooseLeptons(){ _leptonCollectionPtr->selectLooseLeptons(); }
        void selectFOLeptons(){ _leptonCollectionPtr->selectFOLeptons(); }
        void selectTightLeptons(){ _leptonCollectionPtr->selectTightLeptons(); }
       	void cleanElectronsFromLooseMuons( const double coneSize = 0.05 ){ _leptonCollectionPtr->cleanElectronsFromLooseMuons( coneSize ); }
        void cleanElectronsFromFOMuons( const double coneSize = 0.05 ){ _leptonCollectionPtr->cleanElectronsFromFOMuons( coneSize ); }
        void cleanTausFromLooseLightLeptons( const double coneSize = 0.4 ){ _leptonCollectionPtr->cleanTausFromLooseLightLeptons( coneSize ); }
        void cleanTausFromFOLightLeptons( const double coneSize = 0.4 ){ _leptonCollectionPtr->cleanTausFromFOLightLeptons( coneSize ); }

	// particle level leptons selection and cleaning
	void selectGoodParticleLevelLeptons() const;

        // separate lepton flavor collections
        MuonCollection muonCollection() const{ return _leptonCollectionPtr->muonCollection(); }
        ElectronCollection electronCollection() const{ return _leptonCollectionPtr->electronCollection(); }
        TauCollection tauCollection() const{ return _leptonCollectionPtr->tauCollection(); }
        LightLeptonCollection lightLeptonCollection() const{ return _leptonCollectionPtr->lightLeptonCollection(); }
        LeptonCollection::size_type numberOfMuons() const{ return _leptonCollectionPtr->numberOfMuons(); }
        LeptonCollection::size_type numberOfElectrons() const{ return _leptonCollectionPtr->numberOfElectrons(); }
        LeptonCollection::size_type numberOfTaus() const{ return _leptonCollectionPtr->numberOfTaus(); }
        LeptonCollection::size_type numberOfLightLeptons() const{ return _leptonCollectionPtr->numberOfLightLeptons(); }

        //remove taus from the lepton collection
        void removeTaus(){ _leptonCollectionPtr->removeTaus(); }

        //consider making these functions more efficient at some point with caching of the collections that are now generated every time
        LightLepton& lightLepton( const LightLeptonCollection::size_type leptonIndex ) const{ return lightLeptonCollection()[ leptonIndex ]; }
        Muon& muon( const MuonCollection::size_type muonIndex ) const{ return muonCollection()[ muonIndex ]; }
        Electron& electron( const ElectronCollection::size_type electronIndex ) const{ return electronCollection()[ electronIndex ]; }
        Tau& tau( const TauCollection::size_type tauIndex ) const{ return tauCollection()[ tauIndex ]; }

        //lepton collections based on selection
        LeptonCollection looseLeptonCollection() const{ return _leptonCollectionPtr->looseLeptonCollection(); }
        LeptonCollection FOLeptonCollection() const{ return _leptonCollectionPtr->FOLeptonCollection(); }
        LeptonCollection TightLeptonCollection() const{ return _leptonCollectionPtr->tightLeptonCollection(); }
        LeptonCollection::size_type numberOfLooseLeptons() const{ return _leptonCollectionPtr->numberOfLooseLeptons(); }
        LeptonCollection::size_type numberOfFOLeptons() const{ return _leptonCollectionPtr->numberOfFOLeptons(); }
        LeptonCollection::size_type numberOfTightLeptons() const{ return _leptonCollectionPtr->numberOfTightLeptons(); }

        //user specified lepton selection
        void selectLeptons( bool (&passSelection)( const Lepton& ) ){ _leptonCollectionPtr->selectObjects( passSelection ); }
        void selectLightLeptons( bool (&passSelection)( const LightLepton& ) ){ lightLeptonCollection().selectObjects( passSelection ); }
        void selectElectrons( bool (&passSelection)( const Electron& ) ){ electronCollection().selectObjects( passSelection ); }
        void selectMuons( bool (&passSelection)( const Muon& ) ){ muonCollection().selectObjects( passSelection ); }
        void selectTaus( bool (&passSelection)( const Tau& ) ){ tauCollection().selectObjects( passSelection ); }

        //apply lepton cone correction for fake-rate prediction
        void applyLeptonConeCorrection() const{ _leptonCollectionPtr->applyConeCorrection(); }

        //Trigger information
       	bool passTriggers_e() const{ return _triggerInfoPtr->passTriggers_e(); }
        bool passTriggers_m() const{ return _triggerInfoPtr->passTriggers_m(); }
        bool passTriggers_ee() const{ return _triggerInfoPtr->passTriggers_ee(); }
        bool passTriggers_em() const{ return _triggerInfoPtr->passTriggers_em(); }
        bool passTriggers_et() const{ return _triggerInfoPtr->passTriggers_et(); }
        bool passTriggers_mm() const{ return _triggerInfoPtr->passTriggers_mm(); }
        bool passTriggers_mt() const{ return _triggerInfoPtr->passTriggers_mt(); }
        bool passTriggers_eee() const{ return _triggerInfoPtr->passTriggers_eee(); }
        bool passTriggers_eem() const{ return _triggerInfoPtr->passTriggers_eem(); }
        bool passTriggers_emm() const{ return _triggerInfoPtr->passTriggers_emm(); }
        bool passTriggers_mmm() const{ return _triggerInfoPtr->passTriggers_mmm(); }
        bool passTriggers_FR() const{ return _triggerInfoPtr->passTriggers_FR(); }
        bool passTriggers_FR_iso() const{ return _triggerInfoPtr->passTriggers_FR_iso(); }
	bool passTriggers_ref() const{ return _triggerInfoPtr->passTriggers_ref(); }
        bool passMetFilters() const{ return _triggerInfoPtr->passMetFilters(); }
        bool passTrigger( const std::string& triggerName ) const{ return _triggerInfoPtr->passTrigger( triggerName ); }
        bool passMetFilter( const std::string& filterName ) const{ return _triggerInfoPtr->passMetFilter( filterName ); }

        //number of leptons 
        LeptonCollection::size_type numberOfLeptons() const{ return _leptonCollectionPtr->size(); }
        bool isSinglelepton() const{ return ( numberOfLeptons() == 1 ); }
        bool isDilepton() const{ return ( numberOfLeptons() == 2 ); }
        bool isTrilepton() const{ return ( numberOfLeptons() == 3 ); }
        bool isFourLepton() const{ return ( numberOfLeptons() == 4 ); }

        //lepton flavor and charge combinations
        bool hasOSSFLeptonPair() const{ return _leptonCollectionPtr->hasOSSFPair(); }
        bool hasOSSFLightLeptonPair() const{ return _leptonCollectionPtr->hasLightOSSFPair(); }
        bool hasOSLeptonPair() const{ return _leptonCollectionPtr->hasOSPair(); }
        bool leptonsAreSameSign() const{ return _leptonCollectionPtr->isSameSign(); }
        LeptonCollection::size_type numberOfUniqueOSSFLeptonPairs() const{ return _leptonCollectionPtr->numberOfUniqueOSSFPairs(); }
        LeptonCollection::size_type numberOfUniqueOSLeptonPairs() const{ return _leptonCollectionPtr->numberOfUniqueOSPairs(); }


        //presence of a Z boson
        double bestZBosonCandidateMass(bool allowSameSign = false);
        std::pair< LeptonCollection::size_type, LeptonCollection::size_type > bestZBosonCandidateIndices(bool allowSameSign = false);
        std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > bestZBosonCandidateIndicesAndMass(bool allowSameSign = false);
        bool hasZTollCandidate( const double oneSidedMassWindow, bool allowSameSign = false );
	unsigned int nZTollCandidates( const double oneSidedMassWindow );

        //transverse mass of lepton from W decay in 1 or 3 lepton events and the MET
        LeptonCollection::size_type WLeptonIndex();
        Lepton& WLepton(){ return lepton( WLeptonIndex() ); }
        double mtW();

        //other transverse mass options
        double mtLeptonMet( const LeptonCollection::size_type leptonIndex ) const{ return mt( lepton( leptonIndex ), met() ); }
        
        //number of jets 
        JetCollection::size_type numberOfJets() const{ return _jetCollectionPtr->size(); }
        JetCollection::size_type numberOfGoodJets() const{ return _jetCollectionPtr->numberOfGoodJets(); }

        //number of b-tagged jets
        JetCollection::size_type numberOfLooseBTaggedJets() const{ return _jetCollectionPtr->numberOfLooseBTaggedJets(); }
        JetCollection::size_type numberOfMediumBTaggedJets() const{ return _jetCollectionPtr->numberOfMediumBTaggedJets(); }
        JetCollection::size_type numberOfTightBTaggedJets() const{ return _jetCollectionPtr->numberOfTightBTaggedJets(); }
        bool hasLooseBTaggedJet() const{ return ( numberOfLooseBTaggedJets() != 0 ); }
        bool hasMediumBTaggedJet() const{ return ( numberOfMediumBTaggedJets() != 0 ); }
        bool hasTightBTaggedJet() const{ return ( numberOfTightBTaggedJets() != 0 ); }

        //total lepton system
        PhysicsObject leptonSystem() const{ return _leptonCollectionPtr->objectSum(); }

        //total jet system 
        PhysicsObject jetSystem() const{ return _leptonCollectionPtr->objectSum(); }

        //total lepton + jet system 
        PhysicsObject leptonJetSystem() const{ return ( leptonSystem() + jetSystem() ); }

        //total lepton + jet + met system
        PhysicsObject leptonJetMetSystem() const{ return ( leptonJetSystem() + met() ); }

        //check what year event corresponds to and whether it is a data or MC event
        bool isData() const{ return _samplePtr->isData(); }
        bool isMC() const{ return _samplePtr->isMC(); }
        bool is2016() const{ return _samplePtr->is2016(); }
        bool is2017() const{ return _samplePtr->is2017(); }
        bool is2018() const{ return _samplePtr->is2018(); }
        const Sample& sample() const{ return *_samplePtr; }

        //event tags
        long unsigned eventNumber() const{ return eventTags().eventNumber(); }
        long unsigned luminosityBlock() const{ return eventTags().luminosityBlock(); }
        long unsigned runNumber() const{ return eventTags().runNumber(); }

	// copy event with modified energy/momentum scales
	// experimental stage! need to check what attributes are copied or modified in-place!
	Event electronScaleUpEvent() const;
	Event electronScaleDownEvent() const;
	Event electronResUpEvent() const;
	Event electronResDownEvent() const;
	void setLeptonCollection( const LeptonCollection& );
	// (to do: check if a public function arbitrarily modifying the lepton collection 
	//         can be avoided...)

    private:
        LeptonCollection* _leptonCollectionPtr = nullptr;
        JetCollection* _jetCollectionPtr = nullptr;
        Met* _metPtr = nullptr;
        TriggerInfo* _triggerInfoPtr = nullptr;
	JetInfo* _jetInfoPtr = nullptr;
        EventTags* _eventTagsPtr = nullptr;
        GeneratorInfo* _generatorInfoPtr = nullptr;
        SusyMassInfo* _susyMassInfoPtr = nullptr;
	EFTInfo* _eftInfoPtr = nullptr;
	LeptonParticleLevelCollection* _leptonParticleLevelCollectionPtr = nullptr;
	JetParticleLevelCollection* _jetParticleLevelCollectionPtr = nullptr;
	MetParticleLevel* _metParticleLevelPtr = nullptr;
        unsigned _numberOfVertices = 0;
        double _weight = 1;
	double _genWeight = 1;
	double _lumiScale = 1;
        const Sample* _samplePtr = nullptr;

        //presence of Z boson
        bool ZIsInitialized = false;
        std::pair< LeptonCollection::size_type, LeptonCollection::size_type > _bestZBosonCandidateIndices;
        LeptonCollection::size_type _WLeptonIndex = 0;
        double _bestZBosonCandidateMass;
        void initializeZBosonCandidate(bool allowSameSign = false);
        
        //always make sure lepton collection is sorted before selecting Z candidates 
        //WARNING : make sure this is robust when cone-correction is applied!
        //bool leptonCollectionIsSorted = false;

        //check the presence of generator information
        bool hasGeneratorInfo() const{ return ( _generatorInfoPtr != nullptr ); }
        void checkGeneratorInfo() const;

        // check the presence of EFT information
        bool hasEFTInfo() const{ return ( _eftInfoPtr != nullptr ); }
        void checkEFTInfo() const;

        //check the presence of susy information
        bool hasSusyMassInfo() const{ return ( _susyMassInfoPtr != nullptr ); }
        void checkSusyMassInfo() const;

	// check the presence of particle level information
	bool hasParticleLevel() const{ 
	    return ( _leptonParticleLevelCollectionPtr != nullptr
		    && _jetParticleLevelCollectionPtr != nullptr
		    && _metParticleLevelPtr != nullptr ); }
	void checkParticleLevel() const;

	Event variedLeptonCollectionEvent(
                    LeptonCollection (LeptonCollection::*variedCollection)() const ) const;
};

#endif
