/*
Fill response matrices
*/

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TTree.h"
#include <Math/Vector4D.h>

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/readFakeRateTools.h"
#include "../../Tools/interface/readChargeFlipTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/variableTools.h"

// include analysis tools
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"


std::map< std::string, std::map< std::string, std::shared_ptr<TH2D> > > initResponseMap(
    const std::vector<HistogramVariable> histvars,
    const std::vector<std::string> instanceNames,
    int finebins, bool lowmass ){
    // initialize the output histogram map
    // note: the argument finebins overrides the number of bins of the histvars;
    //       it can be used to set a very fine binning (e.g. 1000 bins)
    //       to allow automatic bin edge optimization at a later stage;
    //       use 0 to take the bins from histvars.
    std::map< std::string, std::map< std::string, std::shared_ptr<TH2D> > > histMap;
    // loop over instances (could be e.g. event selection regions)
    for(std::string instanceName: instanceNames){
        // make a set of histograms for this instance
        std::map<std::string, std::shared_ptr<TH2D>> hists;
        if( finebins<=0 ){
	    hists = variableTools::initializeHistograms2D( histvars );
	} else{
	    for( HistogramVariable var: histvars ){
		std::shared_ptr<TH2D> hist;
		hist = std::make_shared<TH2D>(
		    var.name().c_str(), var.name().c_str(),
		    finebins, var.xlow(), var.xhigh(),
		    finebins, var.xlow(), var.xhigh() );
		hist->SetDirectory(0);
		hist->Sumw2();
		hists[var.name()] = hist;
	    }
	}
        // add the histograms to the total structure
        for(unsigned int i=0; i<histvars.size(); ++i){
            std::string variable = histvars[i].name();
            std::string lowname = "low" + instanceName + "_" + variable;
            std::string highname = "high" + instanceName + "_" + variable;
            histMap[instanceName][variable] = hists[variable];
            if( lowmass){
              histMap[instanceName][variable]->SetName(lowname.c_str());
            }
            else{
              histMap[instanceName][variable]->SetName(highname.c_str());
            }
        }
    }
    return histMap;
}


void fillHistograms(
    const std::string& inputDirectory,
    const std::string& sampleList,
    int sampleIndex,
    unsigned long nEvents,
    const std::string& outputDirectory,
    const std::vector<std::string>& eventSelections,
    const std::vector<HistogramVariable> histvars,
    int finebins ){
    std::cout << "=== start function fillHistograms ===" << std::endl;
    
    // initialize TreeReader from input file
    std::cout<<"initializing TreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // make reweighter
    std::cout << "initializing Reweighter..." << std::endl;
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    reweighterFactory = std::shared_ptr<ReweighterFactory>( 
	new EmptyReweighterFactory() ); // for testing
	//new Run2ULReweighterFactory() ); // for real
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    
    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::map< std::string,std::map< std::string,std::shared_ptr<TH2D>> > lowhistMap =
        initResponseMap( histvars, eventSelections, finebins,true );
    std::map< std::string,std::map< std::string,std::shared_ptr<TH2D>> > highhistMap =
        initResponseMap( histvars, eventSelections, finebins,false );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// loop over event selections
	for(std::string eventSelection: eventSelections){

	    // initialize map of variables
	    bool passDL = false;
	    bool passPL = false;
	    std::map<std::string,double> varmapDL = eventFlattening::initVarMap();
	    std::map<std::string,double> varmapPL = eventFlatteningParticleLevel::initVarMap();
	    Event event = treeReader.buildEvent(entry,false,false,false,false,true);
	    double weight = event.weight();

	    // do event selection at detector level
	    if( passES(event, eventSelection, "tight", "nominal") ){
		passDL = true;
		varmapDL = eventFlattening::eventToEntry(event,
		    reweighter, "tight", nullptr, nullptr, nullptr, "nominal");
	    }

	    // do selection at particle level
	    if( eventSelectionsParticleLevel::passES(event, eventSelection) ){
		passPL = true;
		varmapPL = eventFlatteningParticleLevel::eventToEntry(event);
	    }

	    // if both selections failed, skip event
	    if( !passDL && !passPL ) continue;

            // check the top masses, both have to be below or above to fill
            // just double going to this entry to be sure
            bool lowmass = false;
            std::vector<double> masses;
            int amnt = 0;
            treeReader.GetEntry(entry);
            for( unsigned j = 0; j < treeReader._gen_n; ++j ){
              if( abs(treeReader._gen_pdgId[j]) != 6)continue;
              if(not treeReader._gen_isLastCopy[j])continue;
              amnt +=1 ;
              ROOT::Math::PtEtaPhiEVector vec(treeReader._gen_pt[j],treeReader._gen_eta[j],treeReader._gen_phi[j],treeReader._gen_E[j]);
              masses.push_back(vec.M());
            }
            if(amnt !=2){std::cout<<"we have " << amnt <<"tops while we expected 2." ;}
            if(masses.at(0) < 172.2 and masses.at(1) < 172.2){lowmass = true;}
            else if(masses.at(0) > 172.8 and masses.at(1) > 172.8){lowmass = false;}
            else{continue;}

	    // loop over variables
	    for(HistogramVariable histVar: histvars){
		std::string variableName = histVar.name();
		std::string variable = histVar.variable();
		// if both selection passed, fill the response matrix in the usual way
		if( passDL && passPL ){
                    if( lowmass ){
		      histogram::fillValues( lowhistMap.at(eventSelection).at(variableName).get(),
					varmapPL.at(variable), varmapDL.at(variable), weight );
                    }
                    else{
                      histogram::fillValues( highhistMap.at(eventSelection).at(variableName).get(),
                                        varmapPL.at(variable), varmapDL.at(variable), weight );
                    }
		}
		// if one of both selections passed and the other failed,
		// fill with underflow bins
		if( passDL && !passPL ){
                    if( lowmass ){
		      std::shared_ptr<TH2D> lowhist = lowhistMap.at(eventSelection).at(variableName);
		      double xvalue = lowhist->GetXaxis()->GetBinLowEdge(1)-0.1;
		      double yvalue = histogram::boundedYValue(lowhist.get(), varmapDL.at(variable));
		      lowhist->Fill(xvalue, yvalue, weight);
                    }
                    else{
                      std::shared_ptr<TH2D> hist = highhistMap.at(eventSelection).at(variableName);
                      double xvalue = hist->GetXaxis()->GetBinLowEdge(1)-0.1;
                      double yvalue = histogram::boundedYValue(hist.get(), varmapDL.at(variable));
                      hist->Fill(xvalue, yvalue, weight);
                    }
		}
		if( !passDL && passPL ){
                    if( lowmass ){
		      std::shared_ptr<TH2D> lowhist = lowhistMap.at(eventSelection).at(variableName);
		      double xvalue = histogram::boundedXValue(lowhist.get(), varmapPL.at(variable));
		      double yvalue = lowhist->GetYaxis()->GetBinLowEdge(1)-0.1;
		      lowhist->Fill(xvalue, yvalue, weight);
                    }
                    else{
                      std::shared_ptr<TH2D> hist = highhistMap.at(eventSelection).at(variableName);
                      double xvalue = histogram::boundedXValue(hist.get(), varmapPL.at(variable));
                      double yvalue = hist->GetYaxis()->GetBinLowEdge(1)-0.1;
                      hist->Fill(xvalue, yvalue, weight);
                    }
		}
	    } // end loop over variables
	} // end loop over event selections
    } // end loop over events

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over event selections
    for(std::string eventSelection: eventSelections){
	// loop over variables
	for(HistogramVariable histVar: histvars){
	    std::string variableName = histVar.name();
	    std::shared_ptr<TH2D> hist = lowhistMap[eventSelection][variableName];
	    hist->Write();
            std::shared_ptr<TH2D> highhist = highhistMap[eventSelection][variableName];
            highhist->Write();
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 9 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " variable_file, event_selection,";
	std::cerr << " nevents finebins" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& variable_file = argvStr[5];
    std::string& event_selection = argvStr[6];
    std::vector<std::string> event_selections = stringTools::split(event_selection,",");
    unsigned long nevents = std::stoul(argvStr[7]);
    int finebins = std::stoi(argvStr[8]);

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - variable file: " << variable_file << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;
    std::cout << "  - number of finebins (0 for default): " << std::to_string(finebins) << std::endl;

    // read variables
    std::vector<HistogramVariable> histvars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histvars ){
	std::cout << var.toString() << std::endl;	
    }*/

    // check variables
    std::map<std::string,double> emptymapDL = eventFlattening::initVarMap();
    std::map<std::string,double> emptymapPL = eventFlatteningParticleLevel::initVarMap();

    for(HistogramVariable histVar: histvars){
	std::string variableName = histVar.name();
        std::string variable = histVar.variable();
        if( emptymapDL.find(variable)==emptymapDL.end() ){
	    std::string msg = "ERROR: variable "+variable+" ("+variableName+")";
	    msg.append(" not recognized in detector-level variable map.");
	    throw std::invalid_argument(msg);
	}
	if( emptymapPL.find(variable)==emptymapPL.end() ){
            std::string msg = "ERROR: variable "+variable+" ("+variableName+")";
            msg.append(" not recognized in particle-level variable map.");
            throw std::invalid_argument(msg);
        }
    }

    // fill the histograms
    fillHistograms( input_directory, 
		    sample_list, 
		    sample_index, 
		    nevents,
		    output_directory,
		    event_selections,
		    histvars,
                    finebins );
    std::cerr << "###done###" << std::endl;
    return 0;
}
