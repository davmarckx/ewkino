
//include c++ library classes
#include <string>
#include <iostream>
#include <fstream>

//include ROOT classes
#include "TFile.h"
#include "TH1D.h"
#include "TROOT.h"

//include other parts of code
#include "../interface/Sample.h"


std::shared_ptr<TH1D> rebinHistogram(const std::shared_ptr<TH1D>& oldHist, const unsigned newNBins){
    const unsigned oldNBins = oldHist->GetNbinsX();  

    bool error = false;
    if(newNBins > oldNBins){
        std::cerr << "Error: trying to rebin histogram to a larger amount of bins than it originally had!" << std::endl;
        error = true;
    }

    if(oldNBins%newNBins != 0){
        std::cerr << "Error: the new number of bins is not a divisor of the old number of bins when rebinning the histogram!" << std::endl;
        error = true;
    }

    if(error){
        return std::shared_ptr<TH1D>( (TH1D*) nullptr );
    }

    if(oldNBins == newNBins){
        std::cerr << "Warning: attempting to rebin histogram to original amount of bins. Returning clone of the original histogram" << std::endl;
        return std::shared_ptr<TH1D>( (TH1D*) oldHist->Clone() );
    }

    //range of new histogram
    double min = oldHist->GetBinLowEdge(1);
    double max = ( oldHist->GetBinLowEdge(oldNBins) + oldHist->GetBinWidth(oldNBins) )/(oldNBins/newNBins);

    std::shared_ptr<TH1D> newHist = std::make_shared<TH1D>( std::string(std::string(oldHist->GetName()) + "_new").c_str(), oldHist->GetTitle(), newNBins, min, max); 
    for(unsigned b = 1; b < newNBins + 1; ++b){
        newHist->SetBinContent(b, oldHist->GetBinContent(b));   
        newHist->SetBinError(b, oldHist->GetBinError(b));   
    }
    
    return newHist;
}


void extractPuWeights(const Sample& sample){

    //no weights to be determined for data
    if( sample.isData() ){
        std::cerr << "Error: trying to extract pu weights for data sample, returning control." << std::endl;
        return;
    }

    //skip 2016 samples in the 2017 sample list
    if( sample.is2017() && sample.getFileName().find("Summer16") != std::string::npos){
        std::cout << "Skipping 2016 sample : " << sample.getFileName() << " from 2017 list, it should also be included in the 2016 list" << std::endl;
        return;
    }

    //directory where ntuples are stored 
    //const std::string directory = "~/Work/ntuples_tzq/";

    //read MC pu distribution from given MC sample
    std::shared_ptr<TFile> mcInputFile = sample.getFile( );
    std::shared_ptr<TH1D> mcPuDist = std::shared_ptr<TH1D>( (TH1D*) mcInputFile->Get("blackJackAndHookers/nTrueInteractions") );
    mcPuDist->SetDirectory(gROOT);
    mcInputFile->Close();

    //normalize histogram to unity
    mcPuDist->Scale(1./mcPuDist->GetSumOfWeights());

    //pu weights
    std::shared_ptr<TH1D> puWeights[14][3];
   
    //categorization by year, run era and ucertainty
    const std::vector< std::string > eras2016 = {"2016Inclusive", "2016B", "2016C", "2016D", "2016E", "2016F", "2016G", "2016H"};
    const std::vector< std::string > eras2017  = {"2017Inclusive", "2017B", "2017C", "2017D", "2017E", "2017F"};
    std::vector< std::string > allEras = eras2016;
    allEras.insert(allEras.begin(), eras2017.begin(), eras2017.end() );
    const std::string uncertainty[3] = {"central", "down", "up"};

    for(unsigned e = 0; e < allEras.size(); ++e){
        for(unsigned unc = 0; unc < 3; ++unc){

            //different location for 2016 and 2017 pu weights
            std::string year = allEras[e].substr(0, 4); 

            //read data pu distributions 
            TFile* dataFile = TFile::Open( (const TString&) "weights/pileUpData/" + year + "/dataPuHist_" + allEras[e] + "_" + uncertainty[unc] + ".root");
            std::shared_ptr<TH1D> dataPuDist = std::shared_ptr<TH1D>( (TH1D*) dataFile->Get("pileup") );
            dataPuDist->SetDirectory(gROOT);

            //make a copy of the data PU profile as the numerator 
            std::shared_ptr<TH1D> numerator = std::shared_ptr<TH1D> ( (TH1D*) dataPuDist->Clone() );
            numerator->SetDirectory(gROOT);

            //normalize histogram to unity
            numerator->Scale(1./numerator->GetSumOfWeights());

            //make a copy of the MC PU profile as the denominator
            std::shared_ptr<TH1D> denominator = std::shared_ptr<TH1D>( (TH1D*) mcPuDist->Clone() );
            denominator->SetDirectory(gROOT);


            //rebin denominator or numerator histogram if needed
            if( sample.is2016() && (year == "2017") ){
                numerator = rebinHistogram(numerator, 50);
            } else if( sample.is2017() && (year == "2016") ){
                denominator = rebinHistogram(denominator, 50);
            }

            //divide data and MC shapes
            numerator->Divide(denominator.get());

            puWeights[e][unc] = std::shared_ptr< TH1D >( (TH1D*) numerator->Clone() );
            puWeights[e][unc]->SetDirectory(gROOT);

            //needed to set ownership to memory?
            dataFile->Close();
        }
    }

    //write pu Weights to file
    TFile* outputFile = TFile::Open( (const TString&) "weights/pileUpWeights/puWeights_" + sample.getFileName(), "RECREATE");
    for(unsigned e = 0; e < allEras.size(); ++e){
        for(unsigned unc = 0; unc < 3; ++unc){
            puWeights[e][unc]->Write((const TString&) "puw_Run" + allEras[e] + "_" + uncertainty[unc] );
        }
    }
    outputFile->Close();
}

int main(int argc, char* argv[]){

    //list of samples
    std::vector< Sample > sampleVector = readSampleList( "sampleLists/samples_dilepCR_2016.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino" );

    //read sample lists from txt 
    std::vector< Sample > sampleVector2017 = readSampleList( "sampleLists/samples_dilepCR_2017.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino" );
    for( auto& samp : sampleVector2017 ){
        sampleVector.push_back( samp );
    }

    for(const auto& sample : sampleVector){
        if(sample.isData()) continue;
        std::cout << "Extracting weights for " << sample.getFileName() << std::endl;
        extractPuWeights(sample);
    }
    return 0;
}
