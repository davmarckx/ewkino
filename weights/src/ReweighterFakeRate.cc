#include "../interface/ReweighterFakeRate.h"

//include c++ library classes
#include <vector>

//include ROOT classes
#include "TFile.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/readFakeRateTools.h"


// ---------------
// help functions
// ---------------

std::shared_ptr<TH2D> morph(
    const std::shared_ptr<TH2D>& lower,
    const std::shared_ptr<TH2D>& upper,
    const std::string& axis){
    // make a map that varies linearly from lower to upper along the x-axis
    // check arguments
    if(axis!="x" && axis!="y"){
	std::string msg = "ERROR in ReweighterFakeRate::morph:";
	msg += " axis '"+axis+"' not recognized.";
	throw std::runtime_error(msg);
    }
    // initializations
    std::shared_ptr<TH2D> morph = std::shared_ptr<TH2D>(static_cast<TH2D*>(lower->Clone()));
    double xmin = morph->GetXaxis()->GetBinCenter(1);
    double xmax = morph->GetXaxis()->GetBinCenter(morph->GetNbinsX());
    double ymin = morph->GetYaxis()->GetBinCenter(1);
    double ymax = morph->GetYaxis()->GetBinCenter(morph->GetNbinsY());
    // loop over bins
    for(int i = 0; i <= morph->GetNbinsX()+1; ++i){
        for(int j = 0; j <= morph->GetNbinsY()+1; ++j){
	    // determine interpolation faction
	    double frac = 0;
	    if(axis=="x"){ frac = (morph->GetXaxis()->GetBinCenter(i) - xmin) / (xmax - xmin); }
	    else{ frac = (morph->GetYaxis()->GetBinCenter(j) - ymin) / (ymax - ymin); }
	    // do interpolation
	    morph->SetBinContent(i,j,
                lower->GetBinContent(i,j) + frac*(upper->GetBinContent(i,j)-lower->GetBinContent(i,j)) );
        }
    }
    return morph;
}


// ------------
// constructor
// ------------

ReweighterFakeRate::ReweighterFakeRate(
    const std::string& electronFakeRateFile,
    const std::string& muonFakeRateFile,
    const std::string& flavor,
    const std::string& year,
    const std::string& uncType) {
    // input arguments:
    // - electronFakeRateFile: path to a root file containing electron fake rate histograms
    // - muonFakeRateFile: path to a root file containing muon fake rate histograms
    // - flavor: either "electron", "muon" or "both"
    //   (to determine which flavor fake rate maps will be varied)
    // - year: data taking year (needed to retrieve the correct histogram in the file)
    // - uncType: either "norm", "pt", or "eta"
    //   (to determine what kind of variation will be applied)
    //   - "norm": vary the whole map up (or down) simultaneously
    //   - "pt": vary the map from down to up as a function of pt
    //   - "eta": vary the map from down to up as a function of eta
    // notes:
    // - preliminary implementation, partially but not fully synchronized with Oviedo.
    //   the idea is to use a fake rate weight obtained with the fake rate map
    //   varied up/down within statistical uncertainty.
    //   this is not really realistic as all statistical errors are considered correlated.

    // check arguments
    if(flavor!="electron" && flavor!="muon" && flavor!="both"){
	std::string msg = "ERROR in ReweighterFakeRate: flavor '"+flavor+"' not recognized.";
	throw std::runtime_error(msg);
    }
    if(uncType!="norm" && uncType!="pt" && uncType!="eta"){
        std::string msg = "ERROR in ReweighterFakeRate: uncType '"+uncType+"' not recognized.";
        throw std::runtime_error(msg);
    }
    // get nominal fake rate maps
    electronFakeRateMap = readFakeRateTools::readFRMap(electronFakeRateFile, "electron", year);
    muonFakeRateMap = readFakeRateTools::readFRMap(muonFakeRateFile, "muon", year);
    // make fake rate maps plus and minus statistical uncertainty for electrons
    // (which are just nominal if flavor is muon)
    electronFakeRateMapStatUp = std::shared_ptr<TH2D>(static_cast<TH2D*>(electronFakeRateMap->Clone()));
    electronFakeRateMapStatDown = std::shared_ptr<TH2D>(static_cast<TH2D*>(electronFakeRateMap->Clone()));
    if(flavor=="electron" || flavor=="both"){
	for(int i = 0; i <= electronFakeRateMap->GetNbinsX()+1; ++i){
	    for(int j = 0; j <= electronFakeRateMap->GetNbinsY()+1; ++j){
		electronFakeRateMapStatUp->SetBinContent(i,j,
		    electronFakeRateMap->GetBinContent(i,j) + electronFakeRateMap->GetBinError(i,j));
		electronFakeRateMapStatDown->SetBinContent(i,j,
		    electronFakeRateMap->GetBinContent(i,j) - electronFakeRateMap->GetBinError(i,j));
	    }
	}
    }
    // make fake rate maps plus and minus statistical uncertainty for muons
    // (which are just nominal if flavor is electron)
    muonFakeRateMapStatUp = std::shared_ptr<TH2D>(static_cast<TH2D*>(muonFakeRateMap->Clone()));
    muonFakeRateMapStatDown = std::shared_ptr<TH2D>(static_cast<TH2D*>(muonFakeRateMap->Clone()));
    if(flavor=="muon" || flavor=="both"){
        for(int i = 0; i <= muonFakeRateMap->GetNbinsX()+1; ++i){
            for(int j = 0; j <= muonFakeRateMap->GetNbinsY()+1; ++j){
                muonFakeRateMapStatUp->SetBinContent(i,j,
                    muonFakeRateMap->GetBinContent(i,j) + muonFakeRateMap->GetBinError(i,j));
                muonFakeRateMapStatDown->SetBinContent(i,j, 
                    muonFakeRateMap->GetBinContent(i,j) - muonFakeRateMap->GetBinError(i,j));
            }   
        }
    }
    // make templates based on uncertainty type
    if(uncType=="pt"){
	electronFakeRateMapUp = morph(electronFakeRateMapStatDown, electronFakeRateMapStatUp, "x");
	electronFakeRateMapDown = morph(electronFakeRateMapStatUp, electronFakeRateMapStatDown, "x");
	muonFakeRateMapUp = morph(muonFakeRateMapStatDown, muonFakeRateMapStatUp, "x");
	muonFakeRateMapDown = morph(muonFakeRateMapStatUp, muonFakeRateMapStatDown, "x");
    } else if(uncType=="eta"){
        electronFakeRateMapUp = morph(electronFakeRateMapStatDown, electronFakeRateMapStatUp, "y");
	electronFakeRateMapDown = morph(electronFakeRateMapStatUp, electronFakeRateMapStatDown, "y");
        muonFakeRateMapUp = morph(muonFakeRateMapStatDown, muonFakeRateMapStatUp, "y");
	muonFakeRateMapDown = morph(muonFakeRateMapStatUp, muonFakeRateMapStatDown, "y");
    } else{
        electronFakeRateMapUp = electronFakeRateMapStatUp;
	electronFakeRateMapDown = electronFakeRateMapStatDown;
        muonFakeRateMapUp = muonFakeRateMapStatUp;
	muonFakeRateMapDown = muonFakeRateMapStatDown;
    }
}


// ---------------------------
// weight retrieval functions 
// ---------------------------

double ReweighterFakeRate::weight( const Event& event ) const{
    // retrieve weight for an event
    // return 1 since this reweighter is only for relative variations wrt nominal
    // dummy condition on event to avoid compilation warnings
    if( event.isData() ) return 1;
    return 1;
}

double ReweighterFakeRate::weightDown( const Event& event ) const{
    // retrieve down-varied weight for and event
    double nom = readFakeRateTools::fakeRateWeight(event,
                    muonFakeRateMap, electronFakeRateMap);
    if( nom<1e-12 ) return 1;
    double down = readFakeRateTools::fakeRateWeight(event,
		    muonFakeRateMapDown, electronFakeRateMapDown);
    return down/nom;
}

double ReweighterFakeRate::weightUp( const Event& event ) const{
    // retrieve up-varied weight for an event
    double nom = readFakeRateTools::fakeRateWeight(event,
                    muonFakeRateMap, electronFakeRateMap);
    if( nom<1e-12 ) return 1;
    double up = readFakeRateTools::fakeRateWeight(event,
                    muonFakeRateMapUp, electronFakeRateMapUp);
    return up/nom;
}
