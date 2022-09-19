/*
Class containing data and background histograms for plotting
*/
#ifndef Plot_H
#define Plot_H
//include c++ library classes
#include <map>
#include <utility>
#include <string>
#include <memory>

#include <iostream>

//include root classes
#include "TH1D.h"

class Plot{
    public:
        Plot(
            const std::string& file,    //plot file name
            const std::pair< std::string, std::shared_ptr<TH1D> >& obs,  //observed yield and name
            const std::map< std::string, std::pair< std::shared_ptr < TH1D >, bool >  >& back,  //map of backgrounds, their names and whether they are a SM signal
            const std::map< std::string, std::shared_ptr< TH1D > >& unc = std::map< std::string, std::shared_ptr< TH1D > >(), //map of systematic uncertainties corresponding background names
            const std::map< std::string, std::shared_ptr< TH1D > >& sig = std::map< std::string, std::shared_ptr< TH1D > >()  //map of new physics signals and their names 
            ): 
            fileName(file), data(obs), bkg(back), syst(unc), signal(sig) {}

        //draw the plot to canvas and save 
        void draw(const std::string& outputDirectory, const std::string& analysis = "", bool log = false, bool normToData = false, const std::string& header = "", TH1D* bkgSyst = nullptr, const bool sigNorm = true, const bool drawSMSignalShape = false) const;

    private:
        std::pair< std::string, std::shared_ptr<TH1D> > data;
        std::map< std::string, std::pair< std::shared_ptr< TH1D >, bool > > bkg;
        std::map< std::string, std::shared_ptr< TH1D > > syst; 
        std::map< std::string, std::shared_ptr< TH1D > > signal;
        std::string fileName;

        //map to extract plot legend name given process name
        static std::map < std::string, std::string> processNameMap;

	    //extract name of background from map
	    std::string newName(const std::string&) const;
};
#endif
