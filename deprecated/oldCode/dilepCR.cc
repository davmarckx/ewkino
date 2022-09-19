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
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

void treeReader::Analyze(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples_dilepCR.txt");
    //info on kinematic distributions to plot
    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfo;
    //name      xlabel    nBins,  min, max
    histInfo = {
        std::make_tuple("sip3d", "SIP_{3D}", 100, 0, 8),
        std::make_tuple("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        std::make_tuple("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        std::make_tuple("miniIso", "miniIso", 100, 0, 0.4),
        std::make_tuple("leptonMvaSUSY", "SUSY lepton MVA value", 100, -1, 1),
        std::make_tuple("leptonMvaTTH", "TTH lepton MVA value", 100, -1, 1),
        std::make_tuple("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        std::make_tuple("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        std::make_tuple("closestJetCsv", "closest jet CSV", 100, 0, 1),
        std::make_tuple("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        std::make_tuple("electronMvaGP", "electron GP MVA value", 100, -1, 1),
        std::make_tuple("muonSegComp", "muon segment compatibility", 100, 0, 1),
        std::make_tuple("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        std::make_tuple("mll", "M_{ll} (GeV)", 200, 12, 200),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        std::make_tuple("nVertex", "number of vertices", 100, 0, 100),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8)
    };
    //read pu weights for every period
    TFile* puFile = TFile::Open("weights/puWeights2017.root");
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    TH1D* puWeights[6];
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e] = (TH1D*) puFile->Get( (const TString&) "puw_Run" + eras[e]);
    }
    //split histograms in run periods
    //const unsigned nRuns = 7;
    //const std::string runNames[nRuns] = {"all2017", "RunA", "RunB", "RunC", "RunD", "RunE", "RunF"};
    const unsigned nRuns = 6;
    const std::string runNames[nRuns] = {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"};
    //split histograms in flavor combinations
    const unsigned nFlav = 4;
    const std::string flavNames[nFlav] = {"inclusive", "ee", "em", "mm"};
    const unsigned nJetCat = 2;
    const std::string jetNames[nJetCat] = {"nJetsInclusive", "1pt40Jet"};
    const unsigned nPuRew = 2;
    const std::string puNames[nPuRew] = {"noPuW", "PuW"};

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::vector < std::vector< std::vector< TH1D* > > > > > > hists(nJetCat);
    for(unsigned j = 0; j < nJetCat; ++j){
        hists[j] = std::vector< std::vector < std::vector< std::vector < std::vector< TH1D* > > > > >(nPuRew);
        for(unsigned pu = 0; pu < nPuRew; ++pu){
            hists[j][pu] = std::vector < std::vector< std::vector < std::vector< TH1D* > > > > (nRuns);
            for(unsigned run = 0; run < nRuns; ++run){
                hists[j][pu][run] = std::vector< std::vector < std::vector< TH1D* > > > (nFlav);
                for(unsigned flav = 0; flav < nFlav; ++flav){
                    hists[j][pu][run][flav] = std::vector < std::vector< TH1D* > > (nDist);
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        hists[j][pu][run][flav][dist] = std::vector< TH1D*> (samples.size());
                        for(size_t sam = 0; sam < samples.size(); ++sam){
                            hists[j][pu][run][flav][dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + flavNames[flav] + runNames[run] + jetNames[j] + puNames[pu]), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + flavNames[flav] + runNames[run]  + jetNames[j] + puNames[pu] + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
                        }
                    }
                }
            }
        }
    }

    //store maxima of histograms for overflow bins
    double maxBin[nDist];
    for(unsigned dist = 0; dist < nDist; ++dist){
        maxBin[dist] = std::get<4>(histInfo[dist]) - 0.5*(std::get<4>(histInfo[dist]) - std::get<3>(histInfo[dist]) )/std::get<2>(histInfo[dist]);
    }
    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        initSample(1);  //use 2017 lumi
        std::cout<<"Entries in "<< std::get<1>(samples[sam]) << " " << nEntries << std::endl;
        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries; ++it){
            //print progress bar	
            /*
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            */
            if(it % 1000000 == 0) std::cout << "." << std::flush;
            GetEntry(it);
            //vector containing good lepton indices
            std::vector<unsigned> ind;
            //select leptons
            const unsigned lCount = selectLep(ind);
            if(lCount != 2) continue;
            //require pt cuts (25, 20) to be passed
            if(!passPtCuts(ind)) continue;
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //Cut of Mll at 12 GeV
            if((lepV[0] + lepV[1]).M() < 12) continue;
            //reject SS events
            if(_lCharge[ind[0]] == _lCharge[ind[1]]) continue;
            //determine flavor compositions
            unsigned flav = dilFlavorComb(ind) + 1; //reserve 0 for inclusive
            //order jets and check the number of jets
            std::vector<unsigned> jetInd;
            unsigned jetCount = nJets(jetInd);

            //Extra category selection: for DY select onZ, for ttbar select 1 b-jet 2-jets
            if(flav == 1 || flav == 3){ //OSSF
                if(fabs((lepV[0] + lepV[1]).M() - 91) > 10) continue;
                if(nBJets(0,  true, false, 0) != 0) continue;
            } else if(flav == 2){
                if(_met < 50) continue;
                if(jetCount < 2 || nBJets() < 1) continue;
            }
            //determine run perios
            unsigned run;
            run = ewk::runPeriod(_runNb) + 1 - 1; //reserve 0 for inclusive // -1 because we start at run B 
            //max pu to extract
            float max = (run == 0 || run > 3) ? 70 : 60;
            //float max = 60;
            //loop over leading leptons
            for(unsigned l = 0; l < 2; ++l){
                double fill[12] = {_3dIPSig[ind[l]], fabs(_dxy[ind[l]]), fabs(_dz[ind[l]]), _miniIso[ind[l]], _leptonMvaSUSY[ind[l]], _leptonMvaTTH[ind[l]], _ptRel[ind[l]], _ptRatio[ind[l]], _closestJetCsvV2[ind[l]], (double) _selectedTrackMult[ind[l]], (_lFlavor[ind[l]] == 0) ? _lElectronMva[ind[l]] : 0,  (_lFlavor[ind[l]] == 1) ? _lMuonSegComp[ind[l]] : 0};
                //fill histograms
                for(unsigned j = 0; j < nJetCat; ++j){
                    if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                    for(unsigned pu = 0; pu < nPuRew; ++pu){
                        for(unsigned dist = 0; dist < 12; ++dist){
                            for(unsigned r = 0; r < nRuns; ++r){
                                if(sam != 0 || r == run || r == 0){
                                    double puw = 1.;
                                    if(sam != 0 && pu == 1){
                                        puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                                    }
                                    hists[j][pu][r][flav][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight*puw);
                                    hists[j][pu][r][0][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight*puw);
                                }
                            }
                        }
                    }
                }
            }
            double fill[nDist - 12] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], (double) _nVertex, (double) jetCount, (double) nBJets(0, false), (double) nBJets()}; //replace 0 by _met for correct trees
            for(unsigned j = 0; j < nJetCat; ++j){
                if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 12; dist < nDist; ++dist){
                        for(unsigned r = 0; r < nRuns; ++r){
                            if(sam != 0 || r == run || r == 0){
                                double puw = 1.;
                                if(sam != 0 && pu == 1){
                                    puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                                }
                                hists[j][pu][r][flav][dist][sam]->Fill(std::min(fill[dist - 12], maxBin[dist]), weight*puw);
                                hists[j][pu][r][0][dist][sam]->Fill(std::min(fill[dist - 12], maxBin[dist]), weight*puw);
                            }
                        }
                    }
                } 
            }
        }
        //set histograms to 0 if negative
        for(unsigned j = 0; j < nJetCat; ++j){
            for(unsigned pu = 0; pu < nPuRew; ++pu){
                for(unsigned run = 0; run < nRuns; ++run){
                    for(unsigned flav = 0; flav < nFlav; ++flav){
                        for(unsigned dist = 0; dist < nDist; ++dist){
                            tools::setNegativeZero(hists[j][pu][run][flav][dist][sam]);
                        }	
                    }
                }
            }
        }
        std::cout << std::endl;
    }
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"obs.", "DY", "TT + Jets", "WJets", "VV", "TT + X", "T + X"};
    std::vector< std::vector <std::vector < std::vector< std::vector < std::vector < TH1D*> > > > > > mergedHists(nJetCat);
    for(unsigned j = 0; j < nJetCat; ++j){
        mergedHists[j] = std::vector <std::vector < std::vector< std::vector < std::vector < TH1D*> > > > >(nPuRew);
        for(unsigned pu = 0; pu < nPuRew; ++pu){
            mergedHists[j][pu] = std::vector <std::vector < std::vector< std::vector < TH1D*> > > >(nRuns);
            for(unsigned run = 0; run < nRuns; ++run){
                mergedHists[j][pu][run] = std::vector <std::vector < std::vector < TH1D*> > > (nFlav);
                for(unsigned flav = 0; flav < nFlav; ++flav){
                    mergedHists[j][pu][run][flav] = std::vector <std::vector < TH1D*> > (nDist);
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        mergedHists[j][pu][run][flav][dist] = std::vector<TH1D*>(proc.size());
                        for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                            mergedHists[j][pu][run][flav][dist][m] = (TH1D*) hists[j][pu][run][flav][dist][sam]->Clone();
                            while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                                mergedHists[j][pu][run][flav][dist][m]->Add(hists[j][pu][run][flav][dist][sam + 1]);
                                ++sam;
                            }
                            ++sam;
                        }
                    }
                }
            }
        }
    }
    //plot all distributions
    //const std::string runString[nRuns] = {"32.5 fb^{-1} (13 TeV)", "2017 Run A", "2017 Run B", "2017 Run C", "2017 Run D", "2017 Run E", "2017 Run F"};
    const std::string runString[nRuns] = {"42 fb^{-1} (13 TeV)", "2017 Run B", "2017 Run C", "2017 Run D", "2017 Run E", "2017 Run F"};
    const std::string flavString[nFlav] = {"", "ee : ", "e#mu : ", "#mu#mu : "};
    for(unsigned j = 0; j < nJetCat; ++j){
        for(unsigned pu = 0; pu < nPuRew; ++pu){
            for(unsigned run = 0; run < nRuns; ++run){
                for(unsigned flav = 0; flav < nFlav; ++flav){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        plotDataVSMC(mergedHists[j][pu][run][flav][dist][0], &mergedHists[j][pu][run][flav][dist][1], &proc[0], mergedHists[j][pu][run][flav][dist].size() - 1, "ewkino/dilepCR/" + runNames[run] + "/" + flavNames[flav] + "/" + jetNames[j] + "/" + puNames[pu] + "/" + std::get<0>(histInfo[dist]) + "_" + flavNames[flav] + "_" + runNames[run], "ewkinoDilep", false, true, flavString[flav] + runString[run]);             //linear plots
                        plotDataVSMC(mergedHists[j][pu][run][flav][dist][0], &mergedHists[j][pu][run][flav][dist][1], &proc[0], mergedHists[j][pu][run][flav][dist].size() - 1, "ewkino/dilepCR/" + runNames[run] + "/" + flavNames[flav] + "/" + jetNames[j] + "/" + puNames[pu] + "/" + std::get<0>(histInfo[dist]) + "_" + flavNames[flav] + "_" + runNames[run] + "_log", "ewkinoDilep", true, true, flavString[flav] + runString[run]);     //log plots
                    }
                }
            }
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
