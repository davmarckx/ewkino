/*
Class representing an efficiency and uncertainties on this efficiency. The uncertainties are stored as varied Efficiency objects.
*/
#ifndef EfficiencyUnc_H
#define EfficiencyUnc_H

//include c++ standard library classes 
#include <map>

//include ROOT classes 

//include other parts of code 
#include "Efficiency.h"

class EfficiencyUnc : public Efficiency{

    public:
        EfficiencyUnc(const std::string& name, const HistInfo& info, const std::vector<std::string>& uncNames, const bool sideband = false);

        void fillVariationDown(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband = false);
        void fillVariationUp(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband = false);

        std::shared_ptr< TH1D > getNumeratorUnc();
        std::shared_ptr< TH1D > getDenominatorUnc();

    private:
        std::map< std::string, Efficiency > efficiencyVariationsDown;
        std::map< std::string, Efficiency > efficiencyVariationsUp;

        void fillVariation(std::map< std::string, Efficiency>&, const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband = false);
        std::shared_ptr<TH1D> getUncertainty( std::shared_ptr<TH1D> (Efficiency::*getHistogram)() );

        std::shared_ptr<TH1D> getNumeratorDown(const std::string&);
        std::shared_ptr<TH1D> getNumeratorUp(const std::string&);

        std::map< std::string, Efficiency >::iterator findEntry( std::map< std::string, Efficiency>&, const std::string& );
        std::map< std::string, Efficiency >::iterator findEntryDown( const std::string& );
        std::map< std::string, Efficiency >::iterator findEntryUp( const std::string& );
};

#endif
