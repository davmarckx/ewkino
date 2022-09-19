#include "../interface/HistCollection.h"

//include c++ library classes
#include <iostream>
#include <set>
#include <fstream>
#include <algorithm>

//include ROOT classes
#include "TROOT.h"

#include <chrono>

HistCollectionSample::HistCollectionSample(std::shared_ptr< std::vector< HistInfo> > infoList, std::shared_ptr<Sample> sam, std::shared_ptr< Category > categorization, bool includeSB):
    histInfo(infoList), sample(sam), cat(categorization) {
    size_t counter = 0;
    for(auto infoIt = infoList->cbegin(); infoIt != infoList->cend(); ++infoIt){
        collection.push_back(std::vector< std::shared_ptr<TH1D> >() );
        if(includeSB) sideBand.push_back(std::vector< std::shared_ptr<TH1D> >() );
        for(auto catIt = cat->cbegin(); catIt != cat->cend(); ++catIt){
            collection[counter].push_back(infoIt->makeHist(*catIt + sample->getFileName() ) );
            if(includeSB) sideBand[counter].push_back(infoIt->makeHist(*catIt + sample->getFileName() + "_sideband") );  
        }
        ++counter;
    }
}

HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, std::shared_ptr<Sample> sam, std::shared_ptr< Category > categorization, bool includeSB):
    HistCollectionSample(std::make_shared< std::vector< HistInfo > >(infoList), sam, categorization, includeSB) {}

HistCollectionSample::HistCollectionSample(std::shared_ptr< std::vector<HistInfo> > infoList, std::shared_ptr<Sample> sam, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollectionSample(infoList, sam, std::make_shared<Category>(categorization), includeSB) {}

HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, std::shared_ptr<Sample> sam, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollectionSample(std::make_shared< std::vector< HistInfo > >(infoList), sam, std::make_shared<Category>(categorization), includeSB) {}


HistCollectionSample::HistCollectionSample(const std::string& fileListName, std::shared_ptr< std::vector < HistInfo> > infoList, std::shared_ptr<Sample> sam, std::shared_ptr< Category > categorization, bool includeSB):
    histInfo(infoList), cat(categorization), sample(sam)
{
    //make list of files in directory
    //std::system("touch inputList.txt");
    //std::system( ("for f in " + dir + "/*; do echo $f >> inputList.txt; done").c_str());
    //read root file list from txt
    //std::ifstream inputList("inputList.txt");
    std::ifstream inputList(fileListName);
    std::set< std::string > fileList;
    std::string temp;
    //fill set with list of files
    while(std::getline(inputList, temp)){
        //find correct sample and push back those files
        if(temp.find(sample->getFileName()) != std::string::npos) fileList.insert(temp);
    }
    inputList.close();
    //clean up temporary file
    //std::system("rm inputList.txt");
    //read histogram collection from files
    collection = std::vector< std::vector< std::shared_ptr< TH1D > > >(histInfo->size());
    for(unsigned infoInd = 0; infoInd < histInfo->size(); ++infoInd){
        collection[infoInd] = std::vector< std::shared_ptr< TH1D > >(cat->size());
        for(auto it = fileList.cbegin(); it != fileList.cend(); ++it){
            TFile* tempFile = TFile::Open((const TString&) *it);
            for(unsigned c = 0; c < cat->size(); ++c){
                if(it == fileList.cbegin()){
                    collection[infoInd][c].reset( (TH1D*) tempFile->Get((const TString&) name(infoInd, c) ) );
                    collection[infoInd][c]->SetDirectory(gROOT);
                } else{
                    collection[infoInd][c]->Add( (TH1D*) tempFile->Get((const TString&) name(infoInd, c) ) );
                }
            }
            tempFile->Close();
        }
    }
}

std::shared_ptr<TH1D> HistCollectionSample::access(size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    size_t catIndex = cat->getIndex(catIndices);
    if(!sb){
        return collection[infoIndex][catIndex];
    } else{
        if(sideBand.empty()){
            std::cerr << "empty sideband, returning index 0" << std::endl;
            return 0;
        }
        return sideBand[infoIndex][catIndex];
    }
}


std::string HistCollectionSample::name(size_t infoIndex, size_t catIndex, bool sb) const{
    return (*histInfo)[infoIndex].name() + (*cat)[catIndex] + sample->getFileName() + (sb ? "sideband" : "");
}

std::string HistCollectionSample::name(size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    size_t catIndex = cat->getIndex(catIndices);
    return name(infoIndex, catIndex, sb);
}

void HistCollectionSample::setNegZero(){
    //only set negative bins in collection to 0, sideband can be negative
    for(auto dIt = collection.begin(); dIt != collection.cend(); ++dIt){
        for(auto cIt = dIt->cbegin(); cIt != dIt->cend(); ++cIt){
            for(unsigned b = 1; b < (*cIt)->GetNbinsX() + 1; ++b){
                if((*cIt)->GetBinContent(b) < 0.) (*cIt)->SetBinContent(b, 0.);
            }
        }
    }
}

void HistCollectionSample::store(const std::string& dir, const long unsigned begin, const long unsigned end) const{
    std::string extra("");
    if(begin != 0 || end != 0){
        extra = "_" + std::to_string(begin) + "_" + std::to_string(end);
    }
    //create new root file
    TFile* outFile = TFile::Open((const TString&) dir + "/tempHist_" + sample->getFileName() + extra, "RECREATE"); 
    for(auto& vec : collection){
        for(auto& histP : vec){
            histP->Write();
        }
    }
    outFile->Close();
}
/*
void HistCollectionSample::read(const std::string& dir){
    auto start = std::chrono::high_resolution_clock::now();
    //make list of files in directory
    std::system("touch inputList.txt");
    std::system( ("for f in " + dir + "/*; do echo $f >> inputList.txt; done").c_str());
    //CHECK, remove later
    std::system("cp inputList.txt check.txt");
    ///////
    std::ifstream inputList("inputList.txt");
    std::set< std::string > fileList;
    std::string temp;
    //fill set with list of files
    while(std::getline(inputList, temp)){
        //find correct sample and push back those files
        if(temp.find(sample->getFileName()) != std::string::npos) fileList.insert(temp);
    }
    std::system("rm inputList.txt");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "setting up file list took : " << elapsed.count() << " s" << std::endl;
    //reset and refill all current histograms histograms
    for(unsigned infoInd = 0; infoInd < collection.size(); ++infoInd){
        for(unsigned c = 0; c < cat->size(); ++c){
            start = std::chrono::high_resolution_clock::now();
            //collection[infoInd][c];
            for(auto it = fileList.cbegin(); it != fileList.cend(); ++it){
                //TFile* tempFile = TFile::Open((const TString&) dir + "/" + *it);
                TFile* tempFile = TFile::Open((const TString&) *it);
                if(it == fileList.cbegin()){
                    collection[infoInd][c].reset((TH1D*) tempFile->Get((const TString&) name(infoInd, c) ) );
                    collection[infoInd][c]->SetDirectory(gROOT);
                } else{
                    collection[infoInd][c]->Add( (TH1D*) tempFile->Get((const TString&) name(infoInd, c) ) );
                }
                tempFile->Close();
            }
            end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
            std::cout << "reading one file lasts: " << elapsed.count() << " s" << std::endl;
        }
    }
}
*/

HistCollectionSample& HistCollectionSample::operator+=(const HistCollectionSample& rhs){
    if(collection.size() != rhs.collection.size() || cat->size() != rhs.cat->size()){
        std::cerr << "HistCollection of incompatible dimensions can not be added: returning left hand side!" << std::endl;
    } else{
        for(size_t dist = 0; dist < collection.size(); ++dist){
            for(size_t c = 0; c < collection[dist].size(); ++c){
                collection[dist][c]->Add(rhs.collection[dist][c].get());
                if(!sideBand.empty()){
                    if(rhs.sideBand.empty()) std::cerr << "Right-hand side has no sideband, can not add sidebands!" << std::endl;
                    else sideBand[dist][c]->Add(rhs.sideBand[dist][c].get());
                }
            }
        }
    }
    return *this;
};

HistCollectionSample operator+(const HistCollectionSample& lhs, const HistCollectionSample& rhs){
    HistCollectionSample ret(lhs);
    ret += rhs;
    return ret;
}

HistCollection::HistCollection(std::shared_ptr< std::vector < HistInfo > > infoList, const std::vector<Sample>& samList, std::shared_ptr< Category > categorization, bool includeSB){
    for(auto samIt = samList.cbegin(); samIt != samList.cend(); ++samIt){
        fullCollection.push_back(HistCollectionSample(infoList, std::make_shared<Sample>(Sample(*samIt) ), categorization, includeSB) );
    }
}

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector<Sample>& samList, std::shared_ptr< Category > categorization, bool includeSB):
    HistCollection(std::make_shared< std::vector < HistInfo > >(infoList), samList, categorization, includeSB) {}

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector<Sample>& samList, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollection(infoList, samList, std::make_shared<Category>(Category(categorization)), includeSB) {}

HistCollection::HistCollection(const std::string& dir, std::shared_ptr< std::vector < HistInfo > > infoList, const std::vector<Sample>& samList, std::shared_ptr< Category > categorization, bool includeSB){
    for(auto samIt = samList.cbegin(); samIt != samList.cend(); ++samIt){
        fullCollection.push_back(HistCollectionSample(dir, infoList, std::make_shared<Sample>(Sample(*samIt) ), categorization, includeSB) );
    }
}

HistCollection::HistCollection(const std::string& dir, const std::vector<HistInfo>& infoList, const std::vector<Sample>& samList, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollection(dir, std::make_shared< std::vector < HistInfo > > (infoList), samList, std::make_shared<Category>(Category(categorization)), includeSB) {}

std::shared_ptr<TH1D> HistCollection::access(size_t samIndex, size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    return fullCollection[samIndex].access(infoIndex, catIndices, sb);
}

std::string HistCollection::name(size_t samIndex, size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    return fullCollection[samIndex].name(infoIndex, catIndices, sb);
}

void HistCollection::setNegZero(){
    for(HistCollectionSample& samCol : fullCollection){
        samCol.setNegZero();
    }
}

HistCollection HistCollection::mergeProcesses(){
    setNegZero();
    HistCollection tempCol;
    std::set<std::string> usedProcesses;
    for(auto it = fullCollection.cbegin(); it != fullCollection.cend(); ++it){
        if(usedProcesses.find(it->sample->getProc()) == usedProcesses.end()){
            usedProcesses.insert(it->sample->getProc());
            HistCollectionSample tempSam = *it;
            for(auto jt = it + 1; jt != fullCollection.cend(); ++jt){
                if(it->sample->getProc() == jt->sample->getProc()){
                    tempSam += *jt;
                }
            }
            tempCol.fullCollection.push_back(tempSam);
        }
    }
    //*this = tempCol;
    return tempCol;
}

Plot HistCollection::getPlot(size_t infoIndex, size_t catIndex) const{
    std::shared_ptr<TH1D> obs;
    std::map< std::string, std::shared_ptr<TH1D> > bkg;
    for(auto& samCol : fullCollection){
        if(samCol.isData()){
            if(obs.use_count() == 0) obs = samCol.access(infoIndex, catIndex);
            else obs->Add( samCol.access(infoIndex, catIndex).get() );
        }
        else bkg[samCol.procName()] = samCol.access(infoIndex, catIndex);
    }
    std::cout << fullCollection[0].infoName(infoIndex) + "_" + fullCollection[1].catName(catIndex) << std::endl;
    //make version of filename with every underscore becoming a subdirectory
    std::string directoryName = "ewkino/dilepCR/" + fullCollection[1].catName(catIndex);
    std::replace(directoryName.begin(), directoryName.end(), '_', '/');
    //////////////////
    return Plot(directoryName + "/" + fullCollection[0].infoName(infoIndex) + "_" + fullCollection[1].catName(catIndex), obs, bkg);
}

Plot HistCollection::getPlot(size_t infoIndex, const std::vector<size_t>& catIndices) const{
    return getPlot(infoIndex, getIndex(catIndices));
}

/*
void HistCollection::read(const std::string& dir){
    for(auto& samCol : fullCollection){
        samCol.read(dir);
    }
}
*/
