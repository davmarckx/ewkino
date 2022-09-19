//include ROOT classes
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"

//include C++ library classes
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <tuple>
#include <stdlib.h>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/systemTools.h"
#include "../interface/ewkinoTools.h"
#include "../interface/trilepTools.h"
#include "../interface/HistCollectionSample.h"
#include "../interface/HistCollectionDist.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "../interface/Reweighter_old.h"


void treeReader::setup(){

    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);

    //read samples and cross sections from txt file
    readSamples2016("sampleLists/samples_dilepCR_2016.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino");
    readSamples2017("sampleLists/samples_dilepCR_2017.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino");

    //info on kinematic distributions to plot
    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("sip3d", "SIP_{3D}", 100, 0, 8),
        HistInfo("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        HistInfo("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        HistInfo("miniIso", "miniIso", 100, 0, 0.4),
        HistInfo("relIso", "relIso_{#DeltaR 0.3}", 100, 0, 1),
        HistInfo("leptonMvaSUSY16", "SUSY lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvaTTH16", "TTH lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvatZqTTV16", "tZq/TTV lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvaSUSY17", "SUSY lepton MVA 2017", 100, -1, 1),
        HistInfo("leptonMvaTTH17", "TTH lepton MVA 2017", 100, -1, 1),
        HistInfo("leptonMvatZqTTV17", "tZq/TTV lepton MVA 2017", 100, -1, 1),
        HistInfo("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        HistInfo("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        HistInfo("closestJetCsvV2", "closest jet CSVv2", 100, 0, 1),
        HistInfo("closestJetDeepCsv", "closest jet DeepCSV", 100, 0, 1),
        HistInfo("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        HistInfo("electronMvaGP", "electron GP Spring 16 MVA value", 100, -1, 1),
        HistInfo("electronMvaHZZ", "electron HZZ Spring 16 MVA value", 100, -1, 1),
        HistInfo("electronMvaFall17NoIso", "electron Fall 17 no iso MVA value", 100, -1, 1),
        HistInfo("electronMvaFall17Iso", "electron HZZ Fall 17 iso MVA value", 100, -1, 1),
        HistInfo("muonSegComp", "muon segment compatibility", 100, 0, 1),

        HistInfo("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        HistInfo("mll", "M_{ll} (GeV)", 200, 12, 200),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        HistInfo("leadinEta", "|#eta|^{leading}", 100, 0, 2.5),
        HistInfo("trailingEta", "|#eta|^{trailing}", 100, 0, 2.5),
        HistInfo("nVertex", "number of vertices", 100, 0, 100),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        HistInfo("jetPt_higheEtaJet", "p_{T} (most forward jet)", 100, 25, 500),
        HistInfo("jetEta_highestEtaJet_pTCut20", "|#eta|(most forward jet) (P_{T} > 20 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut20", "#eta (most forward jet) (P_{T} > 20 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut20", "number of |eta| > 2.4, P_{T} > 20 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut25", "|#eta|(most forward jet) (P_{T} > 25 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut25", "#eta(most forward jet) (P_{T} > 25 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut25", "number of |eta| > 2.4, P_{T} > 25 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut30", "|#eta|(most forward jet) (P_{T} > 30 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut30", "#eta(most forward jet) (P_{T} > 30 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut30", "number of |eta| > 2.4, P_{T} > 30 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut35", "|#eta|(most forward jet) (P_{T} > 35 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut35", "#eta(most forward jet) (P_{T} > 35 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut35", "number of |eta| > 2.4, P_{T} > 35 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut40", "|#eta|(most forward jet) (P_{T} > 40 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut40", "#eta(most forward jet) (P_{T} > 40 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut40", "number of |eta| > 2.4, P_{T} > 40 GeV", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut45", "|#eta|(most forward jet) (P_{T} > 45 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut45", "#eta(most forward jet) (P_{T} > 45 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut45", "number of |eta| > 2.4, P_{T} > 45 GeV", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut50", "|#eta|(most forward jet) (P_{T} > 50 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut50", "#eta(most forward jet) (P_{T} > 50 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut50", "number of |eta| > 2.4, P_{T} > 50 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut55", "|#eta|(most forward jet) (P_{T} > 55 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut55", "#eta(most forward jet) (P_{T} > 55 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut55", "number of |eta| > 2.4, P_{T} > 55 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut60", "|#eta|(most forward jet) (P_{T} > 60 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut60", "#eta (most forward jet) (P_{T} > 60 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut60", "number of |eta| > 2.4, P_{T} > 60 GeV jets", 6, 0, 6),

        HistInfo("jetEta", "|#eta|(jet), P_{T} > 25 GeV", 100, 0, 5),
        //HisInfo("jetsEta", "|#eta|(jet), P_{T} > 40 GeV", 100, 0, 5),

        HistInfo("jetPhi", "#phi(jet), P_{T} > 25 GeV", 100, 0, 3.15),
        //HisInfo("jetsPhi", "#phi(jet), P_{T} > 40 GeV", 100, 0, 3.15),

        HistInfo("jetPt", "P_{T}(jet)", 100, 0, 600),

        HistInfo("jetPt_eta0to2p4", "P_{T}(jet), |#eta| < 2.4", 100, 0, 600),
        HistInfo("jetPhi_eta0to2p4", "#phi(jet), |#eta| < 2.4", 100, 0, 3.15),
        HistInfo("jetNeutralHadronFraction_eta0to2p4", "Neutral hadron fraction, |#eta| < 2.4", 100, 0, 1),
        HistInfo("jetChargedHadronFraction_eta0to2p4", "charged hadron fraction, |#eta| < 2.4", 100, 0, 1),
        HistInfo("jetNeutralEmFractionFraction_eta0to2p4", "neutral em fraction, |#eta| < 2.4", 100, 0, 1),
        HistInfo("jetChargedEmFractionFraction_eta0to2p4", "charged em fraction, |#eta| < 2.4", 100, 0, 1),
        HistInfo("jetPtUncorrected_eta0to2p4", "P_{T}^{uncorrected}(jet), |#eta| < 2.4", 100, 0, 600),
        HistInfo("jetPtL1_eta0to2p4", "P_{T}^{L1}(jet), |#eta| < 2.4", 100, 0, 600),
        HistInfo("jetPtL2_eta0to2p4", "P_{T}^{L2}(jet), |#eta| < 2.4", 100, 0, 600),
        HistInfo("jetPtL3_eta0to2p4", "P_{T}^{L3}(jet), |#eta| < 2.4", 100, 0, 600),

        HistInfo("jetPt_eta2p4to2p7", "P_{T}(jet), 2.4 < |#eta| < 2.7", 100, 0, 600),
        HistInfo("jetPhi_eta2p4to2p7", "#phi(jet), 2.4 < |#eta| < 2.7", 100, 0, 3.15),
        HistInfo("jetNeutralHadronFraction_eta2p4to2p7", "Neutral hadron fraction, 2.4 < |#eta| < 2.7", 100, 0, 1),
        HistInfo("jetChargedHadronFraction_eta2p4to2p7", "charged hadron fraction, 2.4 < |#eta| < 2.7", 100, 0, 1),
        HistInfo("jetNeutralEmFractionFraction_eta2p4to2p7", "neutral em fraction, 2.4 < |#eta| < 2.7", 100, 0, 1),
        HistInfo("jetChargedEmFractionFraction_eta2p4to2p7", "charged em fraction, 2.4 < |#eta| < 2.7", 100, 0, 1),
        HistInfo("jetPtUncorrected_eta2p4to2p7", "P_{T}^{uncorrected}(jet), 2.4 < |#eta| < 2.7", 100, 0, 600),
        HistInfo("jetPtL1_eta2p4to2p7", "P_{T}^{L1}(jet), 2.4 < |#eta| < 2.7", 100, 0, 600),
        HistInfo("jetPtL2_eta2p4to2p7", "P_{T}^{L2}(jet), 2.4 < |#eta| < 2.7", 100, 0, 600),
        HistInfo("jetPtL3_eta2p4to2p7", "P_{T}^{L3}(jet), 2.4 < |#eta| < 2.7", 100, 0, 600),

        HistInfo("jetPt_eta2p7to3", "P_{T}(jet), 2.7 < |#eta| < 3", 100, 0, 600),
        HistInfo("jetPhi_eta2p5to3", "#phi(jet), 2.7 < |#eta| < 3", 100, 0, 3.15),
        HistInfo("jetNeutralHadronFraction_eta2p7to3", "Neutral hadron fraction, 2.7 < |#eta| < 3", 100, 0, 1),
        HistInfo("jetChargedHadronFraction_eta2p7to3", "charged hadron fraction, 2.7 < |#eta| < 3", 100, 0, 1),
        HistInfo("jetNeutralEmFractionFraction_eta2p7to3", "neutral em fraction, 2.7 < |#eta| < 3", 100, 0, 1),
        HistInfo("jetChargedEmFractionFraction_eta2p7to3", "charged em fraction, 2.7 < |#eta| < 3", 100, 0, 1),
        HistInfo("jetPtUncorrected_eta2p7to3", "P_{T}^{uncorrected}(jet), 2.7 < |#eta| < 3", 100, 0, 600),
        HistInfo("jetPtL1_eta2p7to3", "P_{T}^{L1}(jet), 2.7 < |#eta| < 3", 100, 0, 600),
        HistInfo("jetPtL2_eta2p7to3", "P_{T}^{L2}(jet), 2.7 < |#eta| < 3", 100, 0, 600),
        HistInfo("jetPtL3_eta2p7to3", "P_{T}^{L3}(jet), 2.7 < |#eta| < 3", 100, 0, 600),

        HistInfo("jetPt_eta3to5", "P_{T}(jet), 3 < |#eta| < 5", 100, 0, 600),
        HistInfo("jetPhi_eta3to5", "#phi(jet), 3 < |#eta| < 5", 100, 0, 3.15),
        HistInfo("jetNeutralHadronFraction_eta3to5", "Neutral hadron fraction, 3 < |#eta| < 5", 100, 0, 1),
        HistInfo("jetChargedHadronFraction_eta3to5", "charged hadron fraction, 3 < |#eta| < 5", 100, 0, 1),
        HistInfo("jetNeutralEmFractionFraction_eta3to5", "neutral em fraction, 3 < |#eta| < 5", 100, 0, 1),
        HistInfo("jetChargedEmFractionFraction_eta3to5", "charged em fraction, 3 < |#eta| < 5", 100, 0, 1),
        HistInfo("jetPtUncorrected_eta3to5", "P_{T}^{uncorrected}(jet), 3 < |#eta| < 5", 100, 0, 600),
        HistInfo("jetPtL1_eta3to5", "P_{T}^{L1}(jet), 3 < |#eta| < 5", 100, 0, 600),
        HistInfo("jetPtL2_eta3to5", "P_{T}^{L2}(jet), 3 < |#eta| < 5", 100, 0, 600),
        HistInfo("jetPtL3_eta3to5", "P_{T}^{L3}(jet), 3 < |#eta| < 5", 100, 0, 600)

    };
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    //auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getFileName() == sampName; } );
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getUniqueName() == sampName; } );
    if(samIt == samples.cend()){
        std::cerr << "Error : Given sample name not found in list of samples!" << std::endl;
        return;
    }
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){
    //initialize sample 
    initSample(samp);

    //run categorization is different for 2016 and 2017 data
    std::vector<std::string> runCategorization;
    if(is2016()){
        std::cout << "is2016 flagged true!" << std::endl;
        runCategorization = {"all2016", "RunB", "RunC", "RunD", "RunE", "RunF", "RunG", "RunH"};
    } else {
        std::cout << "is2016 flagged false!" << std::endl;
        runCategorization = {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"};
    }

    //set up histogram collection for particular sample
    HistCollectionSample histCollection(histInfo, samp, { runCategorization, {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });

    //read pu weights for every period
    TFile* puFile;
    std::vector<std::string> eras;
    if( is2016() ){
        puFile = TFile::Open("weights/pileUpWeights/puWeights_DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_Summer16.root");
        eras = {"Inclusive", "B", "C", "D", "E", "F", "G", "H"}; 
    } else {
        puFile = TFile::Open("weights/pileUpWeights/puWeights_DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8_Fall17.root");
        eras = {"Inclusive", "B", "C", "D", "E", "F"};
    }
    std::vector<TH1D*> puWeights;
    for( auto& era : eras){
        puWeights.push_back( (TH1D*) puFile->Get( (const TString&) "puw_Run" + ( is2016() ? "2016" : "2017") + era + "_central") );
    }
    float maxPuBin;
    if( currentSample.getFileName().find("Summer16") != std::string::npos ){
        maxPuBin = 49.5;
    } else {
        maxPuBin = 99.5;
    }

    //read SF weights
    std::shared_ptr< Reweighter > reweighter(new Reweighter(samples, is2016() ) );
    
    const unsigned nDist = histInfo.size();
    const unsigned nRuns = histCollection.categoryRange(0);
    const unsigned nJetCat = histCollection.categoryRange(2);
    const unsigned nPuRew = histCollection.categoryRange(3);

    for(long unsigned it = begin; it < end; ++it){
        GetEntry(samp, it);

        //vector containing good lepton indices
        std::vector<unsigned> ind;

        //require met filters and triggers
        if( !passMETFilters() ) continue;
        if( !(passSingleLeptonTriggers() || passDileptonTriggers() ) ) continue;

        //select leptons
        const unsigned lCount = selectLep(ind);
        if(lCount != 2) continue;

        //temporary for comparison of shapes
        /*
        if( isMC() ){
            if( !promptLeptons() ){
                continue;
            }
        }
        */

        //require pt cuts (25, 20) to be passed
        if(!passPtCuts(ind)) continue;

        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

        //Cut of Mll at 12 GeV
        if((lepV[0] + lepV[1]).M() < 12) continue;

        //determine flavor compositions
        unsigned flav = dilFlavorComb(ind) + 1; //reserve 0 for inclusive

        //determine whether there is an OS lepton pair 
        bool hasOS = ( _lCharge[ind[0]] != _lCharge[ind[1]] );

        //order jets and check the number of jets
        std::vector<unsigned> jetInd;
        unsigned jetCount = nJets(jetInd);

        //Extra category selection: for DY select onZ, for ttbar select 1 b-jet 2-jets
        if((flav == 1 || flav == 3) && hasOS){ //OSSF
            if( fabs((lepV[0] + lepV[1]).M() - 91) > 10 ) continue;
            if(nBJets(0,  true, true, 0) != 0) continue;    //veto cleaned jets, maybe this fixes csv distribution of close jets
        } else if(flav == 2 || !hasOS){
            if(_met < 50) continue;
            if(jetCount < 2 || nBJets() < 1) continue;
            if(!hasOS && (jetCount < 3 || jetCount > 4)) continue;
        }

        //new flavor categorization if the leptons are SS
        if(!hasOS){
            flav += 3;
        }

        //determine run perios
        unsigned run;
        if( is2016() ){
            run = ewk::runPeriod2016(_runNb) + 1 - 1; //reserve 0 for inclusive // -1 because we start at run B 
        } else{
            run = ewk::runPeriod2017(_runNb) + 1 - 1; //reserve 0 for inclusive // -1 because we start at run B 
        }

        //loop over leading leptons
        for(unsigned l = 0; l < 2; ++l){
            double fill[21] = {_3dIPSig[ind[l]], fabs(_dxy[ind[l]]), fabs(_dz[ind[l]]), _miniIso[ind[l]], _relIso[ind[l]],
                _leptonMvaSUSY16[ind[l]], 
                _leptonMvaTTH16[ind[l]], 
                _leptonMvatZqTTV16[ind[l]],
                _leptonMvaSUSY17[ind[l]], 
                _leptonMvaTTH17[ind[l]], 
                _leptonMvatZqTTV17[ind[l]],
                _ptRel[ind[l]], 
                _ptRatio[ind[l]], 
                _closestJetCsvV2[ind[l]], 
                closestJetDeepCSV( ind[l] ),
                (double) _selectedTrackMult[ind[l]], 
                (_lFlavor[ind[l]] == 0) ? _lElectronMva[ind[l]] : 0,  
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaHZZ[ind[l]] : 0,
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaFall17NoIso[ind[l]] : 0,
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaFall17Iso[ind[l]] : 0,
                (_lFlavor[ind[l]] == 1) ? _lMuonSegComp[ind[l]] : 0
            };

            //fill histograms
            for(unsigned j = 0; j < nJetCat; ++j){
                if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 0; dist < 21; ++dist){
                        if(_lFlavor[ind[l]] == 0 && dist == 20) continue;  //do not plot muonSegComp for electrons
                        if(_lFlavor[ind[l]] == 1 && (dist == 16 || dist == 17 || dist == 18 || dist == 19) ) continue;  //do not plot electronMva for muons
                        for(unsigned r = 0; r < nRuns; ++r){

                            //apply loose ID scale factors 
                            double leptonW = 1.;
                            if( !isData() && r == 0) {
                                for(unsigned l = 0; l < lCount; ++l){
                                    if( isElectron(ind[l]) ) leptonW *= reweighter->electronLooseWeight(_lPt[ind[l]], _lEtaSC[ind[l]]); 
                                    else if (isMuon(ind[l]) ) leptonW *= reweighter->muonLooseWeight(_lPt[ind[l]], _lEta[ind[l]]);
                                    else {
                                        std::cerr << "Error: selected muon seems to be neither electron nor muon!" << std::endl;
                                    }
                                }
                            }

                            if(!samp.isData() || r == run || r == 0){
                                double puw = 1.;
                                
                                //TO DO: currently only reweighting for 2017 data, add 2016 reweighting
                                if( !isData() && pu == 1){
                                    puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, maxPuBin) ) );
                                }
                                histCollection.access(dist, {r, flav, j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw*leptonW); 
                                histCollection.access(dist, {r, 0,    j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw*leptonW);

                            }
                        }
                    }
                }
            }
        }

        //compute number of forward jets and find the highest eta jet
        std::vector<double> forwardJetPtCuts = {20, 25, 30, 35, 40, 45, 50, 55, 60};
        std::vector<unsigned> mostForwardJetIndices(9, 99); 
        std::vector<unsigned> highPtJetCount(9, 0);
        std::vector<unsigned> nForwardJets(9, 0);
        for(unsigned p = 0; p < forwardJetPtCuts.size(); ++p){
            for(unsigned j = 0; j < _nJets; ++j){
                if(jetIsGood(j, forwardJetPtCuts[p], 0, true, true) ){     //last "true" allows jets to have any eta value
                    if( (highPtJetCount[p] == 0) || ( fabs(_jetEta[j]) > fabs(_jetEta[mostForwardJetIndices[p]]) ) ){
                        mostForwardJetIndices[p] = j;
                    }
                    if(fabs(_jetEta[j]) >= 2.4){
                        ++nForwardJets[p];
                    }
                    ++highPtJetCount[p];
                }
            }
        }

        double fill[nDist - 21- 45] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], fabs(_lEta[ind[0]]), fabs(_lEta[ind[1]]), (double) _nVertex, (double) jetCount, (double) nBJets(0, false), (double) nBJets(),
           (highPtJetCount[1] > 0) ? _jetPt[mostForwardJetIndices[1]] : -9999., //pT of most forward jet after 25 GeV pT cut selection`
           (highPtJetCount[0] > 0) ? fabs(_jetEta[mostForwardJetIndices[0]]) : -9999., (highPtJetCount[0] > 0) ? _jetEta[mostForwardJetIndices[0]] : -9999., (double) nForwardJets[0], 
           (highPtJetCount[1] > 0) ? fabs(_jetEta[mostForwardJetIndices[1]]) : -9999., (highPtJetCount[1] > 0) ? _jetEta[mostForwardJetIndices[1]] : -9999., (double) nForwardJets[1], 
           (highPtJetCount[2] > 0) ? fabs(_jetEta[mostForwardJetIndices[2]]) : -9999., (highPtJetCount[2] > 0) ? _jetEta[mostForwardJetIndices[2]] : -9999., (double) nForwardJets[2], 
           (highPtJetCount[3] > 0) ? fabs(_jetEta[mostForwardJetIndices[3]]) : -9999., (highPtJetCount[3] > 0) ? _jetEta[mostForwardJetIndices[3]] : -9999., (double) nForwardJets[3], 
           (highPtJetCount[4] > 0) ? fabs(_jetEta[mostForwardJetIndices[4]]) : -9999., (highPtJetCount[4] > 0) ? _jetEta[mostForwardJetIndices[4]] : -9999., (double) nForwardJets[4], 
           (highPtJetCount[5] > 0) ? fabs(_jetEta[mostForwardJetIndices[5]]) : -9999., (highPtJetCount[5] > 0) ? _jetEta[mostForwardJetIndices[5]] : -9999., (double) nForwardJets[5], 
           (highPtJetCount[6] > 0) ? fabs(_jetEta[mostForwardJetIndices[6]]) : -9999., (highPtJetCount[6] > 0) ? _jetEta[mostForwardJetIndices[6]] : -9999., (double) nForwardJets[6], 
           (highPtJetCount[7] > 0) ? fabs(_jetEta[mostForwardJetIndices[7]]) : -9999., (highPtJetCount[7] > 0) ? _jetEta[mostForwardJetIndices[7]] : -9999., (double) nForwardJets[7], 
           (highPtJetCount[8] > 0) ? fabs(_jetEta[mostForwardJetIndices[8]]) : -9999., (highPtJetCount[8] > 0) ? _jetEta[mostForwardJetIndices[8]] : -9999., (double) nForwardJets[8]
        }; 

        for(unsigned j = 0; j < nJetCat; ++j){
            if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
            for(unsigned pu = 0; pu < nPuRew; ++pu){
                for(unsigned dist = 21; dist < nDist - 45; ++dist){     //-45 since there are 45 jet variables
                    for(unsigned r = 0; r < nRuns; ++r){
                        //apply loose ID scale factors 
                        double leptonW = 1.;
                        if( !isData() && r == 0) {
                            for(unsigned l = 0; l < lCount; ++l){
                                if( isElectron(ind[l]) ) leptonW *= reweighter->electronLooseWeight(_lPt[ind[l]], _lEtaSC[ind[l]]);
                                else if (isMuon(ind[l]) ) leptonW *= reweighter->muonLooseWeight(_lPt[ind[l]], _lEta[ind[l]]);
                                else {
                                    std::cerr << "Error: selected muon seems to be neither electron nor muon!" << std::endl;
                                }
                            }
                        }

                        if(!samp.isData() || r == run || r == 0){
                            double puw = 1.;

                            //TO DO: currently only reweighting for 2017 data, add 2016 reweighting
                            if( !isData() && pu == 1){
                                puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, maxPuBin) ) );
                            }
                            histCollection.access(dist, {r, flav, j, pu})->Fill(std::min(fill[dist - 21], histInfo[dist].maxBinCenter()), weight*puw*leptonW);
                            histCollection.access(dist, {r, 0,    j, pu})->Fill(std::min(fill[dist - 21], histInfo[dist].maxBinCenter()), weight*puw*leptonW);
                        }
                    }
                }
            } 
        }


        //loop over all jets to fill jet distributions
        for(unsigned j = 0; j < _nJets; ++j){
            if( !( jetIsGood(j, 25, 0, true, true) ) ) continue;

            double fillJets[13] = {fabs(_jetEta[j]), _jetPhi[j], _jetPt[j], _jetPt[j], _jetPhi[j], _jetNeutralHadronFraction[j], _jetChargedHadronFraction[j],
                _jetNeutralEmFraction[j], _jetChargedEmFraction[j], _jetPt_Uncorrected[j], _jetPt_L1[j], _jetPt_L2[j], _jetPt_L3[j] };

            for(unsigned jetCat = 0; jetCat < nJetCat; ++jetCat){
                if(jetCat == 1 && ( (jetCount == 0) ? false : _jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 0; dist < 13; ++dist){
                        for(unsigned r = 0; r < nRuns; ++r){

                            //apply loose ID scale factors 
                            double leptonW = 1.;
                            if( !isData() && r == 0) {
                                for(unsigned l = 0; l < lCount; ++l){
                                    if( isElectron(ind[l]) ) leptonW *= reweighter->electronLooseWeight(_lPt[ind[l]],  _lEtaSC[ind[l]]);
                                    else if (isMuon(ind[l]) ) leptonW *= reweighter->muonLooseWeight(_lPt[ind[l]], _lEta[ind[l]]);
                                    else {
                                        std::cerr << "Error: selected muon seems to be neither electron nor muon!" << std::endl;
                                    }
                                }
                            } 

                            if(!samp.isData() || r == run || r == 0){
                                double puw = 1.;

                                //TO DO: currently only reweighting for 2017 data, add 2016 reweighting
                                if( !isData() && pu == 1 ){
                                    puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, maxPuBin) ) );
                                }
                                
                                if(dist < 3){
                                    //std::cout <<  histInfo[dist + 21 + 38].name() << std::endl;
                                    histCollection.access(dist + 21 + 38, {r, flav, jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38].maxBinCenter()), weight*puw*leptonW);
                                    histCollection.access(dist + 21 + 38, {r, 0,    jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38].maxBinCenter()), weight*puw*leptonW);
                                } else {
                                    if( fabs(_jetEta[j]) <= 2.4){
                                        histCollection.access(dist + 21 + 38, {r, flav, jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38].maxBinCenter()), weight*puw*leptonW);
                                        histCollection.access(dist + 21 + 38, {r, 0,    jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38].maxBinCenter()), weight*puw*leptonW);
                                    } else if( fabs(_jetEta[j]) <= 2.7){
                                        histCollection.access(dist + 21 + 38 + 10, {r, flav, jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 10].maxBinCenter()), weight*puw*leptonW);
                                        histCollection.access(dist + 21 + 38 + 10, {r, 0,    jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 10].maxBinCenter()), weight*puw*leptonW);
                                    } else if( fabs(_jetEta[j]) <= 3.0){
                                        histCollection.access(dist + 21 + 38 + 20, {r, flav, jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 20].maxBinCenter()), weight*puw*leptonW);
                                        histCollection.access(dist + 21 + 38 + 20, {r, 0,    jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 20].maxBinCenter()), weight*puw*leptonW);
                                    } else{
                                        histCollection.access(dist + 21 + 38 + 30, {r, flav, jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 30].maxBinCenter()), weight*puw*leptonW);
                                        histCollection.access(dist + 21 + 38 + 30, {r, 0,    jetCat, pu})->Fill(std::min(fillJets[dist], histInfo[dist + 21 + 38 + 30].maxBinCenter()), weight*puw*leptonW);
                                    }
                                }
                             
                            }
                        }
                    }
                }
            }
        }
    }

    //store histograms for later merging
    histCollection.store("tempHists", begin, end);
}

void treeReader::splitJobs(){

    //clear previous histograms
    systemTools::system("rm tempHists/*");

    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample();
        for(long unsigned it = 0; it < nEntries; it+=500000){
            long unsigned begin = it;
            long unsigned end = std::min(nEntries, it + 500000);

            //make temporary job script 
            std::ofstream script("runTuples.sh");
            systemTools::initScript(script);
            script << "./dilepCR " << samples[currentSampleIndex].getUniqueName() << " " << std::to_string(begin) << " " << std::to_string(end);
            script.close();

            //submit job
            systemTools::submitScript("runTuples.sh", "02:00:00");
         }
    }
}

void treeReader::plot(const std::string& distName){

    //loop over all distributions and find the one to plot
    for(size_t d = 0; d < histInfo.size(); ++d){
        if(histInfo[d].name() == distName){
            std::vector<std::string> runCategorization2016 = {"all2016", "RunB", "RunC", "RunD", "RunE", "RunF", "RunG", "RunH"};
            std::vector<std::string> runCategorization2017 = {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"};
        
            //read collection for this distribution from files
            HistCollectionDist col2016("tempHists", histInfo[d], samples2016, { runCategorization2016, {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });
            HistCollectionDist col2017("tempHists", histInfo[d], samples2017, { runCategorization2017, {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });

            //if(distName == "closestJetDeepCsv"){
            if(distName == "leptonMvatZqTTV17"){
                //plot bdt before and after prefiring weights 
                TCanvas* c = new TCanvas("", "", 500, 500 );
                TH1D* DY = (TH1D*) col2017.access(2, {0, 3, 0, 1})->Clone();
                //TH1D* ttbar = (TH1D*) col2016.access(3, {0, 2, 0, 1})->Clone();
                TH1D* ttbar = (TH1D*) col2017.access(3, {0, 3, 0, 1})->Clone();
                DY->SetFillColor(kBlue);
                DY->SetLineColor(kBlue);
                DY->SetMarkerColor(kBlue);
                DY->SetMarkerStyle(0);
                DY->SetFillStyle(0);
                ttbar->SetFillColor(kRed);
                ttbar->SetLineColor(kRed);
                ttbar->SetFillStyle(0);
                ttbar->SetMarkerColor(kRed);
                ttbar->SetMarkerStyle(0);
                TLegend legend(0.2,0.8,0.8,0.9,NULL,"brNDC");
                legend.SetNColumns(2);
                legend.SetFillStyle(0); //avoid legend box
                legend.AddEntry( DY, "DY", "l");
                legend.AddEntry( ttbar, "TT", "l");
                DY->DrawNormalized("histE");
                ttbar->DrawNormalized("histEsame");
                legend.Draw("same");
                //c->SaveAs("closestDeepCSVComparison.pdf");
                c->SaveAs("leptonMVAComparison.pdf");

                TCanvas* c_log = new TCanvas("", "", 500, 500 );
                TLegend legend2(0.2,0.8,0.8,0.9,NULL,"brNDC");
                legend2.SetNColumns(2);
                legend2.SetFillStyle(0); //avoid legend box
                legend2.AddEntry( DY, "DY", "l");
                legend2.AddEntry( ttbar, "TT", "l");
                DY->DrawNormalized("histE");
                ttbar->DrawNormalized("histEsame");
                legend.Draw("same");
                c_log->SetLogy();
                //c_log->SaveAs("closestDeepCSVComparison_log.pdf");
                c_log->SaveAs("leptonMVAComparison_log.pdf");
       
            }
            //rebin same-sign category because of low statistics
            std::vector<std::string> notToRebin = {"nJets", "nForwardJets", "nBJets"};//distributions not  to rebin
            bool doNotRebin = false;
            for(auto& name : notToRebin){
                if(distName.find(name) != std::string::npos){
                    doNotRebin = true;
                    break;
                }
            }
            if(!doNotRebin){
                col2016.rebin("same-sign", 5);
                col2017.rebin("same-sign", 5);
            }

            unsigned is2016 = 0;
            unsigned is2017 = 1;

            //print plots for collection
            col2016.printPlots("plots/ewkino/dilepCR/2016", is2016, "ewkinoDilep", true, true);     //log
            col2016.printPlots("plots/ewkino/dilepCR/2016", is2016, "ewkinoDilep", false, true);    //linear

            col2017.printPlots("plots/ewkino/dilepCR/2017", is2017, "ewkinoDilep", true, true);     //log
            col2017.printPlots("plots/ewkino/dilepCR/2017", is2017, "ewkinoDilep", false, true);    //linear
        }
    }
}

void treeReader::splitPlots(){
    //tools::makeFileList("tempHists", "inputList.txt");
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        systemTools::initScript(script);
        script << "./dilepCR plot " << h.name();
        script.close();
        systemTools::submitScript("printPlots.sh", "00:30:00");
    }
}

int main(int argc, char* argv[]){
    treeReader reader;
    reader.setup();

    //convert all input to std::string format for easier handling
    std::vector<std::string> argvStr;
    for(int i = 0; i < argc; ++i){
        argvStr.push_back(std::string(argv[i]));
    }

    //no arguments given: full workflow of program
    if(argc == 1){
        std::cout << "Step 1: Distributing jobs on T2 grid" << std::endl;
        reader.splitJobs();
        std::cout << "Step 2: sleeping until jobs are finished" << std::endl;
        if( systemTools::runningJobs() ) std::cout << "jobs are running!" << std::endl;
        while( systemTools::runningJobs("runTuples.sh") ){
            systemTools::sleep(60);
        }
        std::cout << "Step 3: submitting plot jobs" << std::endl;
        reader.splitPlots();
        std::cout << "Program closing, plots will be dumped in specified directory soon" << std::endl;
    } 

    //single argument "run" given will do all computations and write output histograms to file
    else if(argc == 2 && argvStr[1] == "run"){
        reader.splitJobs();
    }

    //single argument "plot" given will submit all jobs for plotting from existing output files
    else if(argc == 2 && argvStr[1] == "plot"){
        reader.splitPlots();           
    }

    //arguments "plot" and distribution name given plots just this particular distribution
    else if(argc == 3 && argvStr[1] == "plot"){
        reader.plot(argvStr[2]);
    }

    //submit single histogram computation job
    else if(argc == 4){
        long unsigned begin = std::stoul(argvStr[2]);
        long unsigned end = std::stoul(argvStr[3]);
        //sample, first entry, last entry:
        reader.Analyze(argvStr[1], begin, end);
    }

    //invalid input given
    else{
        std::cerr << "Invalid input given to program: terminating!" << std::endl;
    }
    return 0;
}
