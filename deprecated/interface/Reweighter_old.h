#ifndef Reweighter_H
#define Reweighter_H

//include c++ library classes

//include ROOT classes
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"

//include other parts of code 
#include "../bTagSFCode/BTagCalibrationStandalone.h"
#include "../interface/Sample.h"

//Class storing scale-factor weights to be used in events
class Reweighter{
    public:
        Reweighter(const std::vector<Sample>&, const bool is2016, const std::string& bTagWorkingPoint = "medium");
        ~Reweighter();

        //pileup weight
        double puWeight(const double nTrueInt, const Sample&, const unsigned unc = 0) const;

        //b-tag weight
        double bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0) const;

        //b-tagging efficiency
        double bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta) const;

        //lepton id + reconstruction weight
        double muonTightWeight(const double pt, const double eta) const{ 
            return muonRecoWeight()*muonTightIdWeight(pt,eta);
        }

        double electronTightWeight(const double pt, const double superClusterEta) const{ 
            return electronRecoWeight(superClusterEta, pt)*electronTightIdWeight(pt,superClusterEta);
        }

        double muonLooseWeight(const double pt, const double eta) const{
            return  muonRecoWeight()*muonLooseIdWeight(pt,eta);
        }

        double electronLooseWeight(const double pt, const double superClusterEta) const{
            return electronRecoWeight(superClusterEta, pt)*electronLooseIdWeight(pt,superClusterEta);
        }

        //fakerates 
        double muonFakeRate(const double pt, const double eta, const unsigned unc = 0) const;
        double electronFakeRate(const double pt, const double eta, const unsigned unc = 0) const;

        //jet prefering probabilities
        double jetPrefiringProbability(const double pt, const double eta) const;

    private:
        //boolean flagging weights as 2016 or 2017
        bool is2016;

        //pu weights (one for every sample)
        std::map< std::string, std::vector< std::shared_ptr<TH1D> > > puWeights;

        //btag scale factors and efficiencies
        std::string bTagWP;
        std::shared_ptr<BTagCalibration> bTagCalib;
        std::shared_ptr<BTagCalibrationReader> bTagCalibReader;
        std::shared_ptr<TH2D> bTagEffHist[3];

        //reconstruction scale factors
        std::shared_ptr<TH2D> electronRecoSF_pT0to20;
        std::shared_ptr<TH2D> electronRecoSF_pT20toInf;

        //muon id scale factors
        std::shared_ptr<TH2D> muonLooseToRecoSF;
        std::shared_ptr<TH2D> muonTightToLooseSF;

        //electron id scale factors
        std::shared_ptr<TH2D> electronLooseToRecoSF;
        std::shared_ptr<TH2D> electronTightToLooseSF;

        //fake rate maps
        std::shared_ptr<TH2D> frMapEle[3];
        std::shared_ptr<TH2D> frMapMu[3];

        //jet prefiring probabilities
        std::shared_ptr<TH2D> prefiringMap;

        //initialize all weight histograms
        void initialize2016Weights();
        void initialize2017Weights();

        //return jet flavor index 1 -> 0, 4 -> 1, 5 -> 2
        unsigned flavorInd(const unsigned jetFlavor) const{ 
            return 0 + (jetFlavor == 4) + 2*(jetFlavor == 5);
        }

        //tracking + reconstruction weights
        double muonRecoWeight() const;
        double electronRecoWeight(const double superClusterEta, const double pt) const;

        //loose id weights
        double muonLooseIdWeight(const double pt, const double eta) const;
        double electronLooseIdWeight(const double pt, const double superClusterEta) const;

        //tight id weights
        double muonTightIdWeight(const double pt, const double eta) const;
        double electronTightIdWeight(const double pt, const double superClusterEta) const;

        //read pu weights for a given list of samples
        void initializePuWeights(const std::vector< Sample >&); 

        //read b-tagging weights
        void initializeBTagWeights();

        //read electron id and reco weights
        void initializeElectronWeights();

        //read muon id and reco weights 
        void initializeMuonWeights();

        //initialize fake-rate
        void initializeFakeRate();

        //initialize jet prefiring probabilities
        void initializePrefiringProbabilities();

        //initialize all weights 
        void initializeAllWeights(const std::vector< Sample>&);
};
#endif
