#include "../interface/histogramTools.h"

//include c++ library tools
#include <algorithm>
#include <cmath>


// get bin centers and edges

double histogram::minBinCenter( const TH1* histPtr ){
    return histPtr->GetBinCenter( 1 );
}

double histogram::maxBinCenter( const TH1* histPtr ){
    return histPtr->GetBinCenter( histPtr->GetNbinsX() );
}

double histogram::minXValue( const TH1* histPtr ){
    return histPtr->GetBinLowEdge( 1 );
}

double histogram::maxXValue( const TH1* histPtr ){
    return histPtr->GetBinLowEdge( histPtr->GetNbinsX() ) + histPtr->GetBinWidth( histPtr->GetNbinsX() );
}

double histogram::minXBinCenter( const TH2* histPtr ){
    return histPtr->GetXaxis()->GetBinCenter( 1 );
}

double histogram::maxXBinCenter( const TH2* histPtr ){
    return histPtr->GetXaxis()->GetBinCenter( histPtr->GetNbinsX() );
}

double histogram::minXValue( const TH2* histPtr ){
    return histPtr->GetXaxis()->GetBinLowEdge( 1 );
}

double histogram::maxXValue( const TH2* histPtr ){
    return histPtr->GetXaxis()->GetBinUpEdge( histPtr->GetNbinsX() );
}

double histogram::minYBinCenter( const TH2* histPtr ){
    return histPtr->GetYaxis()->GetBinCenter( 1 );
}

double histogram::maxYBinCenter( const TH2* histPtr ){
    return histPtr->GetYaxis()->GetBinCenter( histPtr->GetNbinsY() );
}

double histogram::minYValue( const TH2* histPtr ){
    return histPtr->GetYaxis()->GetBinLowEdge( 1 );
}

double histogram::maxYValue( const TH2* histPtr ){
    return histPtr->GetYaxis()->GetBinUpEdge( histPtr->GetNbinsY() );
}


// fill histogram taking into account under- and overflow

double boundedValue( const TH1* histPtr, double value ){
    return std::max( histogram::minBinCenter( histPtr ), std::min( value, histogram::maxBinCenter( histPtr ) ) );
}

double boundedXValue( const TH2* histPtr, double xValue ){
    return std::max( histogram::minXBinCenter( histPtr ), std::min( xValue, histogram::maxXBinCenter( histPtr ) ) );
}

double boundedYValue( const TH2* histPtr, double yValue ){
    return std::max( histogram::minYBinCenter( histPtr ), std::min( yValue, histogram::maxYBinCenter( histPtr ) ) );
}

void histogram::fillValue( TH1* histPtr, const double value, const double weight ){
        histPtr->Fill( boundedValue( histPtr, value ), weight );
}

void histogram::fillValues( TH2* histPtr, const double valueX, const double valueY, const double weight ){
        histPtr->Fill( boundedXValue( histPtr, valueX ), boundedYValue( histPtr, valueY ), weight );
}


// get bin content and bin error at values

double histogram::contentAtValue( TH1* histPtr, const double value ){
    return histPtr->GetBinContent( histPtr->FindBin( boundedValue( histPtr, value ) ) );
}

double histogram::uncertaintyAtValue( TH1* histPtr, const double value ){
    return histPtr->GetBinError( histPtr->FindBin( boundedValue( histPtr, value ) ) );
}

double histogram::uncertaintyDownAtValue( TH1* histPtr, const double value ){
    return histPtr->GetBinErrorLow( histPtr->FindBin( boundedValue( histPtr, value ) ) );
}

double histogram::uncertaintyUpAtValue( TH1* histPtr, const double value ){
    return histPtr->GetBinErrorUp( histPtr->FindBin( boundedValue( histPtr, value ) ) );
}

double histogram::contentDownAtValue( TH1* histPtr, const double value ){
    return ( histogram::contentAtValue( histPtr, value ) - histogram::uncertaintyDownAtValue( histPtr, value ) );
}

double histogram::contentUpAtValue( TH1* histPtr, const double value ){
    return ( histogram::contentAtValue( histPtr, value ) + histogram::uncertaintyUpAtValue( histPtr, value ) );
}

double histogram::contentAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return histPtr->GetBinContent( histPtr->FindBin( boundedXValue( histPtr, valueX ), boundedYValue( histPtr, valueY ) ) );
}

double histogram::uncertaintyAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return histPtr->GetBinError( histPtr->FindBin( boundedXValue( histPtr, valueX ), boundedYValue( histPtr, valueY ) ) );
}

double histogram::uncertaintyDownAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return histPtr->GetBinErrorLow( histPtr->FindBin( boundedXValue( histPtr, valueX ), boundedYValue( histPtr, valueY ) ) );
}

double histogram::uncertaintyUpAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return histPtr->GetBinErrorUp( histPtr->FindBin( boundedXValue( histPtr, valueX ), boundedYValue( histPtr, valueY ) ) );
}

double histogram::contentDownAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return ( histogram::contentAtValues( histPtr, valueX, valueY ) - histogram::uncertaintyDownAtValues( histPtr, valueX, valueY ) );
}

double histogram::contentUpAtValues( TH2* histPtr, const double valueX, const double valueY ){
    return ( histogram::contentAtValues( histPtr, valueX, valueY ) + histogram::uncertaintyUpAtValues( histPtr, valueX, valueY ) );
}


// tools for uncertainty calculation

double histogram::totalUncertainty( const TH1* histPtr ){
    double sumUncSquared = 0.;
    for( int bin = 1; bin < histPtr->GetNbinsX() + 1; ++bin ){
        double binError = histPtr->GetBinError( bin );
        sumUncSquared += binError * binError;
    }
    return std::sqrt( sumUncSquared );
}

std::pair< double, double > histogram::sumOfWeightsRatioAndUncertainty( const TH1* numeratorPtr, const TH1* denominatorPtr ){
    double fractionalUncNumerator =  histogram::totalUncertainty( numeratorPtr ) / numeratorPtr->GetSumOfWeights();
    double fractionalUncDenominator = histogram::totalUncertainty( denominatorPtr ) / denominatorPtr->GetSumOfWeights();
    double sumOfWeightsRatio = numeratorPtr->GetSumOfWeights() / denominatorPtr->GetSumOfWeights();
    double totalRatioUnc = std::sqrt( fractionalUncNumerator * fractionalUncNumerator + fractionalUncDenominator * fractionalUncDenominator );
    return { sumOfWeightsRatio, sumOfWeightsRatio * totalRatioUnc };
}


// other histogram manipulations

void histogram::clipHistogram( TH1* histPtr ){
    // put all bins of a histogram to minimum zero (no net negative weights)
    for(int i=0; i<histPtr->GetNbinsX()+2; ++i){
        if(histPtr->GetBinContent(i)<0.){
            histPtr->SetBinContent(i,0.);
            histPtr->SetBinError(i,0.);
        }
    }
}

void histogram::copyHistogram( std::shared_ptr<TH1D> histlhs, std::shared_ptr<TH1D> histrhs){
    // copy bin content of histrhs into histlhs while keeping original name and title
    histlhs->Reset();
    for(int i=0; i<histrhs->GetNbinsX()+2; ++i){
        histlhs->SetBinContent(i,histrhs->GetBinContent(i));
    }
}
