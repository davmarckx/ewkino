#include "../interface/Efficiency.h"

//include c++ standard library classes 
#include <iostream>

Efficiency::Efficiency(const std::string& name, const HistInfo& info, const bool sideband) : 
    histInfo( &info )
{
    numerator = histInfo->makeHist( name + "_numerator" );
    numerator->Sumw2();
    denominator = histInfo->makeHist( name + "_denominator" );
    denominator->Sumw2();

    if(sideband){
        numerator_sideband = histInfo->makeHist(name + "_numerator_sideband");
        numerator_sideband->Sumw2();
        denominator_sideband = histInfo->makeHist(name + "_denominator_sideband");
        denominator_sideband->Sumw2();
    }
}


void Efficiency::fill_numerator(const double entry, const double weight) const{
    numerator->Fill( entryToFill(entry), weight ); 
    denominator->Fill( entryToFill(entry), weight ); 
}


void Efficiency::fill_denominator(const double entry, const double weight) const{
    denominator->Fill( entryToFill(entry), weight );
}


void Efficiency::fill_numerator_sideband(const double entry, const double weight){
    
    //make sure sideband will be re-subtracted when it gets new entries
    sidebandWasSubtracted = false;
    numerator_sideband->Fill( entryToFill(entry), weight );
    denominator_sideband->Fill( entryToFill(entry), weight );
}


void Efficiency::fill_denominator_sideband(const double entry, const double weight){
    sidebandWasSubtracted = false;
    denominator_sideband->Fill( entryToFill(entry), weight );
}


void Efficiency::fill( const double entry, const double weight, const bool isNumerator, const bool isSideband){
    
    //check if sideband is initialized before it can be used 
    if( ( !hasSideband() ) && isSideband ){
        std::cerr << "Error in Efficiency::fill : trying to fill sideband for Efficiency object that has no sideband! Returning control." << std::endl;
        return;
    }

    if( isNumerator ){
        if( isSideband ){
            fill_numerator_sideband(entry, weight);
        } else {
            fill_numerator(entry, weight);
        }
    } else {
        if( isSideband ){
            fill_denominator_sideband(entry, weight);
        } else {
            fill_denominator(entry, weight);
        }
    }
}


void Efficiency::subtractSideband(){
    if( !hasSideband() ){
        return;
    }
    if( !sidebandWasSubtracted ){
        numerator->Add( numerator_sideband.get(), -1. );
        numerator_sideband->Reset();
        denominator->Add( denominator_sideband.get(), -1.);
        denominator_sideband->Reset();
        sidebandWasSubtracted = true;
    }
}


std::shared_ptr<TH1D> Efficiency::getNumerator(){
    if( !sidebandWasSubtracted ){
        subtractSideband();
    }
    return std::shared_ptr<TH1D>( (TH1D*) numerator->Clone() );
} 

std::shared_ptr<TH1D> Efficiency::getDenominator(){
    if( !sidebandWasSubtracted ){
        subtractSideband();
    } 
    return std::shared_ptr<TH1D>( (TH1D*) denominator->Clone() );
}





