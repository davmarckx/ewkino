#include "../interface/Plot.h"

//include plotting scripts
#include "../plotting/plotCode.h"

#include "TCanvas.h"

void Plot::draw(const std::string& outputDirectory, const std::string& analysis, bool log, bool normToData, const std::string& header, TH1D* bkgSyst, const bool sigNorm, const bool drawSMSignalShape) const{
    //first name given to plotting function is that of the data (or pseudodata)
    std::vector< std::string> names = { data.first };
    std::vector< TH1D*> bkgHist;
    //warning, vector of booleans can not be used because it is a bit-field in c++!
    bool isSMSignalVec[(const size_t) bkg.size()];
    size_t counter = 0; //to keep track of array index
    for(auto bkgIt = bkg.cbegin(); bkgIt != bkg.cend(); ++bkgIt){
        //first part of a bkg entry is the background name 
        //the second entry is the histogram itself and whether it is an SM signal
        names.push_back( newName(bkgIt->first) );
        isSMSignalVec[counter] = bkgIt->second.second;
        bkgHist.push_back(bkgIt->second.first.get());
        ++counter;
    }
    std::string outputDir(outputDirectory);
    if(outputDir.back() != '/') outputDir.append("/");

    //add SM signal shape to plot if requested
    std::vector< TH1D* > SMSignal;
    std::vector< std::string > SMSignalNames;
    if(drawSMSignalShape){
        for(auto bkgIt = bkg.cbegin(); bkgIt != bkg.cend(); ++bkgIt){
            //check whether process is signal
            if( bkgIt->second.second ){ 
                SMSignalNames.push_back(newName(bkgIt->first) + " shape");
                SMSignal.push_back( (TH1D*) bkgIt->second.first.get()->Clone() );  //clone to avoid coloring conflicts 
            }
        }
    }

    //TO DO : implement BSM signals

    //call plotting function
    plotDataVSMC(
        data.second.get(),     //data histogram
        &bkgHist[0],    //pointer to first element of bkgHist ( function expects array )
        &names[0],      //bkg names
        bkgHist.size(), //number of backgrounds
        outputDir + fileName + (drawSMSignalShape ? "_withSignalShape" : "" ) + (log ? "_log" : "_lin"),  //name of output file
        analysis, //the analysis determines the color scheme
        log,    //plot on a log scale
        normToData,     //normalize background to data or not 
        header,         //plot header
        bkgSyst,        //background systematics
        isSMSignalVec, //array of booleans whether given process is SM signal
        (drawSMSignalShape ? &SMSignal[0] : nullptr ),        //new physics signals CURRENTLY NOT IMPLEMENTED
        (drawSMSignalShape ? &SMSignalNames[0] : nullptr ),    
        (drawSMSignalShape ? SMSignal.size() : 0),
        sigNorm);
}


std::map < std::string, std::string> Plot::processNameMap = 
        {
        {"TT", "TT + Jets"},
        {"TTDiLep", "TT dilep."},
        {"TTSingleLep", "TT semilep."},
        {"WJets", "WJets"},
        {"TTX", "TT + X"},
        {"ST", "T + X"},
        {"DY", "DY"},
        {"VV", "VV"},
        {"TTZ", "TT + Z"},
        {"Xgamma", "X + #gamma"},
        {"ZZH", "ZZ/H"},
        {"tZq", "tZq"},
        {"WZ", "WZ"},
        {"multiboson", "Multiboson"}
        };


//extract name of background from map
std::string Plot::newName(const std::string& backgroundName) const{
    //if old name is stored in the map return the new name, otherwise keep the old one
    if(processNameMap.count(backgroundName) == 1){
        return processNameMap[backgroundName];
    } else{
        return backgroundName;
    }
}
