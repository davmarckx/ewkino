#include "../interface/CutsFitInfo.h"

//include c++ library classes 
#include <cmath>
#include <limits>
#include <algorithm>

//include ROOT classes 
#include "TF1.h"

//include other parts of code 
#include "../../plotting/interface/plotCode.h"
#include "../../plotting/interface/tdrStyle.h"
#include "../../Tools/interface/stringTools.h"


//CutsFitInfo::CutsFitInfo( const std::map< std::string, double >& cutMap, const double fitVal, const double normalizedChi2 ) :
CutsFitInfo::CutsFitInfo( const std::shared_ptr< TH1D >& heavyFlavorFakeRateHist, const std::shared_ptr< TH1D >& lightFlavorFakeRateHist, const std::map< std::string, double >& cutMap ) :
    heavyFlavorFakeRate( heavyFlavorFakeRateHist ),
    lightFlavorFakeRate( lightFlavorFakeRateHist ),
    cutValueMap( cutMap )
{

    //TO ADD ERROR CHECKING BETWEEN HISTOGRAMS 

    //compute ratio of fake-rate histograms 
    fakeRateRatio = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( heavyFlavorFakeRate->Clone() ) );
    fakeRateRatio->Divide( lightFlavorFakeRateHist.get() );

    fitInfo = ConstantFit( fakeRateRatio );
}


double CutsFitInfo::cut( const std::string& cutVariableName ) const{
    auto it = cutValueMap.find( cutVariableName );
    if( it == cutValueMap.cend() ){
        throw std::invalid_argument( "Requested cut for unknown variable " + cutVariableName + "." );
    } else {
        return it->second;
    }
}


std::ostream& operator<<( std::ostream& os, const CutsFitInfo& fitInfo ){
    for( const auto& entry : fitInfo.cutValueMap ){
        os << entry.first << " cut = " << entry.second << "\t";
    } 
    os << "fit value = " << fitInfo.fitValue() << "\tnormalized chi2 = " << fitInfo.normalizedChi2();
    return os;
}


std::string CutsFitInfo::cutsString() const{
    std::string name = "";
    for( const auto& entry : cutValueMap ){
        std::string cut = entry.first + stringTools::doubleToString( entry.second, 3 );
        name += ( cut + "_" );
    }
    name = name.substr(0, name.size() - 1);
    name = stringTools::replace( name, ".", "p" );
    return name;
}


void CutsFitInfo::makePlots( const std::string& outputDirectory ) const{

    //plot ratio of fake-rates
    fakeRateRatio->GetYaxis()->SetTitle("heavy-flavor/light-flavor fake-rate");
    plotHistograms( fakeRateRatio.get(), "light/heavy fake-rate", stringTools::formatDirectoryName( outputDirectory ) + "fakeRateRatio_" + cutsString(), false);

    //plot the fake-rates together
    heavyFlavorFakeRate->GetYaxis()->SetTitle( "fake-rate" );

    TH1D* histos[2] = { heavyFlavorFakeRate.get(), lightFlavorFakeRate.get() };
    const std::string names[2] = { "heavy-flavor", "light-flavor" };

    plotHistograms( histos, 2, names, stringTools::formatDirectoryName( outputDirectory ) + "fakeRateComparison_" + cutsString(), false, false ); 
}
    


void CutsFitInfoCollection::sortByChi2(){
    std::sort( collection.begin(), collection.end(), []( const CutsFitInfo& lhs, const CutsFitInfo& rhs ){ return lhs.normalizedChi2() < rhs.normalizedChi2(); } );
}


void CutsFitInfoCollection::sortByDiffFromUnity(){
    std::sort( collection.begin(), collection.end(), [this]( const CutsFitInfo& lhs, const CutsFitInfo& rhs ){ return this->diffFromUnity( lhs ) < this->diffFromUnity( rhs ); } );
}


void CutsFitInfoCollection::sortByLossFunction( const double epsilon ){
    std::function< double (const CutsFitInfo& ) > valueLoss = [ epsilon, this ]( const CutsFitInfo& x ){ return ( this->diffFromUnity( x ) + epsilon ); };
    std::function< double (const CutsFitInfo& ) > chi2Loss = [ epsilon ]( const CutsFitInfo& x ){ return ( fabs( x.normalizedChi2() - std::min( 1., x.normalizedChi2() ) ) + epsilon ); };
    std::function< double (const CutsFitInfo& ) > loss = [ valueLoss, chi2Loss ]( const CutsFitInfo& x ){ return valueLoss( x )*chi2Loss( x ); };

    std::sort( collection.begin(), collection.end(), [&loss]( const CutsFitInfo& lhs, const CutsFitInfo& rhs ){ return loss( lhs ) < loss( rhs ); } );
}


void CutsFitInfoCollection::printBestCuts( size_type numberOfCuts, std::ostream& os ) const{
    if( numberOfCuts == 0 ) numberOfCuts = size();
    if( numberOfCuts > size() ) numberOfCuts = size();
    for( size_type i = 0; i < numberOfCuts; ++i ){
        os << collection[i] << std::endl;
    }
}


void CutsFitInfoCollection::plotBestCuts( size_type numberOfCuts, const std::string& outputDirectory ) const{
    if( numberOfCuts == 0 ) numberOfCuts = size();
    if( numberOfCuts > size() ) numberOfCuts = size();
    for( size_type i = 0; i < numberOfCuts; ++i ){
        collection[i].makePlots( outputDirectory );
    }
}
