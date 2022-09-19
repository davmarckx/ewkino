#include "../interface/Reweighter_old.h"

//include c++ library classes

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"

//include other parts of code 
#include "../interface/analysisTools.h"

Reweighter::Reweighter(const std::vector<Sample>& samples, const bool sampleIs2016, const std::string& bTagWorkingPoint) : is2016(sampleIs2016), bTagWP( bTagWorkingPoint ) {
    initializeAllWeights(samples);
}

void Reweighter::initializeAllWeights(const std::vector<Sample>& samples){

    //initialize pu weights
    initializePuWeights(samples);

    //initialize b-tag weights
    initializeBTagWeights();

    //initialize electron weights 
    initializeElectronWeights();

    //initialize muon weights 
    initializeMuonWeights();

    //initialize fake-rate
    initializeFakeRate();

    //initialize prefiring probabilities
    initializePrefiringProbabilities();
}

void Reweighter::initializePuWeights(const std::vector< Sample >& sampleList){

    static const std::string minBiasVariation[3] = {"central", "down", "up"};
    for(auto& sample : sampleList){

        //no pu weights for data 
        if( sample.isData() ) continue;

        //open root file corresponding to sample
        TFile* puFile = TFile::Open( (const TString&) "weights/pileUpWeights/puWeights_" + sample.getFileName() );

        //extract pu weights 
        for(unsigned var = 0; var < 3; ++var){
            std::string histName = "puw_Run";
            histName += (sample.is2016() ? "2016" : "2017");
            histName += "Inclusive_" + minBiasVariation[var];
            puWeights[sample.getUniqueName()].push_back( std::shared_ptr<TH1D> ( (TH1D*)puFile->Get( (const TString&) histName ) ) );

            //make sure histogram does not get deleted when closing file
            puWeights[sample.getUniqueName()].back()->SetDirectory(gROOT);
        }
        puFile->Close();
    }
}


void Reweighter::initializeBTagWeights(){

    //assuming deepCSV tagger is used!
    std::string sfFileName;
    if(is2016){
        sfFileName = "DeepCSV_Moriond17_B_H.csv";
    } else {
        sfFileName = "DeepCSV_94XSF_V3_B_F.csv";
    }
    bTagCalib = std::shared_ptr<BTagCalibration>( new BTagCalibration("deepCsv", "weights/" + sfFileName) );

    BTagEntry::OperatingPoint wp;
    if( bTagWP == "loose"){
        wp = BTagEntry::OP_LOOSE;
    } else if(bTagWP == "medium"){
        wp = BTagEntry::OP_MEDIUM;
    } else if(bTagWP == "tight"){
        wp = BTagEntry::OP_TIGHT;
    } else if(bTagWP == "reshaping"){
        wp = BTagEntry::OP_RESHAPING;
    } else{
        std::cerr << "Error in Reweighter::initializeBTagWeights : unknown b-tagging working point given. Leaving weights uninitialized" << std::endl;
        return;
    }

    bTagCalibReader = std::shared_ptr<BTagCalibrationReader>( new BTagCalibrationReader( wp, "central", {"up", "down"}) );

    std::string methodHeavy = (bTagWP == "reshaping") ? "iterativefit" : "comb";
    std::string methodLight = (bTagWP == "reshaping") ? "iterativefit" : "incl";

    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_B, methodHeavy);
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_C, methodHeavy);
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_UDSG, methodLight);    

    //initialize b-tag efficiencies
    //efficiency files are not needed when reshaping the full csv distribution
    if( bTagWP != "reshaping"){
        std::string effFileName;
        if(is2016){
            effFileName = "bTagEff_deepCSV_cleaned_tZq_2016.root";
        } else {
            effFileName = "bTagEff_deepCSV_cleaned_tZq_2017.root";
        }

        TFile* bTagFile = TFile::Open( (const TString&) "weights/" + effFileName);
        const std::string quarkFlavors[3] = {"udsg", "charm", "beauty"};
        for(unsigned flav = 0; flav < 3; ++flav){
            
            //WARNING: b-tagging efficiencies currently assume medium deepCSV tagger!
            bTagEffHist[flav] = std::shared_ptr<TH2D>( (TH2D*) bTagFile->Get( (const TString&) "bTagEff_" + bTagWP + quarkFlavors[flav]) );
            bTagEffHist[flav]->SetDirectory(gROOT);
        }
        bTagFile->Close();
    }
}

void Reweighter::initializeElectronWeights(){	

    //read electron reco SF weights
    std::string year = ( is2016 ? "2016" : "2017" );

    //low pT SF
    TFile* electronRecoFile_pT0to20 = TFile::Open( (const TString&) "weights/electronRecoSF_" + year + "_pT0to20.root");
    electronRecoSF_pT0to20 = std::shared_ptr<TH2D>( (TH2D*) electronRecoFile_pT0to20->Get("EGamma_SF2D") );
    electronRecoSF_pT0to20->SetDirectory(gROOT);
    electronRecoFile_pT0to20->Close();

    //high pT SF
    TFile* electronRecoFile_pT20toInf = TFile::Open( (const TString&) "weights/electronRecoSF_" + year + "_pT20toInf.root");
    electronRecoSF_pT20toInf = std::shared_ptr<TH2D>( (TH2D*) electronRecoFile_pT20toInf->Get("EGamma_SF2D") );
    electronRecoSF_pT20toInf->SetDirectory(gROOT);
    electronRecoFile_pT20toInf->Close();

    //read electron ID SF weights
    TFile* electronIdFile = TFile::Open( (const TString&) "weights/electronIDScaleFactors_" + year + ".root");
    electronLooseToRecoSF = std::shared_ptr<TH2D>( (TH2D*) electronIdFile->Get("EleToTTVLoose") );
    electronLooseToRecoSF->SetDirectory(gROOT);
    electronTightToLooseSF = std::shared_ptr<TH2D>( (TH2D*) electronIdFile->Get("TTVLooseToTTVLeptonMvatZq") );
    electronTightToLooseSF->SetDirectory(gROOT);
    electronIdFile->Close();
}


void Reweighter::initializeMuonWeights(){

    //Muon tracking weights are no longer applied following the recommendations of the muon POG
    /*
    //read muon reco SF weights
    if(is2016){

        //WARNING: not clear how ownership works for TGraph, can not set directory
        //make sure the TGraph is not DELETED when file is closed!
        TFile* muonRecoFile = TFile::Open("weights/muonTrackingSF_2016.root");
        muonRecoSF = std::shared_ptr<TGraph>( (TGraph*) muonRecoFile->Get("ratio_eff_eta3_dr030e030_corr") );
        muonRecoFile->Close();
    } else {
        //WARNING: 2017 muon reco SF are currently not available
    }
    */    

    //read muon ID SF weights
    std::string year = ( is2016 ? "2016" : "2017" );
    TFile* muonIdFile = TFile::Open( (const TString&) "weights/muonIDScaleFactors_" + year + ".root");
    muonLooseToRecoSF = std::shared_ptr<TH2D>( (TH2D*) muonIdFile->Get("MuonToTTVLoose") );
    muonLooseToRecoSF->SetDirectory(gROOT);
    muonTightToLooseSF = std::shared_ptr<TH2D>( (TH2D*) muonIdFile->Get("TTVLooseToTTVLeptonMvatZq") );
    muonTightToLooseSF->SetDirectory(gROOT);
    muonIdFile->Close();	
}

void Reweighter::initializeFakeRate(){

    //WARNING : Currently no fakerate maps with electroweak variations are available
    for( auto& flavor : {std::string("e"), std::string("mu")} ){
        std::string year = (  is2016 ? "2016" : "2017" );
        TFile* frFile = TFile::Open( (const TString&) "weights/fakeRate_tZq_" + flavor + "_" + year + ".root");
        if( flavor == "e"){
            frMapEle[0] = std::shared_ptr<TH2D>( (TH2D*) frFile->Get("passed") );
            frMapEle[0]->SetDirectory(gROOT);
        } else {
            frMapMu[0] = std::shared_ptr<TH2D>( (TH2D*) frFile->Get("passed") );
            frMapMu[0]->SetDirectory(gROOT);
        }
        frFile->Close();
    }
}

void Reweighter::initializePrefiringProbabilities(){
    std::string year = ( is2016 ? "2016" : "2017" );
    TFile* prefiringFile = TFile::Open( (const TString&) "weights/L1prefiring_eff_jet_" + year + ".root");
    prefiringMap = std::shared_ptr<TH2D>( (TH2D*) prefiringFile->Get( is2016? "prefireEfficiencyMap" : "L1prefiring_jet_2017BtoF" ) );
    prefiringMap->SetDirectory(gROOT);
    prefiringFile->Close();
}

Reweighter::~Reweighter(){}

double Reweighter::puWeight(const double nTrueInt, const Sample& sample, const unsigned unc) const{
    if(unc < 3){

        //find weights for given sample
        const auto& weightVectorIter = puWeights.find(sample.getUniqueName() );

        //check if pu weights are available for this sample
        if( weightVectorIter == puWeights.cend() ){
            std::cerr << "Error: no pu weights found for sample : " << sample.getUniqueName() << " returning weight 0  " << std::endl;
            return 0.;
        }

        //WARNING: 2016 samples might be used when making 2017 plots when no 2017 equivalent is available
        // In this case is2016() will return false, but the pileup weights for the sample will only exist
        // up to 50 interactions. So one needs to be sure to check for what campaign the sample was generated.
        bool sampleIsSimulatedFor2016 =  ( sample.getFileName().find("Summer16") != std::string::npos );
        double maxBin = (sampleIsSimulatedFor2016 ?  49.5 : 99.5 );

        TH1D* weights = ( (*weightVectorIter).second)[unc].get();

        //buggy events in 2017 MC can have negative pileup! catch them 
        if( nTrueInt >= 0){
            return weights->GetBinContent(weights->FindBin(std::min(nTrueInt, maxBin) ) );
        } else {
            std::cerr << "Error: event with nTrueInt = " << nTrueInt << " , returning weight 9999." << std::endl;
            return 9999.;
        }
    }
    else {
        std::cerr << "Error: invalid pu uncertainty requested: returning weight 0" << std::endl;
        return 0.;
    }
}

double Reweighter::bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc) const{
    static const BTagEntry::JetFlavor flavorEntries[3] = {BTagEntry::FLAV_UDSG, BTagEntry::FLAV_C, BTagEntry::FLAV_B};
    static const std::string uncName[3] = {"central", "down", "up"};
    if(unc < 3){
        return bTagCalibReader->eval_auto_bounds(uncName[unc], flavorEntries[flavorInd(jetFlavor)], jetEta, jetPt, jetCSV);
    } else {
        std::cerr << "Error: invalid b-tag SF uncertainty requested: returning weight 1" << std::endl;
        return 1.;
    }
}

double Reweighter::bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta) const{
    
    //map jet flavor to efficiecy index map 
    unsigned index = flavorInd(jetFlavor);

    //check whether bTagEffHist was initialized, which is not the case when Reweighter was initialized with the reshaping option
    if( bTagEffHist[index].use_count() == 0 ){
        std::cerr << "Error in Reweighter::bTagEff, b-tag efficiencies were not initialized. Perhaps you have initialized Reweighter to use b-tag reshaping? Returning 999." << std::endl;
        return 9999.;
    }

    if(jetPt > 25){
        double croppedPt = std::min( std::max(jetPt, 25.), 599.);
        double croppedAbsEta = std::min( fabs(jetEta), 2.39 ); 
        return bTagEffHist[index]->GetBinContent(bTagEffHist[index]->FindBin(croppedPt, croppedAbsEta) );
    } else {
        std::cerr << "Error: requesting b-tag efficiency for jet with pT below 25 GeV, this jet should not enter the selection! Returning efficiency 0." << std::endl;
        return 0.;
    } 
}

double Reweighter::muonRecoWeight() const{
    /*
    if( is2016 ){
        return muonRecoSF->Eval(std::max(-2.4,std::min(eta, 2.4) ) );
    } else {
        //WARNING temporary, implement this later
        return 1.;
    }
    */
    return 1.;
}

double Reweighter::electronRecoWeight(const double superClusterEta, const double pt) const{
    double croppedSuperClusterEta = std::max(-2.49, std::min(superClusterEta, 2.49) );
    if( pt <= 20 ){
        double croppedPt = std::max(10.01, pt);
        return electronRecoSF_pT0to20->GetBinContent( electronRecoSF_pT0to20->FindBin( croppedSuperClusterEta, croppedPt ) );
    } else {
        double croppedPt = std::min(pt, 499.); 
        return electronRecoSF_pT20toInf->GetBinContent( electronRecoSF_pT20toInf->FindBin( croppedSuperClusterEta, croppedPt ) );
    }
}

double Reweighter::muonLooseIdWeight(const double pt, const double eta) const{
    double croppedPt = std::min(pt, 199.);
    double croppedEta = std::min( fabs(eta), 2.39 ); 
    return muonLooseToRecoSF->GetBinContent( muonLooseToRecoSF->FindBin( croppedPt, croppedEta) );
}

double Reweighter::electronLooseIdWeight(const double pt, const double superClusterEta) const{
    double croppedPt = std::min(pt, 199.);
    double croppedEta = std::min( std::max( -2.49, superClusterEta ), 2.49 ); 
    return electronLooseToRecoSF->GetBinContent( electronLooseToRecoSF->FindBin( croppedPt, croppedEta) );
}

double Reweighter::muonTightIdWeight(const double pt, const double eta) const{
    double croppedPt = std::min(pt, 199.);
    double croppedEta = std::min( fabs(eta), 2.39);
    double sf = muonLooseToRecoSF->GetBinContent( muonLooseToRecoSF->FindBin( croppedPt, croppedEta) );
    sf*= muonTightToLooseSF->GetBinContent( muonTightToLooseSF->FindBin( croppedPt, croppedEta) );
    return sf;
}

double Reweighter::electronTightIdWeight(const double pt, const double superClusterEta) const{
    double croppedPt = std::min(pt, 199.);
    double croppedEta = std::min( std::max( -2.49, superClusterEta ), 2.49 ); 
    double sf = electronLooseToRecoSF->GetBinContent( electronLooseToRecoSF->FindBin( croppedPt, croppedEta) );
    sf*= electronTightToLooseSF->GetBinContent( electronTightToLooseSF->FindBin( croppedPt, croppedEta) );
    return sf;
}

double Reweighter::muonFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!! 
    if(unc < 3){

        //normally croppedPt should allow values up to 99, but to avoid the region where the 
        //fake-rate measurement suffers from too severe EWK contamination, we don't use the 
        //last two fake-rate bins.
        double croppedPt = std::min(pt, 44.);
        double croppedEta = std::min( fabs(eta), 2.39);

        //WARNING: temporary patch for empty spot at low muon pT, |eta| > 2.1 in 2016 fake-rate map
        if( is2016 && croppedPt <= 15){
            croppedEta = std::min( fabs(eta), 2.09);
        }

        return frMapMu[unc]->GetBinContent(frMapMu[unc]->FindBin( croppedPt, croppedEta ) );
    } else {
        std::cerr << "Error: invalid muon fake-rate uncertainty requested: returning fake-rate 99" << std::endl;
        return 99;
    }
}

double Reweighter::electronFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    if(unc < 3){ 
        
        //normally croppedPt should allow values up to 99, but to avoid the region where the 
        //fake-rate measurement suffers from too severe EWK contamination, we don't use the 
        //last two fake-rate bins.
        double croppedPt = std::min(pt, 44.); 
        double croppedEta = std::min( fabs(eta), 2.49);
        return frMapEle[unc]->GetBinContent(frMapEle[unc]->FindBin( croppedPt, croppedEta ) );
    } else {
        std::cerr << "Error: invalid electron fake-rate uncertainty requested: returning fake-rate 99" << std::endl;
        return 99;
    }
}

double Reweighter::jetPrefiringProbability(const double pt, const double eta) const{

    //consider probabilities outside of map to be zero
    //this is implicitly implemented by not requiring the pt and eta values to fall in the map

    //abseta binning for 2016, eta binning for 2017
    double croppedEta = eta;
    if( is2016 ){
        croppedEta = fabs(eta);
    }
    return prefiringMap->GetBinContent( prefiringMap->FindBin(croppedEta, pt ) );
}
