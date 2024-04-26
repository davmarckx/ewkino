// Fill theory (i.e. particle level) differential cross-section distributions

// Note: the input files should be in NanoAOD or NanoGen format!
//       This script uses the NanoGenTreeReader instead of the normal TreeReader.
//	 This is basically a copy of differential/filltheory.cc but with different TreeReader.
//	 Later check if it can be merged in a single script that switches TreeReader as needed.

// Note: contrary to the analysis script at detector level,
//       this script uses the generator weight of each event instead of the full weight
//       (i.e. event.genWeight() instead of event.weight()).
//       Hence the histograms produced by this script are not normalized correctly to xsection;
//       an additional normalization factor of xsection / hCounter needs to be applied
//       in post-processing (and the hCounter is written to the output file for convenience).
//       The motivation for this design is to allow quick xsection or lumi changes,
//       without having to re-run this script.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of the framework
#include "../../TreeReader/interface/NanoGenTreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/NanoGenSampleCrossSections.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"
#include "../analysis/interface/systematicTools.h"


std::map< std::string,     // process
    std::map< std::string, // event selection
    std::map< std::string, // selection type
    std::map< std::string, // variable
    std::map< std::string, // systematic
    std::shared_ptr<TH1D> > > > > > initHistMap(
    const std::vector<std::string>& processNames,
    const std::vector<std::string>& eventSelections,
    const std::vector<std::string>& selectionTypes,
    const std::vector<HistogramVariable>& histVars,
    const std::vector<std::string>& systematics,
    unsigned numberOfPdfVariations=100,
    unsigned numberOfQcdScaleVariations=6){
    // make map of histograms
    // the resulting map has five levels: 
    // map[process name][event selection][selection type][variable name][systematic]
    // notes:
    // - processNames should be just one name (per file) in most cases,
    //   but can be multiple if a sample is split in sub-categories.
    // - selectionTypes supports only one value (for now): particle level selection.
    
    // initialize the output histogram map
    std::map< std::string, 
	std::map< std::string,
	std::map< std::string, 
	std::map< std::string, 
	std::map< std::string, 
	std::shared_ptr<TH1D> > > > > > histMap;
    // loop over process names
    for(std::string processName: processNames){
	// loop over event selections
	for(std::string eventSelection: eventSelections){
	    // loop over selection types
	    for(std::string selectionType: selectionTypes){
		// loop over variables
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    // form the histogram name
		    std::string baseName = processName+"_"+eventSelection+"_"+selectionType+"_"+variableName;
		    baseName = stringTools::removeOccurencesOf(baseName," ");
		    baseName = stringTools::removeOccurencesOf(baseName,"/");
		    baseName = stringTools::removeOccurencesOf(baseName,"#");
		    baseName = stringTools::removeOccurencesOf(baseName,"{");
		    baseName = stringTools::removeOccurencesOf(baseName,"}");
		    baseName = stringTools::removeOccurencesOf(baseName,"+");
		    // add nominal histogram
		    histMap[processName][eventSelection][selectionType][variableName]["nominal"] = histVar.initializeHistogram( baseName+"_nominal" );
		    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at("nominal")->SetTitle(processName.c_str());
		    // add special histograms holding hCounter info
		    HistogramVariable histInfoHCounter( "", "", 1, 0, 1 );
                    histMap[processName][eventSelection][selectionType][variableName]["hCounter"] = histInfoHCounter.initializeHistogram( baseName+"_hCounter" );
                    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at("hCounter")->SetTitle(processName.c_str());
		    // loop over systematics
		    for(std::string systematic : systematics){
			// special case for PDF variations: store individual variations 
			// as well as rms
			if(systematic=="pdfShapeVar"){
			    for(unsigned i=0; i<numberOfPdfVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			    std::vector<std::string> temps = {"pdfShapeRMSUp", "pdfShapeRMSDown"};
			    for(std::string temp: temps){
				histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			else if(systematic=="pdfTotalVar"){
                            for(unsigned i=0; i<numberOfPdfVariations; ++i){
                                std::string temp = systematic + std::to_string(i);
                                histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
                                histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
                            }
                            std::vector<std::string> temps = {"pdfTotalRMSUp", "pdfTotalRMSDown"};
                            for(std::string temp: temps){
                                histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
                                histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
                            }
                        }
			// special case for QCD scales: store individual variations
			// as well as envelope
			else if(systematic=="qcdScalesShapeVar"){
			    for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			    std::vector<std::string> temps = {"qcdScalesShapeEnvUp", "qcdScalesShapeEnvDown"};
			    for(std::string temp: temps){
				histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			else if(systematic=="qcdScalesTotalVar"){
                            for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
                                std::string temp = systematic + std::to_string(i);
                                histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
                                histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
                            }
                            std::vector<std::string> temps = {"qcdScalesTotalEnvUp", "qcdScalesTotalEnvDown"};
                            for(std::string temp: temps){
                                histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
                                histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
                            }
                        }
			// now general case: store up and down variation
			else{
			    std::string temp = systematic + "Up";
			    histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
			    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    temp = systematic + "Down";
			    histMap[processName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
			    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			}
		    }
		}
	    }
	}
    }
    return histMap;
}


void fillTheoryHistograms(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<HistogramVariable>& histVars, 
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
	    unsigned long nEntries,
            std::vector<std::string>& systematics ){
    // initialize TreeReader from input file
    std::cout << "=== start function fillTheoryHistograms ===" << std::endl;;
    std::cout << "initializing TreeReader and setting to sample n. " << sampleIndex << "..." << std::endl;
    NanoGenTreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample(sampleIndex, true);
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // get hCounter info
    double hCounterWeights = treeReader.sumGenWeights();
    int hCounterEntries = treeReader.numGenEvents();
    std::cout << "retrieved follwing info from hCounter:" << std::endl;
    std::cout << "  - sum of weights: " << hCounterWeights << std::endl;
    std::cout << "  - entries: " << hCounterEntries << std::endl;

    // determine global sample properties related to pdf and scale variations
    unsigned numberOfScaleVariations = 0;
    unsigned numberOfPdfVariations = 0;
    std::shared_ptr< NanoGenSampleCrossSections > xsecs;
    std::vector< double > qcdScalesXSecRatios;
    double qcdScalesMinXSecRatio = 1.;
    double qcdScalesMaxXSecRatio = 1.;
    std::vector<double> pdfXSecRatios;
    double pdfMinXSecRatio = 1.;
    double pdfMaxXSecRatio = 1.;
    // check whether lhe systematics are needed
    bool considerlhe = true;
    bool hasValidPdfs = false; // global switch to use later on in the event loop
    bool hasValidQcds = false; // global switch to use later on in the event loop
    if( treeReader.numberOfEntries()>0 
	&& considerlhe 
	&& !treeReader.isData() ){
	std::cout << "finding available PDF and QCD scale variations..." << std::endl;
	Event event = treeReader.buildEvent(0); 
        numberOfScaleVariations = event.generatorInfo().numberOfScaleVariations();
	numberOfPdfVariations = event.generatorInfo().numberOfPdfVariations();	
	// make vector of cross-section modifications due to pdf and qcd scale variations
	// the vectors contain the ratio of the varied cross-section to the nominal one
	xsecs = std::make_shared<NanoGenSampleCrossSections>( treeReader );
	if(numberOfScaleVariations==9){
	    hasValidQcds = true;
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_2() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_1() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_2() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5() );
	    // note: order doesnt matter here as it is only used for min and max calculation
	    qcdScalesMinXSecRatio = *std::min_element( qcdScalesXSecRatios.begin(), 
							qcdScalesXSecRatios.end() );
	    qcdScalesMaxXSecRatio = *std::max_element( qcdScalesXSecRatios.begin(), 
							qcdScalesXSecRatios.end() );
	}
	if(numberOfPdfVariations>=100){
	    hasValidPdfs = true;
	    for(unsigned i=0; i<numberOfPdfVariations; i++){
		double val = xsecs.get()->crossSectionRatio_pdfVar(i);
		pdfXSecRatios.push_back( val ); }
	    pdfMinXSecRatio = *std::min_element( pdfXSecRatios.begin(), pdfXSecRatios.end() );
	    pdfMaxXSecRatio = *std::max_element( pdfXSecRatios.begin(), pdfXSecRatios.end() );
	}
	// printouts
	std::cout << "- hasValidQcds: " << hasValidQcds << std::endl;
	std::cout << "    minimum QCD scale modified cross-section: ";
	std::cout << std::to_string(qcdScalesMinXSecRatio) << std::endl;
	std::cout << "    maximum QCD scale modified cross-section: ";
        std::cout << std::to_string(qcdScalesMaxXSecRatio) << std::endl;
	std::cout << "- hasValidPdfs: " << hasValidPdfs << std::endl;
	std::cout << "    minimum PDF modified cross-section: ";
        std::cout << std::to_string(pdfMinXSecRatio) << std::endl;
        std::cout << "    maximum PDF modified cross-section: ";
        std::cout << std::to_string(pdfMaxXSecRatio) << std::endl;
    }

    // determine global sample properties related to ISR/FSR variations
    unsigned numberOfPSVariations = 0;
    // check whether parton shower parameters are needed
    bool considerps = true;
    bool hasValidPSs = false; // global switch to use later on in the event loop
    if( treeReader.numberOfEntries()>0
        && considerps
        && !treeReader.isData() ){
        std::cout << "finding available PS scale variations..." << std::endl;
        Event event = treeReader.buildEvent(0);
        numberOfPSVariations = event.generatorInfo().numberOfPsWeights();
	// make vector of cross-section modifications due to ps variations
        // the vectors contain the ratio of the varied cross-section to the nominal one
	// (not used here but called later in event loop!)
        xsecs = std::make_shared<NanoGenSampleCrossSections>( treeReader );
        if(numberOfPSVariations==4){ hasValidPSs = true; }
        std::cout << "- hasValidPSs: " << hasValidPSs << std::endl;
    }

    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::vector<std::string> processNames = {processName};
    std::map< std::string,     // process
	std::map< std::string, // event selection
	std::map< std::string, // selection type
	std::map< std::string, // variable
	std::map< std::string, // systematic
        std::shared_ptr<TH1D> > > > > > histMap = initHistMap(
	    processNames, event_selections, selection_types, 
	    histVars, systematics,
	    numberOfPdfVariations, 6);

    // fill hCounters
    for( std::string event_selection: event_selections ){
    for( std::string selection_type: selection_types ){
    for( HistogramVariable histVar: histVars ){
        std::string variableName = histVar.name();
	std::shared_ptr<TH1D> hCounter = histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at("hCounter");
	hCounter->SetBinContent(1, hCounterWeights);
	hCounter->SetBinError(1, 0.);
    } } }

    // initialize pass nominal counter
    std::map<std::string,     // event selection
	std::map<std::string, // selection type
	long unsigned > > passNominalCounter;
    for(std::string event_selection: event_selections){
	for(std::string selection_type: selection_types){
	    passNominalCounter[event_selection][selection_type] = 0;
	}
    }

    // initialize pass nominal weight counter
    std::map<std::string,     // event selection
        std::map<std::string, // selection type
        double > > passNominalWeightCounter;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types){
            passNominalWeightCounter[event_selection][selection_type] = 0.;
        }
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    double nEntriesReweight = 1;
    if( nEntries!=0 && nEntries<numberOfEntries ){
	// loop over a smaller number of entries if samples are impractically large
	std::cout << "limiting number of entries to " << nEntries << std::endl;
	nEntriesReweight = (double)numberOfEntries/nEntries;
	std::cout << "(with corresponding reweighting factor " << nEntriesReweight << ")" << std::endl;
	numberOfEntries = nEntries;
    }
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// build the event
	Event event = treeReader.buildEvent(entry);

	// initialize map of variables
        std::map<std::string,double> varmap = eventFlatteningParticleLevel::initVarMap();

	// loop over event selections and selection types
	for( std::string event_selection: event_selections ){
	for( std::string selection_type: selection_types ){

	if( selection_type!="particlelevel" ){
	    std::string msg = "ERROR: found selection_type " + selection_type;
	    msg.append(" while only 'particlelevel' is supported for now.");
	}

        // do nominal selection and calculate particle-level event variables
        if( !eventSelectionsParticleLevel::passES(event, event_selection) ) continue;
	varmap = eventFlatteningParticleLevel::eventToEntry(event);
	double nominalWeight = event.genWeight()*nEntriesReweight;
	for(HistogramVariable histVar: histVars){
	    std::string variableName = histVar.name();
	    std::string variable = histVar.variable();
	    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at("nominal").get(),
				    varmap.at(variable), nominalWeight );
	}

	// fill the counters
	passNominalCounter.at(event_selection).at(selection_type) += 1;
	passNominalWeightCounter.at(event_selection).at(selection_type) += event.genWeight();

	// loop over systematics
	for(std::string systematic : systematics){
	    std::string upvar = systematic + "Up";
            std::string downvar = systematic + "Down";

	    if(systematic=="fScaleShape" && hasValidQcds){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
		double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();	    
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fScaleNorm" && hasValidQcds){
		double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();      
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fScaleTotal" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_0p5();
                for(HistogramVariable histVar: histVars){
                    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rScaleShape" && hasValidQcds){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScaleNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rScaleTotal" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_1();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_1();
                for(HistogramVariable histVar: histVars){
                    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rfScalesShape" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rfScalesNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rfScalesTotal" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5();
                for(HistogramVariable histVar: histVars){
                    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="isrShape" && hasValidPSs){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_2()
							    / xsecs.get()->crossSectionRatio_ISR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_0p5()
							    / xsecs.get()->crossSectionRatio_ISR_0p5();
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="isrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_0p5();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="isrTotal" && hasValidPSs){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_0p5();
                for(HistogramVariable histVar: histVars){
                    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="fsrShape" && hasValidPSs){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_2()
							    / xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_0p5()
							    / xsecs.get()->crossSectionRatio_FSR_0p5();
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
		    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fsrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_0p5();
                for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					  varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
					  varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="fsrTotal" && hasValidPSs){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_0p5();
                for(HistogramVariable histVar: histVars){
                    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    // run over qcd scale variations to calculate envolope later
	    else if(systematic=="qcdScalesShapeVar" && hasValidQcds){
                std::vector<double> qcdvariations;
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_1()
					    / xsecs.get()->crossSectionRatio_MuR_2_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_1()
					    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_2()
					    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_2()
					    / xsecs.get()->crossSectionRatio_MuR_1_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_0p5()
					    / xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
					    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5() );
		for(unsigned i=0; i<qcdvariations.size(); ++i){
                    std::string temp = systematic + std::to_string(i);
		    double reweight = qcdvariations[i];
                    double qcdweight = nominalWeight * reweight;
		    for(HistogramVariable histVar: histVars){
			std::string variableName = histVar.name();
			std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(temp).get(),
					      varmap.at(variable), qcdweight);
		    }
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="qcdScalesTotalVar" && hasValidQcds){
		std::vector<double> qcdvariations;
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_0p5() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5() );
                for(unsigned i=0; i<qcdvariations.size(); ++i){
                    std::string temp = systematic + std::to_string(i);
                    double reweight = qcdvariations[i];
                    double qcdweight = nominalWeight * reweight;
                    for(HistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(temp).get(),
                                              varmap.at(variable), qcdweight);
                    }
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="qcdScalesNorm" && hasValidQcds){
		double upWeight = nominalWeight*qcdScalesMaxXSecRatio;
                double downWeight = nominalWeight*qcdScalesMinXSecRatio;
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // run over pdf variations to calculate envelope later
	    else if(systematic=="pdfShapeVar" && hasValidPdfs){
		for(unsigned i=0; i<numberOfPdfVariations; ++i){
                    std::string temp = systematic + std::to_string(i);
		    /*std::cout << "--------" << std::endl;
		    std::cout << event.generatorInfo().relativeWeightPdfVar(i) << std::endl;
		    std::cout << xsecs.get()->crossSectionRatio_pdfVar(i) << std::endl;
		    std::cout << event.generatorInfo().relativeWeightPdfVar(i)/xsecs.get()->crossSectionRatio_pdfVar(i) << std::endl;*/
		    double reweight = event.generatorInfo().relativeWeightPdfVar(i)
                                        / xsecs.get()->crossSectionRatio_pdfVar(i);
                    double pdfweight = nominalWeight * reweight;
		    for(HistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(temp).get(),
                                              varmap.at(variable), pdfweight);
                    }		    
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // run over pdf variations to calculate envelope later
            else if(systematic=="pdfTotalVar" && hasValidPdfs){
                for(unsigned i=0; i<numberOfPdfVariations; ++i){
                    std::string temp = systematic + std::to_string(i);
                    double reweight = event.generatorInfo().relativeWeightPdfVar(i);
                    double pdfweight = nominalWeight * reweight;
                    for(HistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(temp).get(),
                                              varmap.at(variable), pdfweight);
                    }
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="pdfNorm" && hasValidPdfs){
		double upWeight = nominalWeight*pdfMaxXSecRatio;
                double downWeight = nominalWeight*pdfMinXSecRatio;
		for(HistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string variable = histVar.variable();
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
                                          varmap.at(variable), upWeight);
                    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                          varmap.at(variable), downWeight);   
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
            else{
		std::string msg = "ERROR: unrecognized systematic: " + systematic;
		throw std::runtime_error(msg);
	    }
        } // end loop over systematics
	} } // end loop over event selections and selection types
    } // end loop over events

    // print number of events passing nominal selection
    std::cout << "number of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types){
            std::cout << "  - " << event_selection << " " << selection_type;
	    unsigned long nentries = passNominalCounter.at(event_selection).at(selection_type);
	    double nentriesScaled = ((double)nentries)*nEntriesReweight;
	    double nentriesRatio = nentriesScaled/hCounterEntries;
	    std::cout << " " << nentries << std::endl;
	    std::cout << "    (scaled to total sample: " << nentriesScaled;
	    std::cout << "    = " << nentriesRatio << " of hCounter entries)." << std::endl;
        }
    }

    // print sum of weights of events passing nominal selection
    // and set as bin content 
    std::cout << "sum of weights of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types){
            std::cout << "  - " << event_selection << " " << selection_type;
	    double weights = passNominalWeightCounter.at(event_selection).at(selection_type);
            double weightsScaled = weights*nEntriesReweight;
	    double weightsRatio = weightsScaled/hCounterWeights;
            std::cout << " " << weights << std::endl;
	    std::cout << "    (scaled to total sample: " << weightsScaled;
	    std::cout << "    = " << weightsRatio << " of hCounter weights)." << std::endl;
        }
    }

    // make envelopes and/or RMS for systematics where this is needed
    for(std::string event_selection: event_selections){
    for(std::string selection_type: selection_types){
	for( std::string systematic : systematics ){
	    if(systematic=="pdfShapeVar" && hasValidPdfs){
		// do rms
		std::string upvar = "pdfShapeRMSUp";
		std::string downvar = "pdfShapeRMSDown";
		for(HistogramVariable histVar: histVars){
		    systematicTools::fillRMS( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
					      upvar, downvar, "pdfShapeVar"); 
		}
	    }
            if(systematic=="pdfTotalVar" && hasValidPdfs){
		// do rms
                std::string upvar = "pdfTotalRMSUp";
                std::string downvar = "pdfTotalRMSDown";
                for(HistogramVariable histVar: histVars){
                    systematicTools::fillRMS( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
                                              upvar, downvar, "pdfTotalVar");
                }
	    }
	    else if(systematic=="qcdScalesShapeVar"){
		std::string upvar = "qcdScalesShapeEnvUp";
		std::string downvar = "qcdScalesShapeEnvDown";
		for(HistogramVariable histVar: histVars){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    std::shared_ptr<TH1D> nominalHist = histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at("nominal");
		    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(upvar)->SetBinContent(i,nominalHist->GetBinContent(i));
			histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(downvar)->SetBinContent(i,nominalHist->GetBinContent(i));
		    }
		    // then fill envelope in case valid qcd variations are present
		    if(hasValidQcds){ 
			systematicTools::fillEnvelope( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
						       upvar, downvar, "qcdScalesShapeVar"); 
		    }
		}
	    }
	    else if(systematic=="qcdScalesTotalVar"){
                std::string upvar = "qcdScalesTotalEnvUp";
                std::string downvar = "qcdScalesTotalEnvDown";
                for(HistogramVariable histVar: histVars){
                    // first initialize the up and down variations to be equal to nominal
                    // (needed for correct envelope computation)
                    std::shared_ptr<TH1D> nominalHist = histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at("nominal");
                    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
                        histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(upvar)->SetBinContent(i,nominalHist->GetBinContent(i));
                        histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(downvar)->SetBinContent(i,nominalHist->GetBinContent(i));
                    }
                    // then fill envelope in case valid qcd variations are present
                    if(hasValidQcds){ 
                        systematicTools::fillEnvelope( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
                                                       upvar, downvar, "qcdScalesTotalVar");
                    }   
                }
            }
	} // end loop over systematics to fill envelope and/or RMS
    } } // end loop over event selections and selection types to fill envelope and/or RMS

    // save histograms to the output file
    for(std::string event_selection: event_selections){
    for(std::string selection_type: selection_types){
	// make output ROOT file
	std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
	outputFilePath += event_selection + "/" + selection_type + "/";
        systemTools::makeDirectory(outputFilePath);
	outputFilePath += inputFileName;
	TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
	// loop over variables
	for(HistogramVariable histVar: histVars){
	    std::string variableName = histVar.name();
	    bool storeLheVars = false;
	    bool doClip = true;
	    // first find nominal histogram for this variable
	    std::shared_ptr<TH1D> nominalhist = histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at("nominal");
	    // if nominal histogram is empty, fill with dummy value (needed for combine);
	    // also put all other histograms equal to the nominal one to avoid huge uncertainties
	    bool nominalIsEmpty = false;
	    if( nominalhist->GetEntries()==0 ){
		std::string msg = std::string("WARNING: nominal histogram ") + nominalhist->GetName();
		msg.append(" is empty (i.e. GetEntries() returned 0).");
		msg.append(" No events from this sample seem to have passed the selections.");
		msg.append(" Will fill this histogram with a small value in first bin (needed for combine).");
		std::cout << msg << std::endl;
		nominalIsEmpty = true;
		nominalhist->SetBinContent(1,1e-6); 
		for(auto mapelement : histMap.at(processName).at(event_selection).at(selection_type).at(variableName)){
		    if(stringTools::stringContains(mapelement.first,"nominal")) continue;
		    histogram::copyHistogram(mapelement.second,nominalhist);
		}
	    }
	    // clip and write nominal histogram
	    if( doClip ) histogram::clipHistogram(nominalhist.get());
	    nominalhist->Write();
	    // loop over rest of histograms
	    for(auto mapelement : histMap.at(processName).at(event_selection).at(selection_type).at(variableName)){
		if( stringTools::stringContains(mapelement.first,"nominal")) continue;
		std::shared_ptr<TH1D> hist = mapelement.second;
		// selection: do not store all individual pdf variations
		if(stringTools::stringContains(hist->GetName(),"pdfShapeVar") 
		   && !storeLheVars) continue;
		if(stringTools::stringContains(hist->GetName(),"pdfTotalVar")
                   && !storeLheVars) continue;
		// selection: do not store all individual qcd scale variations
		if(stringTools::stringContains(hist->GetName(),"qcdScalesShapeVar")
		   && !storeLheVars) continue;
		if(stringTools::stringContains(hist->GetName(),"qcdScalesTotalVar")
                   && !storeLheVars) continue;
		// below are special treatments of empty histograms,
		// but the case where the nominal histogram was empty was already handled above,
		// and need not be considered here again.
		if( !nominalIsEmpty ){
		    // special treatment of histograms with zero entries or all weights zero
		    if(hist->GetEntries()==0 or std::abs(hist->GetSumOfWeights())<1e-12){
			// use nominal histogram instead
			std::string msg = std::string("WARNING: systematic histogram ") + hist->GetName();
			msg.append(" contains 0 entries or all weights are 0");
			msg.append(" (while nominal histogram was not empty).");
			msg.append(" Either 0 events have passed the varied selections");
			msg.append(" or systematic is not present in sample or all weights are 0.");
			msg.append(" Will use nominal histogram instead.");
			std::cout << msg << std::endl;
			histogram::copyHistogram(hist,nominalhist);
		    }
		    // special histograms with zero mean and std, even if they have 'entries' (e.g. pdfEnv)
		    else if(std::abs(hist->GetMean())<1e-12 
			    && std::abs(hist->GetStdDev())<1e-12){ 
			std::string msg = std::string("WARNING: systematic histogram ") + hist->GetName();
			msg.append(" has 0 mean and standard deviation");
			msg.append(" (while nominal histogram was not empty).");
			msg.append(" Will use nominal histogram instead.");
			std::cout << msg << std::endl;
			histogram::copyHistogram(hist,nominalhist);
		    }
		}
		// clip histograms to minimum zero
		if( doClip ) histogram::clipHistogram(hist.get());
		// save histograms
		hist->Write();
	    }
	} // end loop over variables for writing histograms
	outputFilePtr->Close();
    } } // end loop over event selections and selection types for writing histograms
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 9;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: runanalysis.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input_directory" << std::endl;
	std::cerr << "sample_list" << std::endl;
	std::cerr << "sample_index" << std::endl;
	std::cerr << "output_directory" << std::endl;
	std::cerr << "variable_file" << std::endl;
	std::cerr << "event_selection" << std::endl;
	std::cerr << "selection_type" << std::endl;
	std::cerr << "nevents" << std::endl;
	std::cerr << "systematics (comma-separated list)" << std::endl;
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
    std::string& selection_type = argvStr[7];
    std::vector<std::string> selection_types = stringTools::split(selection_type,",");
    unsigned long nevents = std::stoul(argvStr[8]);
    std::string& systematicstr = argvStr[9];
    std::vector<std::string> systematics;
    if( systematicstr!="none" ){
	systematics = stringTools::split(systematicstr,",");
    }

    // print arguments
    std::cout << "Found following arguments:" << std::endl;
    std::cout << "  - input directory: " << input_directory << std::endl;
    std::cout << "  - sample list: " << sample_list << std::endl;
    std::cout << "  - sample index: " << std::to_string(sample_index) << std::endl;
    std::cout << "  - output_directory: " << output_directory << std::endl;
    std::cout << "  - variable file: " << variable_file << std::endl;
    std::cout << "  - event selection: " << event_selection << std::endl;
    std::cout << "  - selection type: " << selection_type << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;
    std::cout << "  - systematics:" << std::endl;
    for( std::string systematic: systematics ) std::cout << "      " << systematic << std::endl;

    // read variables
    std::vector<HistogramVariable> histVars = variableTools::readVariables( variable_file );
    /*std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( HistogramVariable var: histVars ){
        std::cout << var.toString() << std::endl;       
    }*/

    // check variables
    std::map<std::string,double> emptymap = eventFlatteningParticleLevel::initVarMap();
    for(HistogramVariable histVar: histVars){
        std::string variable = histVar.variable();
        if( emptymap.find(variable)==emptymap.end() ){
            std::string msg = "ERROR: variable '"+variable+"' not recognized.";
            throw std::runtime_error(msg);
        }
    }

    // fill the histograms
    fillTheoryHistograms( input_directory, sample_list, sample_index, output_directory,
			       histVars, event_selections, selection_types, 
                               nevents, systematics );

    std::cerr << "###done###" << std::endl;
    return 0;
}
