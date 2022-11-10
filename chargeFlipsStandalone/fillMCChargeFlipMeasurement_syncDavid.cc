/*
Perform a charge flip measurement in MC
*/

// include c++ library classes 
#include <memory>
#include <thread>

// include ROOT classes
#include "TH2D.h"
#include "TStyle.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/histogramTools.h"
#include "../Tools/interface/analysisTools.h"
#include "../plotting/interface/plotCode.h"
#include "../plotting/interface/tdrStyle.h"
#include "../constants/particleMasses.h"


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
Float_t		genWeight;
Bool_t		isEE;
Bool_t		isEMu;
Bool_t		isMuMu;
Bool_t		isOS;
Int_t		l1;
Float_t		l1_pt;
Int_t		l2;
Float_t		l2_pt;
Float_t		mll;
Int_t		nLepSel;

// declare branches
TBranch		*b__runNb;
TBranch		*b__lumiBlock;
TBranch		*b__eventNb;
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
TBranch		*b_genWeight;
TBranch		*b_isEE;
TBranch		*b_isEMu;
TBranch		*b_isMuMu;
TBranch		*b_isOS;
TBranch		*b_l1;
TBranch		*b_l1_pt;
TBranch		*b_l2;
TBranch		*b_l2_pt;
TBranch		*b_mll;
TBranch		*b_nLepSel;

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
    treeReader._currentTreePtr->SetBranchAddress("_weight", &_weight, &b__weight);
    treeReader._currentTreePtr->SetBranchAddress("_lIsPrompt", _lIsPrompt, &b__lIsPrompt);
    treeReader._currentTreePtr->SetBranchAddress("_lMatchPdgId", _lMatchPdgId, &b__lMatchPdgId);
    treeReader._currentTreePtr->SetBranchAddress("_lMatchCharge", _lMatchCharge, &b__lMatchCharge);
    treeReader._currentTreePtr->SetBranchAddress("_lMomPdgId",  _lMomPdgId, &b__lMomPdgId);
    treeReader._currentTreePtr->SetBranchAddress("genWeight", &genWeight, &b_genWeight);
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


void determineMCChargeFlipRate( const std::string& year,
				const std::string& flavour,
				const std::string& process,
				const std::string& binning,
				const std::string& sampleListFile, 
				const std::string& sampleDirectory,
				const std::string& outputDirectory,
				const long nEntries ){

    // simple check on provided year identifier
    analysisTools::checkYearString( year );

    // simple check on provided flavour identifier
    if( flavour!="electron" && flavour!="muon" ){
	throw std::invalid_argument("ERROR: flavour '"+flavour+"' not recognized.");
    }

    // simple check on provided process identifier
    bool doAllProcesses = false;
    if( process=="all" ){ doAllProcesses = true; }

    // simple check on provided binning identifier
    if( binning!="Gianny" && binning!="TuThong" && binning !="David"
	&& binning!="test" ){
	throw std::invalid_argument("ERROR: binning '"+binning+"' not recognized.");
    }

    // initialize bins (syncing with Gianny, see AN-21-182)
    std::vector< double > ptBins = {10., 30., 45., 60., 100., 200.};
    std::vector< double > etaBins = {0., 0.8, 1.1, 1.4, 1.6, 1.9, 2.5};
    // initialize bins (for syncing with Tu Thong, see AN-19-127)
    if( binning=="TuThong" ){
	ptBins = {10., 30., 45., 65., 100., 200.};
	etaBins = {0., 0.4, 0.8, 1.1, 1.4, 1.6, 1.9, 2.2, 2.5};
    }
    else if( binning=="David" ){
	ptBins = {10., 40., 75., 100., 150., 200.};
        etaBins = {0., 0.4, 0.8, 1.1, 1.4442, 1.566, 1.9, 2.2, 2.5};
    }
    else if( binning=="test" ){
	ptBins = {10., 200.};
        etaBins = {0., 2.5};
    }

    // initialize 2D histogram for numerator
    std::string numerator_name = "chargeFlipRate_numerator_" + flavour + "_" + year;
    std::shared_ptr< TH2D > numeratorMap( 
	new TH2D( numerator_name.c_str(), (numerator_name+"; p_{T} (GeV); |#eta|").c_str(), 
	ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    numeratorMap->Sumw2();

    // initialize 2D histogram for denominator
    std::string denominator_name = "chargeFlipRate_denominator_" + flavour + "_" + year;
    std::shared_ptr< TH2D > denominatorMap( 
	new TH2D( denominator_name.c_str(), (denominator_name+"; p_{T} (GeV); |#eta|").c_str(), 
	ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    denominatorMap->Sumw2();

    // initialize 2D histogram for ratio
    std::string ratio_name = "chargeFlipRate_" + flavour + "_" + year;
    std::shared_ptr< TH2D > ratioMap(
        new TH2D( ratio_name.c_str(), (ratio_name+"; p_{T} (GeV); |#eta|").c_str(),
        ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    ratioMap->Sumw2();

    // make TreeReader and loop over samples
    std::cout << "initializing TreeReader..." << std::endl;
    TreeReader treeReader( sampleListFile, sampleDirectory );
    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
	// initSample arguments: doInitTree, doInitHCounter
        treeReader.initSample( false, false );

	// check if this sample needs to be considered
	std::cout << "current sample: " << treeReader.currentSamplePtr()->fileName() << std::endl;
	std::string currentProcess = treeReader.currentSamplePtr()->processName();
	if( !doAllProcesses && currentProcess!=process ){
	    std::string msg = "  -> skipping this sample since it belongs to the ";
	    msg.append( currentProcess+" process, and only "+process+" was requested.");
	    std::cout << msg << std::endl;
	    continue;
	}

	// set branch addresses
	setBranchAddresses(treeReader);
    
	// loop over entries
	long unsigned numberOfPassingLeptons = 0;
        long unsigned numberOfChargeFlips = 0;
	long unsigned numberOfEntries = treeReader.numberOfEntries();
	if( nEntries>0 && (unsigned)nEntries<numberOfEntries ){ 
	    numberOfEntries = (unsigned) nEntries; 
	}
	std::cout << "starting loop over " << numberOfEntries << " events." << std::endl;
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
	    treeReader._currentTreePtr->GetEntry(entry);

	    // printouts for testing
	    /*std::cout << "event ID: " << _runNb << "  " << _lumiBlock << "  " << _eventNb << "  " << std::endl;
	    std::cout << "  leptons: " << l1 << " " << l2 << std::endl;
	    std::cout << "  lepton pt: " << l1_pt << " " << l2_pt << std::endl;
	    std::cout << "  number of leptons: " << _nL << std::endl;
	    std::cout << "  lFlavor: " << _lFlavor[l1] << " " << _lFlavor[l2] << std::endl;
	    std::cout << "  lPt: " << _lPt[l1] << " " << _lPt[l2] << std::endl;
	    std::cout << "  lPtCorr: " << _lPtCorr[l1] << " " << _lPtCorr[l2] << std::endl;
	    std::cout << "  weight: " << _weight << std::endl;
	    std::cout << "  genWeight: " << genWeight << std::endl;*/

	    // do event selection
	    // both leptons are required to be prompt
	    if( !(_lIsPrompt[l1] && _lIsPrompt[l2]) ) continue;
            // invariant mass must be on-Z (only for DY)
	    // (turn this cut OFF for syncing with David on 08/11/2022)
            //if( currentProcess == "DY" && fabs(mll-particle::mZ)>15 ) continue;
	    // veto events with conversions 
	    // (only for syncing with David on 08/11/2022)
	    if( _lMatchPdgId[l1]==22 || _lMatchPdgId[l2]==22 ) continue;

	    // check lepton indices
	    if( (_lFlavor[l1]==0 && fabs(l1_pt-_lPtCorr[l1])>1e-3) 
		|| (_lFlavor[l1]==1 && fabs(l1_pt-_lPt[l1])>1e-3)){
		std::string msg = "ERROR: something wrong with first lepton index...";
		msg.append( " l1_pt: "+std::to_string(l1_pt)+"," );
		msg.append( " l1(pt): "+std::to_string(_lPt[l1])+"," );
		msg.append( " l1(ptcorr): "+std::to_string(_lPtCorr[l1])+"." );
		throw std::runtime_error(msg);
	    }

	    // check lepton indices
            if( (_lFlavor[l2]==0 && fabs(l2_pt-_lPtCorr[l2])>1e-3)
                || (_lFlavor[l2]==1 && fabs(l2_pt-_lPt[l2])>1e-3) ){
                std::string msg = "ERROR: something wrong with lepton indices...";
                msg.append( " l2_pt: "+std::to_string(l2_pt)+"," );
		msg.append( " l2(pt): "+std::to_string(_lPt[l2])+"," );
		msg.append( " l2(ptcorr): "+std::to_string(_lPtCorr[l2])+"." );
                throw std::runtime_error(msg);
            }

	    // check first lepton
	    bool considerL1 = true;
	    if( flavour=="muon" && _lFlavor[l1]!=1 ) considerL1 = false;
	    if( flavour=="electron" && _lFlavor[l1]!=0 ) considerL1 = false;
	    bool l1IsChargeFlip = (_lCharge[l1]!=_lMatchCharge[l1]);
	    if( _lMatchPdgId[l1]==22 ) l1IsChargeFlip = false;

	    // check second lepton
	    bool considerL2 = true;
            if( flavour=="muon" && _lFlavor[l2]!=1 ) considerL2 = false;
	    if( flavour=="electron" && _lFlavor[l2]!=0 ) considerL2 = false;
            bool l2IsChargeFlip = (_lCharge[l2]!=_lMatchCharge[l2]);
	    if( _lMatchPdgId[l2]==22 ) l2IsChargeFlip = false;

	    // additional selection on flavor
            if( !(considerL1 && considerL2) ) continue;

	    // fill histograms
	    numberOfPassingLeptons += 2;
	    histogram::fillValues( denominatorMap.get(), l1_pt, fabs(_lEta[l1]), 1. );
	    histogram::fillValues( denominatorMap.get(), l2_pt, fabs(_lEta[l2]), 1. );
	    if( l1IsChargeFlip ){
		numberOfChargeFlips++;
		histogram::fillValues( numeratorMap.get(), l1_pt, fabs(_lEta[l1]), 1. );
                histogram::fillValues( ratioMap.get(), l1_pt, fabs(_lEta[l1]), 1. );
	    }
            if( l2IsChargeFlip ){
		numberOfChargeFlips++;
                histogram::fillValues( numeratorMap.get(), l2_pt, fabs(_lEta[l2]), 1. );
                histogram::fillValues( ratioMap.get(), l2_pt, fabs(_lEta[l2]), 1. );
            }
	}
	std::cout << "number of leptons passing selections: " << numberOfPassingLeptons << std::endl;
	std::cout << "of which charge flips: " << numberOfChargeFlips << std::endl;
    }

    // divide numerator by denominator to get charge flip rate
    ratioMap->Divide( denominatorMap.get() );

    // create output directory if it does not exist 
    systemTools::makeDirectory( outputDirectory );
    std::string outputBaseName = "chargeFlipMap_MC_" + flavour + "_" + year;
    outputBaseName += "_process_" + process + "_binning_" + binning;
    
    // plot fake-rate map
    // write numbers in exponential notation because charge flip rates tend to be very small
    gStyle->SetPaintTextFormat( "4.2e" );
    std::string plotOutputPath =  stringTools::formatDirectoryName( outputDirectory );
    plotOutputPath += outputBaseName+".pdf";
    plot2DHistogram( ratioMap.get(), plotOutputPath );

    // write fake-rate map to file 
    std::string rootOutputPath = stringTools::formatDirectoryName( outputDirectory ); 
    rootOutputPath += outputBaseName+".root";
    TFile* outputFile = TFile::Open( rootOutputPath.c_str(), "RECREATE" );
    ratioMap->Write();
    numeratorMap->Write();
    denominatorMap->Write();
    outputFile->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    long unsigned int nargs = 8;
    if( argvStr.size() != nargs+1 ){
        std::cerr << "ERROR: found " << argc-1 << " command line args,";
	std::cerr << " while " << nargs << " are needed:" << std::endl;
        std::cerr << "  - flavour" << std::endl;
	std::cerr << "  - year" << std::endl;
	std::cerr << "  - process" << std::endl;
	std::cerr << "  - binning" << std::endl;
	std::cerr << "  - sample list" << std::endl;
	std::cerr << "  - sample directory" << std::endl;
	std::cerr << "  - output directory" << std::endl;
	std::cerr << "  - number of entries" << std::endl;
        return 1;
    }
    std::string flavour = argvStr[1];
    std::string year = argvStr[2];
    std::string process = argvStr[3];
    std::string binning = argvStr[4];
    std::string sampleList = argvStr[5];
    std::string sampleDirectory = argvStr[6];
    std::string outputDirectory = argvStr[7];
    long nEntries = std::stol(argvStr[8]);
    setTDRStyle();
    determineMCChargeFlipRate(
	year, flavour, process, binning, sampleList, sampleDirectory, outputDirectory, nEntries);
    std::cerr << "###done###" << std::endl;
    return 0;
}
