/*
Class that collects the his 
*/

#ifndef HistCollectionSample_H
#define HistCollectionSample_H

//include c++ library classes
#include <fstream>

//include other parts of code
#include "HistCollectionBase.h"

//include ROOT classes

class HistCollectionSample{
    public:

        //direct initialization:
        HistCollectionSample(const std::vector< std::shared_ptr< HistInfo > >&, const std::shared_ptr< Sample >&, const std::shared_ptr< Category >&, const bool includeSB = false);
        HistCollectionSample(const std::vector< HistInfo>&, const Sample&, const Category&, const bool includeSB = false);
        HistCollectionSample(const std::vector< HistInfo>&, const Sample&, const std::vector< std::vector < std::string > >& categoryVec = std::vector< std::vector < std::string > >(), const bool includeSB = false);

        //access histograms
        std::shared_ptr< TH1D > access(const size_t infoIndex, const std::vector<size_t>& categoryIndices, const bool sb = false) const{ 
            return collection[infoIndex].access(categoryIndices, sb);
        }

        //write histograms to ROOT files in given directory, begin and end indicate the event numbers stored in this file
        void store(const std::string&, const long unsigned begin = 0, const long unsigned end = 0) const;

        //get range of each category
        size_t categoryRange(const size_t c) const { return collection.front().categoryRange(c); }

    private:
        std::vector < HistCollectionBase > collection;
        std::string sampleUniqueName() const{
            return collection.front().sampleUniqueName();
        }

        //access histograms with direct category index
        std::shared_ptr< TH1D > access(const size_t infoIndex, const size_t categoryIndex, const bool sb = false) const{
            return collection[infoIndex].access(categoryIndex, sb);
        }

        bool hasSideBand() const{
            return collection.front().hasSideBand();
        }
};
#endif
