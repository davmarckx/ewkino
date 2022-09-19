/*
Class that collects a collection of histograms for one kinematic distribution of one sample. The class uses a Category object to build a histogram for every category.
*/

#ifndef HistCollectionBase_H
#define HistCollectionBase_H

//include c++ library classes
#include <vector>
#include <memory>

//include other parts of code 
#include "Category.h"
#include "HistInfo.h"
#include "Sample.h"

//include ROOT classes
#include "TH1D.h"

class HistCollectionBase{
    public:

        //direct construction
        HistCollectionBase(const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const bool includeSB = false);

        //construction by reading info from file
        HistCollectionBase(const std::string&, const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const bool includeSB = false);

        //access routines
        std::shared_ptr< TH1D > access(const size_t categoryIndex, const bool sb = false) const;  //used 
        std::shared_ptr< TH1D > access(const std::vector<size_t>& categoryIndices, const bool sb = false) const; //used 
        
        std::string infoName() const { return histInfo->name(); }
        std::string sampleUniqueName() const { return sample->getUniqueName(); }    
        std::string sampleProcessName() const { return sample->getProcessName(); }  //used 
        std::string categoryName(size_t categoryIndex) const { return category->name(categoryIndex); }
        std::string categoryName(const std::vector<size_t>& categoryIndices) const { return category->name(categoryIndices); }

        //full name of every histogram
        std::string name(size_t categoryIndex, const bool sideband = false) const{      //used privately
            return infoName() + categoryName(categoryIndex) + (sideband ? "sideband" : "") + sampleUniqueName(); 
        }
        size_t size() const{ return collection.size(); }

        //add two histcollections
        HistCollectionBase& operator+=(const HistCollectionBase& rhs); //used 

        //set negative bin contents to 0
        void negativeBinsToZero() const;    //used 

        //rebin all histograms in category with certain value
        void rebin(const std::string&, const int) const;

        //check if collection contains sideband
        bool hasSideBand() const { return ! (sideBand.empty()); }

        //check if this is a data collection
        bool isData() const { return sample->isData(); }

        //check if this is a SM signal
        bool isSMSignal() const { return sample->isSMSignal(); }

        //check if this a new physics signal
        bool isNewPhysicsSignal() const { return sample->isNewPhysicsSignal(); }

        //range for every categorization
        size_t categoryRange(const size_t c) const { return category->getRange(c); } //used 

    //protected:
        size_t getCategoryIndex(const std::vector<size_t>& categoryIndices) const { return category->getIndex(categoryIndices); } //used 

    private:
        std::shared_ptr< HistInfo > histInfo;
        std::shared_ptr< Sample > sample;
        std::shared_ptr< Category > category;
        std::vector< std::shared_ptr < TH1D > > collection;
        std::vector< std::shared_ptr < TH1D > > sideBand;
       
        //rebin all histograms of given category index
        void rebin(const size_t, const int) const;
};
#endif
