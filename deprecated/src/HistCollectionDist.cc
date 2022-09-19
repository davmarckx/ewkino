#include "../interface/HistCollectionDist.h"

//include c++ library classes
#include <fstream>
#include <set>
#include <algorithm>

//include other parts of code 
#include "../interface/systemTools.h"
#include "../interface/stringTools.h"
#include "../interface/analysisTools.h"

HistCollectionDist::HistCollectionDist( const std::string& directory, const std::shared_ptr < HistInfo >& histInfo, const std::vector < std::shared_ptr < Sample > >& samples, 
    const std::shared_ptr < Category >& category, const bool sideBand )
{
    //retrieve list of all root files in directory 
    std::vector< std::string > fileNameList = systemTools::listFiles( directory, "", ".root" );

    for(const auto& sampPtr : samples ){

        //make new histogram collection for first file in sample
        bool firstFile= true; 
        
        for(const auto& file : fileNameList){
        
            //check whether file corresponds to sample 
            //WARNING: the file names are assumed here to have the Sample's uniqueName DIRECTLY followed by .root
            //the naming convntion should automatically be taken into account in files written by the HistCollectionSample class
            if( stringTools::stringContains( file, sampPtr->getUniqueName() + ".root") ){

                //make new collection for first file in sample
                if( firstFile ){
                    collection.push_back( HistCollectionBase(file, histInfo, sampPtr, category, sideBand) );
                    firstFile = false;

                //otherwise add the file contents to the existing collection
                } else {
                    collection.back() += HistCollectionBase(file, histInfo, sampPtr, category, sideBand);
                }
            }
        }

    }
}


HistCollectionDist::HistCollectionDist( const std::string& directory, const HistInfo& histInfo, const std::vector< Sample >& samples, const Category& category, const bool sideBand ){
    std::shared_ptr<HistInfo> infoPointer = std::make_shared<HistInfo>(histInfo);
    std::shared_ptr<Category> categoryPointer = std::make_shared<Category>(category);
    std::vector < std::shared_ptr< Sample > > samplePointerList;
    for(const auto& sam: samples){
        samplePointerList.push_back(std::make_shared< Sample >( sam ) );        
    }
    *this = HistCollectionDist( directory, infoPointer, samplePointerList, categoryPointer, sideBand);
}

HistCollectionDist::HistCollectionDist( const std::string& directory, const HistInfo& histInfo, const std::vector< Sample >& samples, const std::vector< std::vector < std::string > >& categoryVec, const bool sideBand ): 
    HistCollectionDist(directory, histInfo, samples, Category(categoryVec) , sideBand) {}

/*
std::vector<std::string> HistCollectionDist::getFileNames(const std::string& fileName){
    std::vector<std::string> fileList;
    std::ifstream listStream(fileName);
    std::string line;
    while(std::getline(listStream, line)){
        if(!line.empty() && line.find(".root") != std::string::npos){
            fileList.push_back(line);
        }
    }
    listStream.close();
    return fileList;
}
*/

void HistCollectionDist::negativeBinsToZero() const{
    for(const auto& baseCollection : collection){
        baseCollection.negativeBinsToZero();
    }
}

void HistCollectionDist::mergeProcesses(){

    //make sure to only megre once for efficiency
    if( merged ){
        return;
    }

    //set negative bins to 0 before merging
    negativeBinsToZero();

    //new merged collection
    std::vector< HistCollectionBase > mergedCollection;

    //keep track of processes that have been used
    std::set< std::string > usedProcesses;

    for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){

        //check if this process is used 
        if( usedProcesses.find( colIt->sampleProcessName() ) != usedProcesses.end() ) continue;

        //add process to merged collection
        mergedCollection.push_back(*colIt);
        usedProcesses.insert(colIt->sampleProcessName());

        //merge all collections containing this process
        for(auto mergeIt = colIt + 1; mergeIt != collection.cend(); ++mergeIt){

            //check if this entry corresponds to the same process
            if(mergeIt->sampleProcessName() == colIt->sampleProcessName()){
                mergedCollection.back() += (*mergeIt);
            }
        }
    }

    //set old collection equal to new collection
    collection = mergedCollection; 

    //flag collection as merged
    merged = true;
}

//EVALUATE NECESSITY
std::string HistCollectionDist::name(const size_t categoryIndex) const{
    return distributionName() + "_" + categoryName(categoryIndex);
}

//EVALUATE NECESSITY
std::string HistCollectionDist::plotPath(const size_t categoryIndex) const{
    std::string path = categoryName(categoryIndex);
    std::replace(path.begin(), path.end(), '_', '/');
    if(path.back() != '/') path.append("/");
    return path;
}

std::shared_ptr<TH1D> HistCollectionDist::getTotalSideBand(const size_t categoryIndex) const{
    if(!hasSideBand()){
        std::cerr<< "Error: requesting totalSideBand for collection without sideband: returning nullptr" << std::endl;
        return std::shared_ptr<TH1D>(nullptr);
    }

    std::shared_ptr<TH1D> totalSideBand = collection.front().access(categoryIndex);
    for(const auto& baseCollection : collection){

        //do not add signal to the total sideband
        if( baseCollection.isSMSignal() || baseCollection.isNewPhysicsSignal() ) continue;
        totalSideBand->Add( ( baseCollection.access(categoryIndex) ).get() );
    }

    //set negative bins to zero
    analysisTools::setNegativeBinsToZero(totalSideBand);
    return totalSideBand;
}


//get a map containing all background histograms, their names and whether they are a SM signal, for given category
//this map is used by the Plot class for plotting
std::map< std::string , std::pair< std::shared_ptr< TH1D >, bool > > HistCollectionDist::getBkgMap(const size_t categoryIndex) const{
    std::map< std::string, std::pair < std::shared_ptr<TH1D>, bool > > bkgMap;

    //add mainband MC backgrounds
    for(auto& bkgCol : collection){

        //only consider MC
        if(!bkgCol.isData()){

            //safety check to make sure files for same processes were already merged
            if(bkgMap.count(bkgCol.sampleProcessName()) != 0){
                std::cerr << "Error: finding several background collections of same process: check whether backgrounds were merged!" << std::endl;
            }
            bkgMap[bkgCol.sampleProcessName()] = {bkgCol.access(categoryIndex), bkgCol.isSMSignal()};
        }
    }

    //add nonPrompt background from sidebands if they exist
    if(hasSideBand()){
        bkgMap["Nonprompt e/#mu"] = {getTotalSideBand(categoryIndex), false};
    }   
    return bkgMap;
}

//get histogram containing the observed yield in given category
std::shared_ptr<TH1D> HistCollectionDist::getObsHist(const size_t categoryIndex) const{
    std::shared_ptr< TH1D > obs;
    for(const auto& baseCollection : collection){
        if( baseCollection.isData() ){

            if( obs.use_count() == 0){
                obs = baseCollection.access(categoryIndex);
            } else if(obs.use_count() == 1){
                obs->Add(baseCollection.access(categoryIndex).get());

            //check whether there aren't more than 2 data histograms: 2016 and 2017 data, or either (keep looping for safety-check)
            } else {
                std::cerr << "Error: multiple data histograms present in collection, not clear how to make plot" << std::endl;
            }

        }
    }
    return obs;
}

//get plot for given categoryIndex
Plot HistCollectionDist::getPlot(const size_t categoryIndex){

    //merge processes if this did not already happen
    mergeProcesses();

    //return plot object
    return Plot(
        plotPath(categoryIndex) + name(categoryIndex),  //name of plot
        { ( categoryIsBlinded(categoryIndex) ? "Total bkg.": "Obs." ), getObsHist(categoryIndex) },        //observed yield and its name
        getBkgMap(categoryIndex)        //background information
        //std::map< std::string, std::shared_ptr< TH1D > >(),     //empty uncertainty map: PLOTTING UNCERTAINTIES CURRENTLY NOT IMPLEMENTED
        );
}

//extract the correct plot header for each category
std::string HistCollectionDist::plotHeader(const size_t categoryIndex, const unsigned era) const{

    //final return value
    std::string header;

    //name of this category
    std::string category = categoryName(categoryIndex); 

    //check for particular flavor combination 
    if(category.find("_mm_") != std::string::npos) header += "#mu#mu : ";
    else if(category.find("_em_") != std::string::npos) header += "e#mu : ";
    else if(category.find("_ee_") != std::string::npos) header += "ee : ";

    //check what era the plot corresponds to (2016, 2017 or combination)
    bool is2016 = (era == 0);
    bool is2017 = (era == 1);
    bool isCombined = (era ==2);
    if(era > 2){
        std::cerr << "Error : era > 2 given in plot request, this can not be interpreted, assuming combined plot (era 2)." << std::endl;
        isCombined = true;
    }

    //check for particular Run era
    if(!isCombined){
        std::string year;
        if(is2016){
            year = "2016";
        } else {
            year = "2017";
        }   
        if(category.find("RunB") != std::string::npos) header += year + " Run B";
        else if(category.find("RunC") != std::string::npos) header += year + " Run C";
        else if(category.find("RunD") != std::string::npos) header += year + " Run D";
        else if(category.find("RunE") != std::string::npos) header += year + " Run E";
        else if(category.find("RunF") != std::string::npos) header += year + " Run F";
        else if(category.find("RunG") != std::string::npos) header += year + " Run G";
        else if(category.find("RunH") != std::string::npos) header += year + " Run H";
    }

    //default case just displays the luminosity
    if(is2016 && (category.find("Run") == std::string::npos) ){
        header += "35.9 fb^{-1}";
    } else if(is2017 && ( category.find("Run") == std::string::npos) ){
        header += "41.5 fb^{-1}";
    } else if(category.find("Run") == std::string::npos){
        header += "77.3 fb^{-1}";
    }
    header += " (13 TeV)";
    return header;
}


void HistCollectionDist::printPlots(const std::string& outputDirectory, const unsigned era, const std::string& analysis, bool log, bool normToData, TH1D* bkgSyst, const bool sigNorm, const bool drawSMSignalShape){

    //loop over all categories and output a plot for each one
    for(size_t c = 0; c < categorySize(); ++c){

        //print plot
        getPlot(c).draw(outputDirectory, analysis, log, normToData, plotHeader(c, era), bkgSyst, sigNorm, drawSMSignalShape);
    }           
}

unsigned HistCollectionDist::numberOfDataCollections() const{
    unsigned numDataCol = 0;
    for(auto& baseCollection : collection){
        if(baseCollection.isData()){
            ++numDataCol;
        }
    }
    
    //check whether the number of data collections is reasonable
    if( numDataCol == 0){
        std::cerr << "Error: no collection corresponding to data found in HistCollectionDist object." << std::endl;
    } else if(numDataCol > 2){
        std::cerr << "Error: more than 2 collections corresponding to data found in HistCollectionDist object." << std::endl;
    }
    
    return numDataCol;
}

//routine that sets data equal to the total background, thus blinding the data
void HistCollectionDist::blindData(const size_t categoryIndex){

    //merge processes if this did not already happen
    mergeProcesses();

    //find the number of data histograms, this determines whether they have to be scaled down before the summation over backgrounds 
    double scale = 1./( numberOfDataCollections() ); 

    //find the HistCollectionBase objects corresponding to data
    for(auto& dataCollection: collection){

        //verify we have data
        if(!dataCollection.isData()) continue;

        //for every category set data equal to total background
        bool first = true;
        for(auto& bkgCollection: collection){

            //add sideband containing nonprompt background if it exists
            if(bkgCollection.hasSideBand()){

                //be careful when resetting data shared_ptr, access copies the pointer, so overwriting this will just disentangle the pointer from the original!
                if(first){
                    dataCollection.access(categoryIndex)->Reset();
                    first = false;
                } 
                dataCollection.access(categoryIndex)->Add(bkgCollection.access(categoryIndex, true).get());
            }

            //only add main band for MC 
            if(bkgCollection.isData()) continue;

            //compute total background
            if(first){
                dataCollection.access(categoryIndex)->Reset();
                first = false;
            }
            dataCollection.access(categoryIndex)->Add(bkgCollection.access(categoryIndex, false).get());
        }

        //apply scaling to make sure the case of 2 data collections is handled correctly
        dataCollection.access(categoryIndex)->Scale(scale);
    }

    //add this category to the list of categories that were blinded
    blindedCategories.insert(categoryIndex);
}

void HistCollectionDist::blindData(const std::string& catName){

    //blind data for any category containing the given name
    for(size_t c = 0; c < categorySize(); ++c){
        if(categoryName(c).find(catName) != std::string::npos){
            blindData(c);
        }
    }
}

//check if particular category was blinded 
bool HistCollectionDist::categoryIsBlinded(const size_t categoryIndex) const{
    return ( blindedCategories.find(categoryIndex) != blindedCategories.cend() );
}

//rebin histograms for categories containing a certain name
void HistCollectionDist::rebin(const std::string& categoryName, const int numberOfBinsToMerge) const{
    for(auto& baseCollection : collection){
        baseCollection.rebin(categoryName, numberOfBinsToMerge);
    }    
}

//access histogram
std::shared_ptr<TH1D> HistCollectionDist::access(const size_t sampleIndex, const std::vector<size_t>& categoryIndices , const bool sb) const{
    //merge processes if this did not already happen
    // mergeProcesses();

    size_t categoryIndex = getCategoryIndex(categoryIndices);
    return access(sampleIndex, categoryIndex, sb);
}

