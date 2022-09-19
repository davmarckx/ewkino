/*
   class implementation of HistCollectionBaseNuisances
 */

#include "../interface/HistCollectionBaseNuisances.h"

//include ROOT classes
#include "TROOT.h"


HistCollectionBaseNuisances::HistCollectionBaseNuisances(const std::shared_ptr<HistInfo>& info, const std::shared_ptr<Sample>& sam, const std::shared_ptr<Category>& cat, 
    const std::vector < std::string >& nuisanceNames, const bool includeSB): HistCollectionBase(info, sam, cat, includeSB)
{
    initNuisanceVectors(nuisanceNames);
    for(size_t n = 0; n < nuisanceNames.size(); ++n){
        for(unsigned c = 0; c < size(); ++c){
            nuisanceCollectionDown[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_" + nuisanceNames[n] + "_Down");
            nuisanceCollectionUp[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_" + nuisanceNames[n] + "_Up");
            if(hasSideBand()){   
                nuisanceSideBandDown[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_sideband_" + nuisanceNames[n] + "_Down");
                nuisanceSideBandUp[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_sideband_" + nuisanceNames[n] + "_Up");
            }
        }
    }
}

HistCollectionBaseNuisances::HistCollectionBaseNuisances(const std::string& inputFileName, const std::shared_ptr< HistInfo>& info, const std::shared_ptr<Sample>& sam,
    const std::shared_ptr< Category >& cat, const std::vector < std::string >& nuisanceNames, const bool includeSB): HistCollectionBase(inputFileName, info, sam, cat, includeSB)
{
    //Open root file to read
    TFile* inputFile = TFile::Open( (const TString&) inputFileName, "READ");
    //read histpgram for every nuisance
    initNuisanceVectors(nuisanceNames); 
    for(size_t n = 0; n < nuisanceNames.size(); ++n){
        //read histogram for every category
        for(size_t c = 0; c < size(); ++c){
            nuisanceCollectionDown[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c) + "_" + nuisanceNames[n] + "_Down") );
            //take away ownership from inputFile so root does not delete histogram
            nuisanceCollectionDown[n][c]->SetDirectory(gROOT);
            nuisanceCollectionUp[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c) + "_" + nuisanceNames[n] + "_Up") );
            nuisanceCollectionUp[n][c]->SetDirectory(gROOT);
            if(includeSB){
                nuisanceSideBandDown[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c, true)  + "_" + nuisanceNames[n] + "_Down") );
                nuisanceSideBandDown[n][c]->SetDirectory(gROOT);
                nuisanceSideBandUp[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c, true)  + "_" + nuisanceNames[n] + "_Up") );
                nuisanceSideBandUp[n][c]->SetDirectory(gROOT);
            }
        }
        inputFile->Close(); 
    }
}

void HistCollectionBaseNuisances::initNuisanceVectors(const std::vector < std::string >& nuisanceNames){ 
    for(size_t n = 0; n < nuisanceNames.size(); ++n){
        nuisanceCollectionDown.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
        nuisanceCollectionUp.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
        if(hasSideBand()){
            nuisanceSideBandDown.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
            nuisanceSideBandUp.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
        }
    }
}

size_t HistCollectionBaseNuisances::numberOfNuisances() const{
    if(nuisanceCollectionUp.size() != nuisanceCollectionDown.size() ){
        std::cerr << "Error: HistCollectionBaseNuisance object with different number of Up and Down nuisance histograms" << std::endl;
        return 0;
    } else{
        return nuisanceCollectionDown.size();
    }
}

std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisanceDown(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb) const{
    if(!sb){
        return nuisanceCollectionDown[nuisanceIndex][categoryIndex];
    } else{
        return nuisanceSideBandDown[nuisanceIndex][categoryIndex];
    }
}

std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisanceUp(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb) const{
    if(!sb){
        return nuisanceCollectionUp[nuisanceIndex][categoryIndex];
    } else{
        return nuisanceSideBandUp[nuisanceIndex][categoryIndex];
    }
}

std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisanceDown(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb) const{
    size_t categoryIndex = getCategoryIndex(categoryIndices);
    return accessNuisanceDown(nuisanceIndex, categoryIndex, sb);
}

std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisanceUp(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb) const{
    size_t categoryIndex = getCategoryIndex(categoryIndices);
    return accessNuisanceUp(nuisanceIndex, categoryIndex, sb);
}

HistCollectionBaseNuisances& HistCollectionBaseNuisances::operator+=(const HistCollectionBaseNuisances& rhs){
    //add base class parts of the objects 
    HistCollectionBase::operator+=(rhs);
    
    //make sure to add compatible histograms 
    if(numberOfNuisances() != rhs.numberOfNuisances()){
        std::cerr << "Error: attempting to add histCollections with different amounts of nuisances: returning *this" << std::endl;
    }
    
    //TO-DO: somehow check names of nuisances here? This requires storage of the nuisance names in this class

    //add all nuisance histograms
    for(size_t n = 0; n < numberOfNuisances(); ++n){
        for(size_t h = 0; h < size(); ++h){
            nuisanceCollectionDown[n][h]->Add( (rhs.nuisanceCollectionDown[n][h]).get() );
            nuisanceCollectionUp[n][h]->Add( (rhs.nuisanceCollectionUp[n][h]).get() );
            if(hasSideBand()){
                nuisanceSideBandDown[n][h]->Add( (rhs.nuisanceSideBandDown[n][h]).get() );
                nuisanceSideBandUp[n][h]->Add( (rhs.nuisanceSideBandUp[n][h]).get() );
            }
        }
    }
    return *this;
}

void HistCollectionBaseNuisances::negBinsToZero() const{
    /*
    Note that only mainband collection is affected here since sideband is allowed to remain negative
    */
    HistCollectionBase::negBinsToZero();
    for(unsigned n = 0; n < numberOfNuisances(); ++n){
        for(unsigned c = 0; c < size(); ++c){
            setNegativeBinsToZero(nuisanceCollectionDown[n][c]);
            setNegativeBinsToZero(nuisanceCollectionUp[n][c]);
        }
    }
}
