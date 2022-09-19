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

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/ewkinoTools.h"
#include "../interface/HistInfo.h"
#include "../interface/Category.h"
#include "../interface/HistCollection.h"
#include "../interface/trilepTools.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

int main(int argc, const char** argv){
    treeReader reader;
    reader.setup();
    /*
    if(argc < 2){
        reader.splitJobs();
    } else if(argc == 4){
        reader.Analyze(argv[1], std::stoi(argv[2]), std::stoi(argv[3]) );
    } else if(argc == 5){
        if(argv[4] != "submit"){
            reader.Analyze(argv[1], std::stoi(argv[2]), std::stoi(argv[3]) );
        } else{
            //std::system(std::string("echo \"./main " + std::string(argv[1]) + " " + std::string(argv[2]) + " " + std::string(argv[3]) + " > submit.sh").c_str());
            //std::system("qsub submit.sh -l walltime=04:00:00");
            std::cout << std::string("echo \"./main " + std::string(argv[1]) + " " + std::string(argv[2]) + " " + std::string(argv[3]) + " > submit.sh").c_str() << std::endl;
            std::cout << "qsub submit.sh -l walltime=04:00:00" << std::endl;
        }
    }
    */
    return 0;
}

void treeReader::setup(){
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples_dilepCR.txt");
    //info on kinematic distributions to plot
    //std::vector< HistInfo > histInfoV;
    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("sip3d", "SIP_{3D}", 100, 0, 8),
        HistInfo("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        HistInfo("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        HistInfo("miniIso", "miniIso", 100, 0, 0.4),
        HistInfo("leptonMva", "lepton MVA value", 100, -1, 1),
        HistInfo("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        HistInfo("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        HistInfo("closestJetCsv", "closest jet CSV", 100, 0, 1),
        HistInfo("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        HistInfo("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        HistInfo("mll", "M_{ll} (GeV)", 200, 12, 200),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        HistInfo("nVertex", "number of vertices", 100, 0, 100),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 5, 0, 8),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 5, 0, 8)
    };
    //HistCollection histCollection(histInfo, samples);
    /*
    const unsigned nDist = histInfoV.size(); //number of distributions to plot
    //initialize vector holding all histograms
    std::vector< std::vector< std::shared_ptr<TH1D> > > hists(nDist);
    for(unsigned dist = 0; dist < nDist; ++dist){
        hists[dist] = std::vector<std::shared_ptr<TH1D> >(samples.size());
        for(size_t sam = 0; sam < samples.size(); ++sam){
            hists[dist][sam] = histInfoV[dist].makeHist(samples[sam].getFileName()); 
        }
    }
    */
}


void treeReader::splitJobs(){
    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample(0);
        for(long unsigned split = 0; split < nEntries; split+=1000000){
            double min = split;
            double max = std::min(nEntries - 1, split + 999999);
            const char* mainArg[4] = {"./main", samples[currentSample].getFileName().c_str(), std::to_string(min).c_str(), std::to_string(max).c_str()};
            main(4, mainArg);
        }
    }
}

void treeReader::Analyze(const std::string& sampName, long unsigned begin, long unsigned end){
    for(auto it = samples.cbegin(); it != samples.cend(); ++it){
        if(it->getFileName() == sampName){
            Analyze(*it, begin, end);
            break;
        }
    }
}

void treeReader::Analyze(const Sample& samp, long unsigned begin, long unsigned end){
    initSample(samp, 0); 
    //store maxima of histograms for overflow bins
    const unsigned nDist = histInfo.size();
    double maxBin[nDist];
    for(unsigned dist = 0; dist < nDist; ++dist){
        maxBin[dist] = histInfo[dist].maxBinCenter();
    }
    //tweakable options
    const TString extra = ""; //for plot names

    double progress = 0; 	//for printing progress bar
    for(long unsigned it = 0; it < nEntries; ++it){
        GetEntry(samp, it);
        //vector containing good lepton indices
        std::vector<unsigned> ind;
        //select leptons
        const unsigned lCount = selectLep(ind);
        if(lCount != 3) continue;
        //require pt cuts (25, 20) to be passed
        if(!passPtCuts(ind)) continue;
        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
    }
    //store histograms

}
/*
void treeReader::printPlots(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //plot all distributions
    for(unsigned dist = 0; dist < nDist; ++dist){
        plotDataVSMC(mergedHists[dist][0], &mergedHists[dist][1], &proc[0], mergedHists[dist].size() - 1, "ewkino/" + std::get<0>(histInfo[dist]), "ewkino", false, true);             //linear plots
        plotDataVSMC(mergedHists[dist][0], &mergedHists[dist][1], &proc[0], mergedHists[dist].size() - 1, "ewkino/" + std::get<0>(histInfo[dist])  + "_log", "ewkino", true, true);    //log plots
    }
}
*/
