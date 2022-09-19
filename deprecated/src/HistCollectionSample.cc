#include "../interface/HistCollectionSample.h"

HistCollectionSample::HistCollectionSample(const std::vector< std::shared_ptr< HistInfo > >& infoList, const std::shared_ptr<Sample>& sam, const std::shared_ptr<Category>& cat, const bool includeSB){
    for(const auto& infoPtr : infoList){
        collection.push_back( HistCollectionBase( infoPtr, sam, cat, includeSB ) );
    }
}

HistCollectionSample::HistCollectionSample(const std::vector< HistInfo>& infoList, const Sample& sam, const Category& cat, const bool includeSB){
    std::shared_ptr< Category > categoryPointer = std::make_shared<Category>(cat);
    std::shared_ptr< Sample > samplePointer = std::make_shared<Sample>(sam);
    std::vector < std::shared_ptr< HistInfo > > infoPointerList;
    for(const auto& info: infoList){
        infoPointerList.push_back(std::make_shared< HistInfo >(info) );
    }
    *this = HistCollectionSample(infoPointerList, samplePointer, categoryPointer, includeSB);
}

HistCollectionSample::HistCollectionSample(const std::vector< HistInfo>& infoList, const Sample& sam, const std::vector< std::vector < std::string > >& categoryVec, const bool includeSB):
    HistCollectionSample(infoList, sam, Category(categoryVec), includeSB) {} 

void HistCollectionSample::store(const std::string& directory, const long unsigned begin, const long unsigned end) const{

    //extra string indicating event numbers when sample is split in multiple jobs
    std::string extra("");
    if(begin != 0 || end != 0){
        extra = std::to_string(begin) + "_" + std::to_string(end);
    }

    //create new root file
    //Warning: unique sample name MUST come EXACTLY before .root in fileName in order to make sure the same sample is uniquely read when used for both 2016 and 2017 data.
    TFile* outputFile = TFile::Open( (directory + "/tempHist_" + extra + "_" + sampleUniqueName() + ".root").c_str(), "RECREATE");
    for(size_t dist = 0; dist < collection.size(); ++dist){

        //ROOT will automatically create random directories when too many histograms are filled, breaking the code
        //to avoid this, create a separate directory for every variable here 
        outputFile->mkdir( collection[dist].infoName().c_str() );
        outputFile->cd( collection[dist].infoName().c_str() );
        for(size_t c = 0; c < collection[dist].size(); ++c){
            access(dist, c, false)->Write();
            if( hasSideBand() ){
                access(dist, c, true)->Write();
            }
        }

        //move back to the top
        outputFile->cd();
    }
    outputFile->Close();
}
