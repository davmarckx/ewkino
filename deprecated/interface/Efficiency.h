/*
Class containing a numerator and denominator of events, and a potential sideband to compute an efficiency
*/

#ifndef Efficiency_H
#define Efficiency_H

//include c++ library classes 
#include <string>
#include <memory>

//include ROOT classes 
#include "TH1D.h"
#include "TGraphAsymmErrors.h"

//include other parts of code 
#include "HistInfo.h"


class Efficiency{
    
    public: 
        Efficiency( const std::string& name, const HistInfo&, const bool sideband = false);

        //std::shared_ptr<TGraphAsymmErrors> efficiencyGraph();
        //std::shared_ptr< TH1D > efficiencyHist();
        
        std::shared_ptr< TH1D > getNumerator();
        std::shared_ptr< TH1D > getDenominator();

        void fill(const double entry, const double weight, const bool isNumerator, const bool isSideband = false);

        void fill_numerator(const double entry, const double weight) const;
        void fill_denominator(const double entry, const double weight) const;
        void fill_numerator_sideband(const double entry, const double weight);
        void fill_denominator_sideband(const double entry, const double weight);

    private:
        std::shared_ptr<TH1D> numerator; 
        std::shared_ptr<TH1D> denominator; 

        std::shared_ptr<TH1D> numerator_sideband;
        std::shared_ptr<TH1D> denominator_sideband;

        const HistInfo* histInfo;

        //bool isFilled; 
        bool sidebandWasSubtracted;

        void subtractSideband();        
 
        double entryToFill( const double entry ) const{
            return std::min(entry, histInfo->maxBinCenter() );
        }

        bool hasSideband() const{
            return ( numerator_sideband.use_count() > 0 );
        }
};


#endif
