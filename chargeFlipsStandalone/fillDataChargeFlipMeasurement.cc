/*
Perform a charge flip measurement in data
*/

// include c++ library classes
#include <memory>
#include <string>
#include <fstream>

// include ROOT classes
#include "TH1D.h"
#include "TStyle.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/HistInfo.h"
#include "../Tools/interface/histogramTools.h"
#include "../Tools/interface/ConstantFit.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/mt2.h"
#include "../Tools/interface/stringTools.h"
#include "../Event/interface/Event.h"
#include "../Event/interface/EventTags.h"
#include "../constants/particleMasses.h"
#include "../constants/luminosities.h"

// include dedicated tools
#include "../chargeFlips/interface/chargeFlipTools.h"


// declare variables
static const unsigned nL_max = 20;
static const unsigned gen_nL_max = 20;
ULong64_t         _runNb;
ULong64_t         _lumiBlock;
ULong64_t         _eventNb;
Double_t        _weight;
UInt_t          _nL;
UInt_t          _nMu;
UInt_t          _nEle;
UInt_t          _nLight;
UInt_t          _nTau;
Double_t        _lPt[nL_max];
Double_t        _lPtCorr[nL_max];
Double_t        _lEta[nL_max];
Double_t        _lEtaSC[nL_max];
Double_t        _lPhi[nL_max];
Double_t        _lE[nL_max];
Double_t        _lECorr[nL_max];
UInt_t          _lFlavor[nL_max];
Int_t           _lCharge[nL_max];
Bool_t          _lIsPrompt[nL_max];
Int_t           _lMatchPdgId[nL_max];
Int_t           _lMatchCharge[nL_max];
Int_t           _lMomPdgId[nL_max];
Bool_t          isEE;
Bool_t          isEMu;
Bool_t          isMuMu;
Bool_t          isOS;
Int_t           l1;
Float_t         l1_pt;
Int_t           l2;
Float_t         l2_pt;
Float_t         mll;
Int_t           nLepSel;
Float_t		genWeight;


// declare branches
TBranch         *b__runNb;
TBranch         *b__lumiBlock;
TBranch         *b__eventNb;
TBranch        *b__weight;
TBranch        *b__nL;
TBranch        *b__nMu;
TBranch        *b__nEle;
TBranch        *b__nLight;
TBranch        *b__nTau;
TBranch        *b__lPt;
TBranch        *b__lPtCorr;
TBranch        *b__lEta;
TBranch        *b__lEtaSC;
TBranch        *b__lPhi;
TBranch        *b__lE;
TBranch        *b__lECorr;
TBranch        *b__lFlavor;
TBranch        *b__lCharge;
TBranch        *b__lIsPrompt;
TBranch        *b__lMatchPdgId;
TBranch        *b__lMatchCharge;
TBranch        *b__lMomPdgId;
TBranch         *b_isEE;
TBranch         *b_isEMu;
TBranch         *b_isMuMu;
TBranch         *b_isOS;
TBranch         *b_l1;
TBranch         *b_l1_pt;
TBranch         *b_l2;
TBranch         *b_l2_pt;
TBranch         *b_mll;
TBranch         *b_nLepSel;
TBranch		*b_genWeight;


// set branch addresses
void setBranchAddresses( TreeReader treeReader ){
    treeReader._currentTreePtr->SetBranchAddress("_runNb", &_runNb, &b__runNb);
    treeReader._currentTreePtr->SetBranchAddress("_lumiBlock", &_lumiBlock, &b__lumiBlock);
    treeReader._currentTreePtr->SetBranchAddress("_eventNb", &_eventNb, &b__eventNb);
    treeReader._currentTreePtr->SetBranchAddress("_nL", &_nL, &b__nL);
    treeReader._currentTreePtr->SetBranchAddress("_nMu", &_nMu, &b__nMu);
    treeReader._currentTreePtr->SetBranchAddress("_nEle", &_nEle, &b__nEle);
    treeReader._currentTreePtr->SetBranchAddress("_nLight", &_nLight, &b__nLight);
    treeReader._currentTreePtr->SetBranchAddress("_nTau", &_nTau, &b__nTau);
    treeReader._currentTreePtr->SetBranchAddress("_lPt", _lPt, &b__lPt);
    treeReader._currentTreePtr->SetBranchAddress("_lPtCorr", _lPtCorr, &b__lPtCorr);
    treeReader._currentTreePtr->SetBranchAddress("_lEta", _lEta, &b__lEta);
    treeReader._currentTreePtr->SetBranchAddress("_lEtaSC", _lEtaSC, &b__lEtaSC);
    treeReader._currentTreePtr->SetBranchAddress("_lPhi", _lPhi, &b__lPhi);
    treeReader._currentTreePtr->SetBranchAddress("_lE", _lE, &b__lE);
    treeReader._currentTreePtr->SetBranchAddress("_lECorr", _lECorr, &b__lECorr);
    treeReader._currentTreePtr->SetBranchAddress("_lFlavor", _lFlavor, &b__lFlavor);
    treeReader._currentTreePtr->SetBranchAddress("_lCharge", _lCharge, &b__lCharge);
    if( treeReader.isMC() ){
	treeReader._currentTreePtr->SetBranchAddress("_weight", &_weight, &b__weight);
	treeReader._currentTreePtr->SetBranchAddress("genWeight", &genWeight, &b_genWeight);
	treeReader._currentTreePtr->SetBranchAddress("_lIsPrompt", _lIsPrompt, &b__lIsPrompt);
	treeReader._currentTreePtr->SetBranchAddress("_lMatchPdgId", _lMatchPdgId, &b__lMatchPdgId);
	treeReader._currentTreePtr->SetBranchAddress("_lMatchCharge", _lMatchCharge, &b__lMatchCharge);
	treeReader._currentTreePtr->SetBranchAddress("_lMomPdgId",  _lMomPdgId, &b__lMomPdgId);
    }
    treeReader._currentTreePtr->SetBranchAddress("isEE",  &isEE, &b_isEE);
    treeReader._currentTreePtr->SetBranchAddress("isEMu",  &isEMu, &b_isEMu);
    treeReader._currentTreePtr->SetBranchAddress("isMuMu",  &isMuMu, &b_isMuMu);
    treeReader._currentTreePtr->SetBranchAddress("isOS",  &isOS, &b_isOS);
    treeReader._currentTreePtr->SetBranchAddress("l1",  &l1, &b_l1);
    treeReader._currentTreePtr->SetBranchAddress("l1_pt",  &l1_pt, &b_l1_pt);
    treeReader._currentTreePtr->SetBranchAddress("l2",  &l2, &b_l2);
    treeReader._currentTreePtr->SetBranchAddress("l2_pt",  &l2_pt, &b_l2_pt);
    treeReader._currentTreePtr->SetBranchAddress("mll",  &mll, &b_mll);
    treeReader._currentTreePtr->SetBranchAddress("nLepSel",  &nLepSel, &b_nLepSel);
}


// help function to get luminosity
double getLumi( const std::string& year ){
    if(year=="2016PreVFP") return lumi::lumi2016PreVFP;
    if(year=="2016PostVFP") return lumi::lumi2016PostVFP;
    if(year=="2017") return lumi::lumi2017;
    if(year=="2018") return lumi::lumi2018;
    throw std::invalid_argument("ERROR in getLumi: invalid year: "+year);
}


// help function to initialize histograms
std::vector< HistInfo > makeDistributionInfo(){
    std::vector< HistInfo > histInfoVec = {
        HistInfo( "mll", "M_{ll} (GeV)", 40, 80, 105 ),
        HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 40, 25, 125 ),
        HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 30, 15, 95 ),

        HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 30, 0, 2.5 ),
        HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 30, 0, 2.5 ),

        //HistInfo( "met", "E_{T}^{miss} (GeV)", 40, 0, 100 ),
        //HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 40, 0, 320 ),
        //HistInfo( "ht", "H_{T} (GeV)", 40, 0, 600 ),
        //HistInfo( "mt2l", "M_{T}^{2l} (GeV)", 40, 0, 300 ),

        //HistInfo( "ptll", "P_{T}^{ll} (GeV)", 40, 0, 300 ),
        //HistInfo( "mt2ll", "M_{T2}^{ll} (GeV)", 40, 0, 120 ),

        //HistInfo( "nJets", "number of jets", 8, 0, 8 ),
        //HistInfo( "nBJets", "number of b-jets (tight deep CSV)", 4, 0, 4 ),
        //HistInfo( "nVertex", "number of vertices", 40, 0, 70 )
    };
    return histInfoVec;
}


// help function to calculate event variables
std::vector< double > computeVariables(){
    return {
        mll,
        l1_pt,
        l2_pt,
        fabs(_lEta[l1]),
        fabs(_lEta[l2])
        //event.metPt(),
        //( event.electron( 0 ).pt() + event.electron( 1 ).pt() + event.metPt() ),
        //event.jetCollection().scalarPtSum(),
        //mt( electronSum, event.met() ),
        //electronSum.pt(),
        //mt2::mt2( event.electron( 0 ), event.electron( 1 ), event.met() ),
        //static_cast< double >( event.numberOfJets() ),
        //static_cast< double >( event.numberOfTightBTaggedJets() ),
        //static_cast< double >( event.numberOfVertices() )
    };
}


bool eventIsNew(  
    const long unsigned runNumber, 
    const long unsigned luminosityBlock, 
    const long unsigned eventNumber, std::set< EventTags >& usedEventTags ){

    // search set for the current event 
    auto tagIt = usedEventTags.find( EventTags(runNumber, luminosityBlock, eventNumber) );
    
    // return false if event with the same tags was seen before
    if( tagIt != usedEventTags.end() ){
        return false;
    }

    // add unseen events to the set 
    usedEventTags.emplace( EventTags(runNumber, luminosityBlock, eventNumber) ); 
    return true;
}


void deriveChargeFlipCorrections( 
		  const std::string& year,
		  const std::string& sampleListFile,
		  const std::string& sampleDirectory,
		  const std::string& chargeFlipPath,
		  const long nEntries ){

    // settings for output file(s)
    std::string outputBaseName = "closurePlots_data_" + year;

    // read MC charge-flip maps
    std::shared_ptr< TH2D > chargeFlipMap_MC;
    chargeFlipMap_MC = chargeFlips::readChargeFlipMap( chargeFlipPath, year, "electron" );

    // initialize histograms for each contribution
    std::vector< std::string > contributions = { 
	"Data", // SS data
	"ChargeFlips", // OS data with CF weight applied
	"NonpromptBkg", // SS nonprompt simulation with no CF
	"PromptBkg", // SS prompt simulation with no CF
	"NonpromptCF", // SS nonprompt simulation with CF
	"PromptCF" // SS prompt simulation with CF
    };
    std::map< std::string, std::vector< std::shared_ptr< TH1D > > > histogramMap;
    std::vector< HistInfo > histInfoVector = makeDistributionInfo();
    for( const auto& process : contributions ){
	for( const auto& dist : histInfoVector ){
	    std::string histName = dist.name() + "_" + process + "_" + year;
	    histogramMap[ process ].push_back( dist.makeHist( histName ) );
	}
    }
    
    // make TreeReader and loop over samples
    std::set< EventTags > usedEventTags;
    TreeReader treeReader( sampleListFile, sampleDirectory );
    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample(false, false);
        setBranchAddresses(treeReader);
	double lumi = getLumi( treeReader.getYearString() );
	std::cout << "current sample: " << treeReader.currentSamplePtr()->fileName() << std::endl;

	// loop over entries
        long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEntries>0 && (unsigned)nEntries<numberOfEntries ){
            numberOfEntries = (unsigned) nEntries;
        }
	std::cout << "starting loop over " << numberOfEntries << " events." << std::endl;
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){

	    // build event
	    treeReader._currentTreePtr->GetEntry(entry);

	    // remove overlap events in data
	    if( treeReader.isData() ){
		if( !eventIsNew( _runNb, _lumiBlock, _eventNb, usedEventTags ) ) continue;
	    }

	    // apply event selections
	    // event must be ee
	    if( !isEE ) continue;
	    // basic pT and trigger cuts
	    if( l1_pt < 25. ) continue;
	    if( l2_pt < 15. ) continue;
	    // note: trigger already passed in skim? or need to apply manually?
	    //if( !( event.passTriggers_e() || event.passTriggers_ee() ) ) continue;
	    // invariant mass must be on-Z
	    if( fabs(mll-particle::mZ)>15 ) continue;
    
	    // find if leptons are same sign
	    double weight = 1.; 
	    bool isSameSign = !(isOS);
	    std::string contributionName;

	    // determine correct category
	    // same sign data
	    if( treeReader.isData() && isSameSign ){
		contributionName = "Data";
		weight = 1.;
	    // opposite sign data (-> multiply by charge flip weight)
	    } else if( treeReader.isData() ){
		contributionName = "ChargeFlips";
		weight = chargeFlips::chargeFlipWeight(
                    l1_pt, fabs(_lEta[l1]), _lFlavor[l1],
                    l2_pt, fabs(_lEta[l2]), _lFlavor[l2],
                    chargeFlipMap_MC, 0);
	    // same sign MC
	    } else if( isSameSign ){
		weight = genWeight*lumi;
		bool isPrompt = true;
		bool isChargeFlip = false;
		if( ! (_lIsPrompt[l1] && _lIsPrompt[l2] ) ){ isPrompt = false; }
		if( ! (_lCharge[l1]==_lMatchCharge[l1] 
		    && _lCharge[l2]==_lMatchCharge[l2]) ){ isChargeFlip = true; }
		if( isPrompt && isChargeFlip ){ contributionName = "PromptCF"; } 
		else if( isPrompt ){ contributionName = "PromptBkg"; }
		else if( isChargeFlip ){ contributionName = "NonpromptCF"; }
		else{ contributionName = "NonpromptBkg"; }

	    // opposite sign MC events (-> skip)
	    } else {
		continue;
	    }

	    // fill histograms
	    auto fillVariables = computeVariables();
	    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
		histogram::fillValue( histogramMap[ contributionName ][ dist ].get(), 
		      fillVariables[ dist ], weight );
	    }
        }
    }

    // set negative bins to zero
    for( const auto& process : contributions ){
        for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
            analysisTools::setNegativeBinsToZero( histogramMap[ process ][ dist ].get() );
        }
    }

    /*// determine the scale factor by fitting Mll
    std::shared_ptr< TH1D > chargeFlipPrediction( dynamic_cast< TH1D* >( 
    histogramMap.at( "Charge-flips" )[ 0 ]->Clone() ) );
    std::shared_ptr< TH1D > chargeFlipObserved( dynamic_cast< TH1D* >( 
    histogramMap.at( "Data" )[ 0 ]->Clone() ) );

    // subtract backgrounds from data
    chargeFlipObserved->Add( histogramMap.at( "Prompt" )[ 0 ].get(), -1. );
    chargeFlipObserved->Add( histogramMap.at( "Nonprompt" )[ 0 ].get(), -1. );

    // avoid negative bins in data after the subtraction
    analysisTools::setNegativeBinsToZero( chargeFlipObserved.get() );

    // fit the ratio of observed to predicted to get the scale-factor
    chargeFlipObserved->Divide( chargeFlipPrediction.get() );
    ConstantFit chargeFlipRatioFit( chargeFlipObserved );
    
    // write scale factor to txt file
    std::ofstream scaleFactorDump( outputBaseName + ".txt" );
    scaleFactorDump << "Charge flip scale factor for " << year;
    scaleFactorDump << " = " << chargeFlipRatioFit.value();
    scaleFactorDump << " +- " << chargeFlipRatioFit.uncertainty() << "\n";
    scaleFactorDump.close();*/

    // write file
    std::string fileName = outputBaseName + ".root";
    TFile* outputFilePtr = TFile::Open( fileName.c_str(), "RECREATE" );
    outputFilePtr->cd();
    for( const auto& process : contributions ){
        for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
            std::shared_ptr<TH1D> hist = histogramMap[ process ][ dist ];
	    std::string histName = histInfoVector[dist].name() + "_" + year + "_" + process; 
	    hist->SetName( histName.c_str() );
	    hist->Write();
	}
    }
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check command line arguments
    unsigned long int nargs = 6;
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( argvStr.size() != nargs+1 ){
        std::cerr << "ERROR: found " << argc-1 << " command line args,";
        std::cerr << " while " << nargs << " are needed:" << std::endl;
        std::cerr << "  - flavour (only 'electron' supported for now)" << std::endl;
        std::cerr << "  - year" << std::endl;
        std::cerr << "  - sample list" << std::endl;
        std::cerr << "  - sample directory" << std::endl;
	std::cerr << "  - path to charge flip map" << std::endl;
        std::cerr << "  - number of entries" << std::endl;
        return 1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2];
    std::string sampleList = argvStr[3];
    std::string sampleDirectory = argvStr[4];
    std::string chargeFlipPath = argvStr[5];
    long nEntries = std::stol(argvStr[6]);
    deriveChargeFlipCorrections(
        year, sampleList, sampleDirectory, chargeFlipPath, nEntries);
    std::cerr << "###done###" << std::endl;
    return 0;
}
