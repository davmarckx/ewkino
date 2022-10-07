#include "../interface/ConcreteReweighterFactory.h"

//include ROOT classes
#include "TFile.h"
#include "TH2.h"

//include other parts of framework
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../interface/ConcreteLeptonReweighter.h"
#include "../interface/ConcreteReweighterLeptons.h"
#include "../interface/ConcreteSelection.h"
#include "../interface/ReweighterPileup.h"
#include "../interface/ConcreteReweighterBTag.h"
#include "../interface/ReweighterPrefire.h"


// --------------------------------------
// empty reweighter for testing purposes 
// --------------------------------------
// allows for testing code with reweighting syntax, but without actual reweighting.
// all individual reweighters are set to an ReweighterEmpty instance,
// which simply returns 1 for each event.

CombinedReweighter EmptyReweighterFactory::buildReweighter(
        const std::string& weightDirectory,
        const std::string& year,
        const std::vector< Sample >& samples ) const{
 
    // reweighter to return
    CombinedReweighter combinedReweighter;
    // dummy condition on args to avoid compilation warnings
    if(weightDirectory=="" && year=="" && samples.size()==0) return combinedReweighter;

    // add all reweighters needed formally, but each reweighter returns unity for each event
    combinedReweighter.addReweighter( "muonID", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "muonIDSyst", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "muonIDStat", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronID", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronIDSyst", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronIDStat", std::make_shared< ReweighterEmpty >() );
    if( year == "2016" || year == "2017" ){
	combinedReweighter.addReweighter( "electronReco_pTBelow20",
        std::make_shared< ReweighterEmpty >() );
        combinedReweighter.addReweighter( "electronReco_pTAbove20",
        std::make_shared< ReweighterEmpty >() );
    } else if( year == "2018" ){
	combinedReweighter.addReweighter( "electronReco",
	std::make_shared< ReweighterEmpty >() );
    }
    combinedReweighter.addReweighter( "pileup", std::make_shared<ReweighterEmpty>() );
    combinedReweighter.addReweighter( "bTag_heavy", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "bTag_light", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "bTag_shape", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterEmpty >() );

    return combinedReweighter;
}

// -------------------------------
// reweighter for ewkino analysis 
// -------------------------------
// example of concrete implementation.
// do not use in new analyses.
// corresponds to recommendations from long ago (certainly before ultra-legacy run 2).

CombinedReweighter EwkinoReweighterFactory::buildReweighter( 
	const std::string& weightDirectory, 
	const std::string& year, 
	const std::vector< Sample >& samples ) const{

    analysisTools::checkYearString( year );

    //reweighter to return
    CombinedReweighter combinedReweighter;

    //make muon ID Reweighter
    TFile* muonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
			+ "weightFiles/leptonSF/looseToTight_" + year + "_m_3l.root" ).c_str() );
    std::shared_ptr< TH2 > muonSFHist( dynamic_cast< TH2* >( muonSFFile->Get( "EGamma_SF2D" ) ) );
    muonSFHist->SetDirectory( gROOT );
    muonSFFile->Close();
    MuonReweighter muonReweighter( muonSFHist, new TightSelector );
    combinedReweighter.addReweighter( "muonID", 
	std::make_shared< ReweighterMuons >( muonReweighter ) );

    //make electron ID Reweighter
    TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
			+ "weightFiles/leptonSF/looseToTight_" + year + "_e_3l.root" ).c_str() );
    std::shared_ptr< TH2 > electronSFHist( dynamic_cast< TH2* >( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist->SetDirectory( gROOT );
    eleSFFile->Close();

    ElectronIDReweighter electronIDReweighter( electronSFHist, new TightSelector );
    combinedReweighter.addReweighter( "electronID", 
	std::make_shared< ReweighterElectronsID >( electronIDReweighter ) );

    //make electron Reconstruction Reweighter
    if( year == "2016" || year == "2017" ){

        //pT below 20 GeV
        TFile* eleRecoSFFile_pTBelow20 = TFile::Open( ( stringTools::formatDirectoryName( 
	    weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" 
	    + year + "_pTBelow20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20( dynamic_cast< TH2* >( 
	    eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
        eleRecoSFFile_pTBelow20->Close();

        ElectronIDReweighter electronRecoReweighter_pTBelow20( electronRecoSFHist_pTBelow20, 
	    new LooseMaxPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTBelow20", 
	    std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTBelow20 ) );

        //pT above 20 GeV
        TFile* eleRecoSFFile_pTAbove20 = TFile::Open( ( stringTools::formatDirectoryName( 
	    weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" 
	    + year + "_pTAbove20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20( dynamic_cast< TH2* >( 
	    eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
        eleRecoSFFile_pTAbove20->Close();

        ElectronIDReweighter electronRecoReweighter_pTAbove20( electronRecoSFHist_pTAbove20, 
	    new LooseMinPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTAbove20", 
	    std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTAbove20 ) );

    } else if( year == "2018" ){

        //inclusive pT 
        TFile* eleRecoSFFile = TFile::Open( ( stringTools::formatDirectoryName( 
	    weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + ".root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist ( dynamic_cast< TH2* >(
	    eleRecoSFFile->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist->SetDirectory( gROOT );
        eleRecoSFFile->Close();

        ElectronIDReweighter electronRecoReweighter( electronRecoSFHist, new LooseSelector );
        combinedReweighter.addReweighter( "electronReco", 
	    std::make_shared< ReweighterElectronsID >( electronRecoReweighter ) );

    }
    
    //make pileup Reweighter
    combinedReweighter.addReweighter( "pileup", 
	std::make_shared< ReweighterPileup >( samples, weightDirectory ) );
    
    //make b-tagging Reweighter 
    const std::string& bTagWP = "tight";

    //read MC b-tagging efficiency histograms
    const std::string& leptonCleaning = "looseLeptonCleaned";
    TFile* bTagEffMCFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFiles/bTagEff/bTagEff_" + leptonCleaning + "_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > bTagEffMCHist_udsg( dynamic_cast< TH2* >( bTagEffMCFile->Get( 
	( "bTagEff_" + bTagWP + "_udsg" ).c_str() ) ) );
    bTagEffMCHist_udsg->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_c( dynamic_cast< TH2* >( bTagEffMCFile->Get( 
	( "bTagEff_" + bTagWP + "_charm" ).c_str() ) ) );
    bTagEffMCHist_c->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_b( dynamic_cast< TH2* >( bTagEffMCFile->Get( 
	( "bTagEff_" + bTagWP + "_beauty" ).c_str() ) ) );
    bTagEffMCHist_b->SetDirectory( gROOT );
    bTagEffMCFile->Close();

    //path of b-tagging SF 
    std::string bTagSFFileName;
    if( year == "2016" ){
        bTagSFFileName= "DeepCSV_2016LegacySF_WP_V1.csv";
    } else if( year == "2017" ){
        bTagSFFileName = "DeepCSV_94XSF_WP_V4_B_F.csv";
    } else {
        bTagSFFileName = "DeepCSV_102XSF_WP_V1.csv";
    }
    std::string bTagSFPath = "weightFiles/bTagSF/" + bTagSFFileName;

    combinedReweighter.addReweighter( "bTag_heavy", 
	std::make_shared< ReweighterBTagHeavyFlavorDeepCSV >( weightDirectory, bTagSFPath, 
	bTagWP, bTagEffMCHist_c, bTagEffMCHist_b ) );
    combinedReweighter.addReweighter( "bTag_light", 
	std::make_shared< ReweighterBTagLightFlavorDeepCSV >( weightDirectory, bTagSFPath, 
	bTagWP, bTagEffMCHist_udsg ) );

    //make prefire Reweighter
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterPrefire >() );

    return combinedReweighter;
}

// ------------------------------------------
// reweighter for run-2 ultralegacy analyses 
// ------------------------------------------
// default reweighter corresponding to recommendations for ultralegacy run-2 analyses.
// note: just a template, might need to modified or extended for specific analyses
//       (e.g. different lepton ID, special b-tagging, updated prefire scale factors, etc)
// note: for the latest recommendations, see here: 
//       https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun2LegacyAnalysis
// note: json files for pileup are coming from here:
//       https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration/-/tree/master/POG/LUM,
//       not sure where (or when) to get them 'officially'.

CombinedReweighter Run2ULReweighterFactory::buildReweighter( 
	const std::string& weightDirectory, 
	const std::string& year, 
	const std::vector< Sample >& samples ) const {

    analysisTools::checkYearString( year );

    // initiaize reweighter to return
    CombinedReweighter combinedReweighter;

    // electron reco reweighter
    // pT below 20 GeV
    TFile* eleRecoSFFile_pTBelow20 = TFile::Open( 
	( stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFilesUL/electronRecoSF/electronRECO_SF_" + year + "_ptBelow20.root" ).c_str() );
    std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20( 
	dynamic_cast< TH2* >( eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
    electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
    eleRecoSFFile_pTBelow20->Close();
    ElectronIDReweighter electronRecoReweighter_pTBelow20( electronRecoSFHist_pTBelow20, 
	new LooseMaxPtSelector< 20 > );
    combinedReweighter.addReweighter( "electronReco_pTBelow20", 
	std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTBelow20 ) );

    // electron reco reweighter
    // pT above 20 GeV
    TFile* eleRecoSFFile_pTAbove20 = TFile::Open( 
	( stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFilesUL/electronRecoSF/electronRECO_SF_" + year + "_ptAbove20.root" ).c_str() );
    std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20( 
	dynamic_cast< TH2* >( eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
    electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
    eleRecoSFFile_pTAbove20->Close();
    ElectronIDReweighter electronRecoReweighter_pTAbove20( electronRecoSFHist_pTAbove20, 
	new LooseMinPtSelector< 20 > );
    combinedReweighter.addReweighter( "electronReco_pTAbove20", 
	std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTAbove20 ) );
    
    // make muon ID reweighter
    std::string muonSFFileName = stringTools::formatDirectoryName( weightDirectory )
        + "weightFilesUL/leptonSF/leptonMVAUL_SF_muons_Medium_"+year+".root";
    TFile* muonSFFile = TFile::Open( (muonSFFileName).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one and errors relative
    // (note: the histogram _combined_syst contains the absolute uncertainties as bin contents!),
    // load the statistical errors and set the bin contents to one and the errors relative
    // (note: the histogram _stat contains the absolute uncertainties as bin errors!).
    std::shared_ptr< TH2 > muonSFHist_nom( dynamic_cast< TH2* >( 
	muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt" ) ) );
    muonSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_syst( dynamic_cast< TH2* >( 
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt_combined_syst" ) ) );
    muonSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_stat( dynamic_cast< TH2* >( 
        muonSFFile->Get( "NUM_LeptonMvaMedium_DEN_TrackerMuons_abseta_pt_stat" ) ) );
    muonSFHist_stat->SetDirectory( gROOT );
    muonSFFile->Close();
    for(int i = 0; i <= muonSFHist_nom->GetNbinsX()+1; ++i){
        for(int j = 0; j <= muonSFHist_nom->GetNbinsY()+1; ++j){
            // process values
            muonSFHist_nom->SetBinError(i,j,0.);
            muonSFHist_syst->SetBinError(i,j,
		muonSFHist_syst->GetBinContent(i,j)/muonSFHist_nom->GetBinContent(i,j));
            muonSFHist_syst->SetBinContent(i,j,1.);
            muonSFHist_stat->SetBinError(i,j,
		muonSFHist_stat->GetBinError(i,j)/muonSFHist_nom->GetBinContent(i,j));
            muonSFHist_stat->SetBinContent(i,j,1.);
        }
    }

    MuonReweighter muonReweighter_nom( muonSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter("muonID",
	std::make_shared<ReweighterMuons>(muonReweighter_nom));
    MuonReweighter muonReweighter_syst( muonSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter("muonIDSyst",
	std::make_shared<ReweighterMuons>(muonReweighter_syst));
    MuonReweighter muonReweighter_stat( muonSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter("muonIDStat",
	std::make_shared<ReweighterMuons>(muonReweighter_stat));
    muonSFFile->Close();

    // make electron ID reweighter
    std::string eleSFFileName = stringTools::formatDirectoryName( weightDirectory )
      + "weightFilesUL/leptonSF/leptonMVAUL_SF_electrons_Tight_"+year+".root";
    TFile* eleSFFile = TFile::Open( (eleSFFileName).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one and errors relative
    // (note: the histogram syst contains the absolute uncertainties as bin contents!),
    // load the statistical errors and set the bin contents to one and errors relative
    // (note: the histogram stat contains the absolute uncertainties as bin contents!).
    std::shared_ptr< TH2 > electronSFHist_nom( dynamic_cast< TH2* >
    ( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_syst( dynamic_cast< TH2* >
        ( eleSFFile->Get( "sys" ) ) );
    electronSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_stat( dynamic_cast< TH2* >
        ( eleSFFile->Get( "stat" ) ) );
    electronSFHist_stat->SetDirectory( gROOT );
    for(int i = 0; i <= electronSFHist_nom->GetNbinsX()+1; ++i){
        for(int j = 0; j <= electronSFHist_nom->GetNbinsY()+1; ++j){
            electronSFHist_nom->SetBinError(i,j,0.);
            electronSFHist_syst->SetBinError(i,j,
		electronSFHist_syst->GetBinContent(i,j)/electronSFHist_nom->GetBinContent(i,j));
            electronSFHist_syst->SetBinContent(i,j,1.);
            electronSFHist_stat->SetBinError(i,j,
		electronSFHist_stat->GetBinContent(i,j)/electronSFHist_nom->GetBinContent(i,j));
            electronSFHist_stat->SetBinContent(i,j,1.);
        }
    }

    ElectronIDReweighter electronIDReweighter_nom( electronSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter( "electronID", 
	std::make_shared<ReweighterElectronsID>(electronIDReweighter_nom) );
    ElectronIDReweighter electronIDReweighter_syst( electronSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter( "electronIDSyst", 
	std::make_shared<ReweighterElectronsID>(electronIDReweighter_syst) );
    ElectronIDReweighter electronIDReweighter_stat( electronSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter( "electronIDStat", 
	std::make_shared<ReweighterElectronsID>(electronIDReweighter_stat) );
    eleSFFile->Close();
    
    // make the b-tag shape reweighter
    // step 1: set correct csv file
    std::string bTagSFFileName = "bTagReshaping_unc_"+year+".csv";
    std::string sfFilePath = "weightFilesUL/bTagSF/"+bTagSFFileName;
    // step 2: set other parameters
    std::string flavor = "all";
    std::string bTagAlgo = "deepFlavor";
    std::vector<std::string> variations = {"jes","hf","lf","hfstats1","hfstats2",
                                        "lfstats1","lfstats2","cferr1","cferr2" };
    // step 3: make the reweighter
    /*std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
	weightDirectory, sfFilePath, flavor, bTagAlgo, variations, samples );
    reweighterBTagShape->initialize(samples, 0);
    combinedReweighter.addReweighter( "bTag_shape", reweighterBTagShape );*/

    // make pileup reweighter
    std::string pileupWeightPath = stringTools::formatDirectoryName( weightDirectory )
	    + "weightFilesUL/pileupWeights/"
	    + "Collisions" + year + "_UltraLegacy_goldenJSON.root";
    combinedReweighter.addReweighter( "pileup",
        std::make_shared< ReweighterPileup >( pileupWeightPath ) );

    // make prefire reweighter
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterPrefire >() );

    return combinedReweighter;
}
