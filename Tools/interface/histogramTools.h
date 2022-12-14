#ifndef histogramTools_H
#define histogramTools_H

//include c++ library classes
#include <utility>

//include ROOT classes
#include "TH1.h"
#include "TH2.h"

namespace histogram{
    double minBinCenter( const TH1* );
    double maxBinCenter( const TH1* );
    double minXValue( const TH1* );
    double maxXValue( const TH1* );

    double minXBinCenter( const TH2* );
    double maxXBinCenter( const TH2* );
    double minXValue( const TH2* );
    double maxXValue( const TH2* );

    double minYBinCenter( const TH2* );
    double maxYBinCenter( const TH2* );
    double minYValue( const TH2* );
    double maxYValue( const TH2* );

    double contentAtValue( TH1*, const double value );
    double uncertaintyAtValue( TH1*, const double value );
    double uncertaintyDownAtValue( TH1*, const double value );
    double uncertaintyUpAtValue( TH1*, const double value );
    double contentDownAtValue( TH1*, const double value );
    double contentUpAtValue( TH1*, const double value );

    double contentAtValues( TH2*, const double valueX, const double valueY );
    double uncertaintyAtValues( TH2*, const double valueX, const double valueY );
    double uncertaintyDownAtValues( TH2*, const double valueX, const double valueY );
    double uncertaintyUpAtValues( TH2*, const double valueX, const double valueY );
    double contentDownAtValues( TH2*, const double valueX, const double valueY );
    double contentUpAtValues( TH2*, const double valueX, const double valueY );

    double boundedValue( const TH1* histPtr, double value );
    double boundedXValue( const TH2* histPtr, double xValue );
    double boundedYValue( const TH2* histPtr, double yValue );
    void fillValue( TH1*, const double value, const double weight );
    void fillValues( TH2*, const double valueX, const double valueY, const double weight );

    double totalUncertainty( const TH1* );
    std::pair< double, double > sumOfWeightsRatioAndUncertainty( const TH1*, const TH1* );

    void clipHistogram( TH1* histPtr );
    void copyHistogram( std::shared_ptr<TH1D> histlhs, std::shared_ptr<TH1D> histrhs);

}

#endif
