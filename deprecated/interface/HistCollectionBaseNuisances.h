/*
version of HistCollectionBase class that also includes nuistances
*/
#ifndef HistCollectionBaseNuisances_H
#define HistCollectionBaseNuisances_H

//include other parts of code 
#include "HistCollectionBase.h"

//include c++ library classes

//include ROOT classes 

class HistCollectionBaseNuisances : public HistCollectionBase{
    
    public:
        HistCollectionBaseNuisances(const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector < std::string >&, const bool includeSB = false);
        HistCollectionBaseNuisances(const std::string&, const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector < std::string >&, const bool includeSB = false);

        std::shared_ptr<TH1D> accessNuisanceUp(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb) const;
        std::shared_ptr<TH1D> accessNuisanceDown(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb) const;
        std::shared_ptr<TH1D> accessNuisanceUp(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb) const;
        std::shared_ptr<TH1D> accessNuisanceDown(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb) const;

        void negBinsToZero() const;
        size_t numberOfNuisances() const;

        HistCollectionBaseNuisances& operator+=(const HistCollectionBaseNuisances&);

   private:
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceCollectionUp;
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceCollectionDown;
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceSideBandUp;
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceSideBandDown;

        void initNuisanceVectors(const std::vector < std::string >&); 
};


#endif
