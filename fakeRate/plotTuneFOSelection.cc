/*
Code to tune to FO definition for light leptons.
The goal of the tuning is having a fake-rate (tight/FO) that is equal for light and heavy-flavor fakes 
*/

//include c++ library classes
#include <fstream>

//include ROOT classes
#include "TF1.h"

//include other parts of code 
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/Categorization.h"
#include "../Tools/interface/HistInfo.h"
#include "../plotting/interface/plotCode.h"
#include "../plotting/interface/tdrStyle.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/analysisTools.h"
#include "interface/CutsFitInfo.h"
#include "interface/fakeRateTools.h"
#include "interface/SlidingCut.h"

#include "interface/tuneFOSelectionTools.h"

// some help functions for histograms

std::vector< std::shared_ptr< TH1D > > getHistogramsFromFile(const std::string& fileName){
    // vector containing all histograms in current file
    std::vector< std::shared_ptr< TH1D > > histogramVector;
    // open file
    TFile* filePtr = TFile::Open( fileName.c_str() );
    // loop over keys in top level directory
    TList* keyList = filePtr->GetListOfKeys();
    for( const auto objectPtr : *keyList ){
	//try if a dynamic_cast to a histogram works to check if object is histogram
	TH1D* histPtr = dynamic_cast< TH1D* >( filePtr->Get( objectPtr->GetName() ) );
	if( histPtr ){
            //make sure histograms don't get deleted by root upon deletion of TDirectory above
            histPtr->SetDirectory( gROOT );
	    histogramVector.emplace_back( histPtr );
	}
    }
    // close file 
    filePtr->Close();
    return histogramVector;
}

unsigned numberOfEmptyBins( const TH1* hist ){
    // count number of "empty" bins (bin content < 1e-8)
    unsigned counter = 0;
    for( int b = 1; b < hist->GetNbinsX(); ++b ){
        if(  hist->GetBinContent(b) < 1e-8 ){
            ++counter;
        }
    }
    return counter;
}

bool ratioHasPathologicalBin( const TH1* hist ){
    // "pathological bin" = bin content > 50 and relative error > 0.5
    for( int b = 1; b < hist->GetNbinsX() + 1; ++b ){
        if( hist->GetBinContent(b) > 50 && hist->GetBinError(b)/hist->GetBinContent(b) > 0.5 ){
            return true;
        }
    }
    return false;
}

bool binIsLowStat( double content, double error ){
    return ( error/content >= 1. );
}

bool isLowStatistics( const TH1* hist ){
    const double maxLowStatFraction = 0.3;
    int nLowStatBins = 0;
    for( int b = 1; b < hist->GetNbinsX() + 1; ++b ){
        if( binIsLowStat( hist->GetBinContent(b), hist->GetBinError(b) ) ){
            ++nLowStatBins;
        }
    }
    double ret = static_cast<double>(nLowStatBins)/hist->GetNbinsX();
    return ret > maxLowStatFraction;
}

// main function 

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 3 ){
	return -1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2]; 

    fakeRate::checkFlavorString( flavor );
    analysisTools::checkYearString( year );

    // read histograms
    std::cout<<"reading histograms..."<<std::endl;
    std::string file_name = "tuneFOSelection_"+flavor+"_"+year;
    file_name.append("_histograms.root");

    std::vector< std::shared_ptr<TH1D> > heavyFlavorNumerator;
    std::vector< std::shared_ptr<TH1D> > heavyFlavorDenominator;
    std::vector< std::shared_ptr<TH1D> > lightFlavorNumerator;
    std::vector< std::shared_ptr<TH1D> > lightFlavorDenominator;
    
    std::vector< std::shared_ptr<TH1D> > allhistograms = getHistogramsFromFile(file_name);
    
    for(unsigned int i=0; i<allhistograms.size(); ++i){ 
	std::shared_ptr<TH1D> hist = allhistograms[i];
	if(stringTools::stringContains(hist->GetTitle(),"heavyFlavorNumerator")){
	    heavyFlavorNumerator.push_back(hist);
	} else if(stringTools::stringContains(hist->GetTitle(),"heavyFlavorDenominator")){
	    heavyFlavorDenominator.push_back(hist);
	} else if(stringTools::stringContains(hist->GetTitle(),"lightFlavorNumerator")){
	    lightFlavorNumerator.push_back(hist);
	} else if(stringTools::stringContains(hist->GetTitle(),"lightFlavorDenominator")){
	    lightFlavorDenominator.push_back(hist);
	} else {
	    std::cout<<"### WARNING ###: unrecognized histogram: "<<hist->GetTitle()<<std::endl;
	}
    }
    std::cout<<"found "<<heavyFlavorNumerator.size()<<" cut collections"<<std::endl;

    // read cut values
    // note: function is in src/tuneFOSelectionTools.cc
    std::map< std::string, double > cutParameters = getCutParameters();
    std::tuple< std::vector<double>, SlidingCutCollection, Categorization > temp = getCutCollection();
    std::vector<double> ptRatioCuts = std::get<0>(temp);
    SlidingCutCollection deepFlavorCutCollection = std::get<1>(temp);
    Categorization categories = std::get<2>(temp);

    // divide numerator and denominator histograms
    // ratio will be stored in what were orignally the numerator histograms
    std::cout<<"determining fake rate for "<<categories.size()<<" cut collections..."<<std::endl;
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        heavyFlavorNumerator[ c ]->Divide( heavyFlavorDenominator[ c ].get() );
        lightFlavorNumerator[ c ]->Divide( lightFlavorDenominator[ c ].get() );
    }

    // loop over all categories (FO ID's) and group the heavy/light ratio and additional info
    std::cout<<"determining heavy/light fakes ratio for "<<categories.size()<<" cut collections..."<<std::endl;
    CutsFitInfoCollection fitInfoCollection;
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){

        // make sure the fit under consideration has decent statistics 
	// number of empyt bins is 1 at maximum
        if( numberOfEmptyBins( heavyFlavorNumerator[ c ].get() ) > 1 || numberOfEmptyBins( lightFlavorNumerator[ c ].get() ) > 1 ) continue;
	// additional check on low statistics
        if( isLowStatistics( heavyFlavorNumerator[ c ].get() ) || isLowStatistics( lightFlavorNumerator[ c ].get() ) ) continue;
        
	// divide heavy flavor fake rate by light flavor fake rate to get the ratio
	std::shared_ptr< TH1 > ratio( dynamic_cast< TH1*>( heavyFlavorNumerator[ c ]->Clone() ) );
        ratio->Divide( lightFlavorNumerator[ c ].get() );
        if( ratioHasPathologicalBin( ratio.get() ) ) continue;

        std::map< std::string, double > cutMap;
        auto indices = categories.indices( c );
        cutMap["pTRatio"] =  ptRatioCuts[ indices[ 0 ] ];
        cutMap["deepFlavor_left"] = deepFlavorCutCollection[ indices[ 1 ] ].cut( cutParameters["minSlidePt"] );
        cutMap["deepFlavor_right"] = deepFlavorCutCollection[ indices[ 1 ] ].cut( cutParameters["maxSlidePt"] );
        fitInfoCollection.push_back( CutsFitInfo( heavyFlavorNumerator[ c ], lightFlavorNumerator[ c ], cutMap ) );
    }
    
    // we will try 3 different optimization metrics for the fit: 
    // 1 : minimum difference between fit value and 1 
    // 2 : smallest Chi2 of constant fit
    // 3 : define a loss function as | fitValue - 1 + epsilon | * | chi2 - min( 1, chi2 ) + epsilon | and minimize it

    std::cout<<"making plots..."<<std::endl;
    std::string plotDirectory = "tuningPlots_" + flavor + "_" + year + "/";

    //make plotting directory if it does not already exist
    systemTools::makeDirectory( plotDirectory );

    int keepNbest = 20;
    std::cout << "FO tuning results for "+year+" "+flavor+"s"<<std::endl;
    std::cout << "--------------------------------------------------------------------" <<std::endl;
    std::cout << "best cut collections according to difference from unity" << std::endl;
    fitInfoCollection.sortByDiffFromUnity();
    fitInfoCollection.printBestCuts( keepNbest );
    fitInfoCollection.plotBestCuts( keepNbest, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "best cut collections according to chi2 of fit" << std::endl;
    fitInfoCollection.sortByChi2();
    fitInfoCollection.printBestCuts( keepNbest );
    fitInfoCollection.plotBestCuts( keepNbest, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "best cut collections according to loss function" << std::endl;
    constexpr double epsilon = 0.01;
    fitInfoCollection.sortByLossFunction( epsilon );
    fitInfoCollection.printBestCuts( keepNbest );
    fitInfoCollection.plotBestCuts( keepNbest, plotDirectory );
  
    std::cerr << "###done###" << std::endl; 
    return 0;
}
