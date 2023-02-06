#include "../interface/analysisTools.h"

//Include c++ library classes
#include <iostream>
#include <string>
#include <fstream>

//include other parts of framework
#include "../interface/stringTools.h"


void analysisTools::printProgress( double progress ){
    const unsigned barWidth = 100;
    std::cout << "[";
    unsigned pos = barWidth * progress;
    for (unsigned i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << unsigned(progress * 100.0) << " %\r" << std::flush;
}


void analysisTools::setNegativeBinsToZero( TH1D* h ){

	//check each bin of the histogram, and set its binContent to 0 if it is negative
    for(int b = 1; b < h->GetNbinsX() + 1; ++b){
        if(h->GetBinContent(b) < 0.) h->SetBinContent(b, 0.);
    }
}


void analysisTools::setNegativeBinsToZero( const std::shared_ptr< TH1D >& h){
	analysisTools::setNegativeBinsToZero( h.get() );
}


//Function to print dataCard to be analysed by CMS combination tool
void analysisTools::printDataCard(const std::string& cardName, const double obsYield, const double sigYield, const std::string& sigName, const double* bkgYield, const unsigned nBkg, const std::string* bkgNames, const std::vector<std::vector<double> >& systUnc, const unsigned nSyst, const std::string* systNames, const std::string* systDist, const bool shapeCard, const std::string& shapeFileName, const bool autoMCStats ){

	//stream for writing card
    std::ofstream card;

    //add .txt to name if no file extension is given
    card.open(cardName + ((cardName.find(".") == std::string::npos) ? ".txt" : "") ); //add .txt to name if no file extension is given

    //define number of channels, background sources and systematics
    card << "imax 1 number of channels \n";
    card << "jmax " << nBkg << " number of backgrounds \n";
    card << "kmax " << nSyst << " number of nuisance parameters (sources of systematical uncertainties) \n";
    card << "---------------------------------------------------------------------------------------- \n";

    //define the channels and the number of observed events
    card << "bin bin1 \n";
    card << "observation " << obsYield << "\n";

    //define all backgrounds and their yields
    card << "---------------------------------------------------------------------------------------- \n";
    if(shapeCard){
        card << "shapes * * " << shapeFileName + ".root  $PROCESS $PROCESS_$SYSTEMATIC\n";
        card << "---------------------------------------------------------------------------------------- \n";
    }
    card << "bin	";
    for(unsigned proc = 0; proc < nBkg + 1; ++proc){
        card << "	" << "bin1";
    }
    card << "\n";
    card << "process";
    card << "	" << sigName;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
        card << "	" << bkgNames[bkg];
    }
    card << "\n";
    card << "process";
    for(unsigned bkg = 0; bkg < nBkg + 1; ++bkg){
        card << "	" << bkg;
    }
    card << "\n";
    card <<	"rate";
    card << "	" << sigYield;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
        if(bkgYield[bkg] <= 0) card << "	" << "0.00";
        else card << "	" << bkgYield[bkg];
    }
    card << "\n";
    card << "---------------------------------------------------------------------------------------- \n";

    //define sources of systematic uncertainty, what distibution they follow and how large their effect is
    if( nSyst != 0 ){
        for(unsigned syst = 0; syst < nSyst; ++syst){
            card << systNames[syst] << "	" << systDist[syst];
            for(unsigned proc = 0; proc < nBkg + 1; ++proc){
                card << "	";
                if(systUnc[syst][proc] == 0) card << "-";
                else card << systUnc[syst][proc];
            }
            card << "\n";
        }
    }

    //add line to automatically include statistical uncertainties from the MC shape histograms 
    if( autoMCStats ){
        card << "* autoMCStats 0\n";
    }
    
    card.close();		
}


// check what year a sample file corresponds to

bool analysisTools::fileIs2016( const std::string& filePath ){
    // note: for simulation this is only true for files covering all of 2016 (legacy sim),
    //	     not for ultra-legacy sim which is split in 2016 PreVFP and 2016 PostVFP;
    //       for data this is true for individual eras if the era belongs to 2016 
    //	     (both for legacy and ultra-legacy), but not necessarily for combined eras,
    //	     depending on whether the combination has PreVFP or PostVFP in its name.

    // first veto PreVFP and PostVFP
    if( stringTools::stringContains( filePath, "16PreVFP" )
        || stringTools::stringContains( filePath, "16PostVFP" ) ) return false;
    // next check specific tags
    return ( stringTools::stringContains( filePath, "MiniAOD2016" ) // pre-UL simulation
        || stringTools::stringContains( filePath, "Run2016" ) // data
        || stringTools::stringContains( filePath, "Summer16" ) // tag given by skimmer
        || stringTools::stringEndsWith( filePath, "data_2016.root" )
        || stringTools::stringStartsWith( stringTools::fileNameFromPath( filePath ), "2016" )
    );
}


bool analysisTools::fileIs2016PreVFP( const std::string& filePath ){
    //  note: for simulation this is only true for ultra-legacy files from 2016 preVFP;
    //        for data this is true if the era belongs to 2016 B, C, D, E or F 
    //        (both for legacy and ultra-legacy), 
    //	      but note that fileIs2016 is also true in that case.
    // for data split in eras
    if( stringTools::stringContains( filePath, "Run2016B" )
	|| stringTools::stringContains( filePath, "Run2016C" )
	|| stringTools::stringContains( filePath, "Run2016D" )
	|| stringTools::stringContains( filePath, "Run2016E" ) ) return true;
    // special case for era F which is split over PreVFP and PostVFP
    if( stringTools::stringContains( filePath, "Run2016F" )
        && (stringTools::stringContains( filePath, "PreVFP" )
        || stringTools::stringContains( filePath, "HIPM" ) ) ) return true;
    // for combined data
    if( stringTools::stringContains( filePath, "Run2016PreVFP" ) ) return true;
    // for simulation (to be extended)
    if( stringTools::stringContains( filePath, "Summer20UL16MiniAODAPV" ) // UL simulation
	|| stringTools::stringContains( filePath, "Summer16PreVFP") // tag given by skimmer 
	) return true;
    return false;
}


bool analysisTools::fileIs2016PostVFP( const std::string& filePath ){
    // note: for simulation this is only true for ultra-legacy files from 2016 postVFP
    //       for data this is true if the era belongs to 2016 G or H 
    //       (both for legacy and ultra-legacy),
    //	     but note that fileIs2016 is also tru in that case.
    // need to check 2016PreVFP first since overlapping names for simulation
    if( fileIs2016PreVFP( filePath ) ) return false;
    // for data split in eras
    if( stringTools::stringContains( filePath, "Run2016F" )
	|| stringTools::stringContains( filePath, "Run2016G" ) 
        || stringTools::stringContains( filePath, "Run2016H" ) ) return true;
    // for combined data
    if( stringTools::stringContains( filePath, "Run2016PostVFP" ) ) return true;
    // for simulation (to be extended)
    if( stringTools::stringContains( filePath, "Summer20UL16MiniAOD" ) // UL simulation
	|| stringTools::stringContains( filePath, "Summer16PostVFP") // tag given by skimmer
	) return true;
    return false;
}


bool analysisTools::fileIs2017( const std::string& filePath ){
    return ( stringTools::stringContains( filePath, "MiniAOD2017" ) // pre-UL simulation
	|| stringTools::stringContains( filePath, "Summer20UL17" ) // UL simulation
        || stringTools::stringContains( filePath, "Run2017" ) // data
        || stringTools::stringContains( filePath, "Fall17" ) // tag given by skimmer
        || stringTools::stringEndsWith( filePath, "data_2017.root" )
        || stringTools::stringEndsWith( filePath, "combined_2017.root" )
        || stringTools::stringContains( filePath, "_2017_v" )
        || stringTools::stringStartsWith( stringTools::fileNameFromPath( filePath ), "2017" )
    );
}


bool analysisTools::fileIs2018( const std::string& filePath ){
    return ( stringTools::stringContains( filePath, "MiniAOD2018" ) // pre-UL simulation
	|| stringTools::stringContains( filePath, "Summer20UL18" ) // UL simulation
        || stringTools::stringContains( filePath, "Run2018" ) // data
        || stringTools::stringContains( filePath, "Autumn18" ) // tag given by skimmer
        || stringTools::stringEndsWith( filePath, "data_2018.root" )
        || stringTools::stringEndsWith( filePath, "combined_2018.root" )
        || stringTools::stringContains( filePath, "_2018_v" )
        || stringTools::stringStartsWith( stringTools::fileNameFromPath( filePath ), "2018" )
    );
}


std::pair< bool, bool > analysisTools::fileIs2017Or2018( const std::string& filePath ){
    bool is2017 = fileIs2017( filePath );
    bool is2018 = fileIs2018( filePath );

    //check consistency
    if( is2017 && is2018 ){
        throw std::invalid_argument( "File '" + filePath + "' is flagged as both 2017 and 2018," 
				    +" and should only be one of the two." );
    }
    return { is2017, is2018 };
}


void analysisTools::checkYearString( const std::string& yearString ){
    if( !( yearString == "2016" || yearString == "2016PreVFP" || yearString == "2016PostVFP"
	    || yearString == "2017" || yearString == "2018" ) ){
        throw std::invalid_argument( "Year string is '" + yearString +
		"' while it must be either '2016', '2016PreVFP', '2016PostVFP', '2017' or '2018'" );
    }
}


bool analysisTools::sampleIsSusy( const std::string& filePath ){
    return ( stringTools::stringContains( filePath, "SMS-T")
        || stringTools::stringContains( filePath, "TChi" )
    );
}
