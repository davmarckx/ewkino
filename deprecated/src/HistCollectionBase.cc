#include "../interface/HistCollectionBase.h"

//include other parts of code 
#include "../interface/analysisTools.h"

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"


HistCollectionBase::HistCollectionBase(const std::shared_ptr< HistInfo >& info, const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const bool includeSB):
    histInfo(info), sample(sam), category(cat)
{
    //make histogram for every category
    for(size_t c = 0; c < category->size(); ++c){
        collection.push_back( info->makeHist( categoryName(c) + sampleUniqueName() ) );
        collection.back()->Sumw2();
        if(includeSB){
            sideBand.push_back(  info->makeHist( categoryName(c) + sampleUniqueName() + "_sideband") );
            sideBand.back()->Sumw2();
        }
    }
}


HistCollectionBase::HistCollectionBase(const std::string& inputFileName, const std::shared_ptr< HistInfo >& info, const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const bool includeSB):
    histInfo(info), sample(sam), category(cat)
{
    //Open root file to read
    TFile* inputFile = TFile::Open( inputFileName.c_str(), "READ");

    //change to the correct directory (named after the distribution)
    inputFile->cd( (const TString&) infoName() );

    //read correct histogram for every category
    for(size_t c = 0; c < category->size(); ++c){
        collection.push_back(std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (infoName() + "/" + name(c)).c_str() ) ) );

        //take away ownership from inputFile so root does not delete histogram
        collection.back()->SetDirectory(gROOT);
        if(includeSB){
            sideBand.push_back(std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (infoName() + "/" + name(c, true)).c_str() ) ) );
            sideBand.back()->SetDirectory(gROOT);
        }
    }
    inputFile->Close();
}


std::shared_ptr< TH1D > HistCollectionBase::access(const size_t categoryIndex, const bool sb) const{
    if(!sb){
        return collection[categoryIndex];
    } else{
        return sideBand[categoryIndex];
    }
}


std::shared_ptr< TH1D > HistCollectionBase::access(const std::vector<size_t>& categoryIndices, const bool sb) const{
    size_t categoryIndex = category->getIndex(categoryIndices);
    return access(categoryIndex, sb);
}


HistCollectionBase& HistCollectionBase::operator+=(const HistCollectionBase& rhs){
    if(size() != rhs.size()){
        std::cerr << "Error: attempting to add histCollections of different size: returning *this" << std::endl;
        return *this;
    }
    if(infoName() != rhs.infoName() || sampleProcessName() != rhs.sampleProcessName() ){
        std::cerr << "Error: attempting to add histCollections of different distribution or process: returning *this" << std::endl;
        std::cerr << "infoName() = " << infoName() << "\t" << "sampleProcessName() = " << sampleProcessName() << std::endl;
        std::cerr << "rhs.infoName() = " << rhs.infoName() << "\t" << "rhs.sampleProcessName() = " << rhs.sampleProcessName() << std::endl;
        return *this;
    }
    if(hasSideBand() && !rhs.hasSideBand() ){
        std::cerr << "Error: attempting to add histcollection with sideband to one without sideband: returning *this" << std::endl;
        return *this;
    }
    for(size_t h = 0; h < size(); ++h){
        collection[h]->Add( (rhs.collection[h]).get() );
        if(hasSideBand()){
            sideBand[h]->Add( (rhs.sideBand[h]).get() );
        }
    }
    return *this;
}


void HistCollectionBase::negativeBinsToZero() const{

    /*
    Note that only collection is affected here since sideband is allowed to remain negative
    */
    //loop over all histograms in collection 
    //for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){ 
    for(auto& hist : collection){
        analysisTools::setNegativeBinsToZero(hist); 
    }
}


//rebin histograms corresponding to given categoryIndex
void HistCollectionBase::rebin(const size_t categoryIndex, const int numberOfBinsToMerge) const{
    collection[categoryIndex]->Rebin(numberOfBinsToMerge);
}


//rebin histograms for all categories whose names contain given string
void HistCollectionBase::rebin(const std::string& categoryName, const int numberOfBinsToMerge) const{
    std::vector<size_t> matchingCategories = category->findCategoriesByName(categoryName);
    for(auto& c : matchingCategories){
        rebin(c, numberOfBinsToMerge);
    }
}
