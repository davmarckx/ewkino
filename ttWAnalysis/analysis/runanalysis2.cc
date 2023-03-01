// This is the main C++ class used to run the analysis,
// i.e. produce distributions + systematic variations as input for combine.
// Modification with respect to runanalysis.cc: use DoubleHistogramVariables
// in order to make distributions suitable for maximum likelihood unfolding.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include specific tools
#include "interface/systematicTools.h"

// include other parts of the framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ReweighterBTagShape.h"

// include analysis tools
#include "../btagging/interface/bTaggingTools.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"


std::string modProcessName( const std::string processName, const std::string selectionType ){
    // small internal helper function to modify the process name.
    // the process name is changed to
    // - "nonprompt" for selection type "fakerate"
    // - "chargeflips" for selection type "chargeflips"
    // - else left unmodified.
    std::string modProcessName = processName;
    if( selectionType=="fakerate" ) modProcessName = "nonprompt";
    if( selectionType=="chargeflips" ) modProcessName = "chargeflips";
    return modProcessName;
}

std::vector<std::string> splitProcessNames( 
    const std::string& processName,
    const DoubleHistogramVariable& histVar,
    const bool doSplitParticleLevel ){
    // make process names split on particle level (if requested).
    // the following process names are made (starting from TTW as example):
    // TTW0, TTW1, ..., TTWn where n is the number of bins.
    // note that TTW0 is intended for events that do not pass particle level selection.
    // note: if the process name is "nonprompt" or "chargeflips",
    //       the doSplitParticleLevel bool is bypassed to be always false
    //       (as nonprompt and chargeflips are taken from data).
    std::vector<std::string> splitProcessNames;
    if( !doSplitParticleLevel 
	|| processName=="nonprompt"
	|| processName=="chargeflips" ){ 
	splitProcessNames = {processName}; 
    }
    else{
	unsigned int nSecondaryBins = histVar.nSecondaryBins();
        for(unsigned int i=0; i<=nSecondaryBins; i++){
	    splitProcessNames.push_back( processName+std::to_string(i) );
        }
    }
    return splitProcessNames;
}

void fillHistogram( 
    std::shared_ptr<TH1D> hist, 
    const DoubleHistogramVariable& variable, 
    double primaryValue, double secondaryValue,
    double weight ){
    // small helper function to fill a given histogram
    // with the primary and secondary value of a double variable.
    int binNb = variable.findBinNumber( primaryValue, secondaryValue );
    // printouts for testing
    //std::cout << primaryValue << " " << secondaryValue << " " << binNb << std::endl;
    // fill the histogram
    hist->Fill( binNb, weight );
}

std::shared_ptr<TH1D> findHistogramToFill(
    const std::map< std::string, // process
	    std::map< std::string, // event selection
	    std::map< std::string, // selection type
	    std::map< std::string, // variable
	    std::map< std::string, // systematic
	    std::shared_ptr<TH1D> > > > > >& histMap,
    const std::string& processName,
    const std::string& eventSelection,
    const std::string& selectionType,
    const std::string& systematic,
    const DoubleHistogramVariable& variable,
    bool doSplitParticleLevel, bool passParticleLevel,
    double secondaryValueParticleLevel ){
    // small helper function to find the correct histogram to fill in the map.
    // for the case where no split in particle level is made, this is straightforward;
    // but in case of split, need to find the correctly modified process name.
    // note: if the process name is "nonprompt" or "chargeflips",
    //       the doSplitParticleLevel bool is bypassed to be always false
    //       (as nonprompt and chargeflips are taken from data).
    std::string variableName = variable.name();
    std::string thisProcessName = processName;
    if( doSplitParticleLevel && processName!="nonprompt" && processName!="chargeflips" ){
	// if event does not pass particle level, use appendix 0
	if( !passParticleLevel ){ thisProcessName = processName + "0"; }
	// else use appendix bin number
	else{
	    int secondaryBinNumber = variable.findSecondaryBinNumber( secondaryValueParticleLevel );
	    thisProcessName = processName + std::to_string(secondaryBinNumber);
	}
    }
    // find the histogram in the map
    try{
	return histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(systematic);
    }
    catch(...){
	std::string msg = "ERROR in findHistogramToFill: histogram not found in the map!";
	msg.append(" Error occurred for following key:\n");
	msg.append("  - process name: "+thisProcessName+"\n");
	msg.append("  - event selection: "+eventSelection+"\n");
	msg.append("  - selection type: "+selectionType+"\n");
	msg.append("  - variable name: "+variableName+"\n");
	msg.append("  - systematic: "+systematic+"\n");
	throw std::runtime_error(msg);
    }
}

void fillHistogram(
    const std::map< std::string, // process
            std::map< std::string, // event selection
            std::map< std::string, // selection type
            std::map< std::string, // variable
            std::map< std::string, // systematic
            std::shared_ptr<TH1D> > > > > >& histMap,
    const std::string& processName,
    const std::string& eventSelection,
    const std::string& selectionType,
    const std::string& systematic,
    const DoubleHistogramVariable& variable,
    double primaryValue, double secondaryValue, double weight,
    bool doSplitParticleLevel, bool passParticleLevel,
    double secondaryValueParticleLevel){
    // small helper function to find correct histogram in the map and fill it.
    std::shared_ptr<TH1D> histogramToFill = findHistogramToFill( 
	histMap, processName, eventSelection, selectionType, systematic,
	variable, doSplitParticleLevel, passParticleLevel, secondaryValueParticleLevel );
    fillHistogram( histogramToFill, variable, primaryValue, secondaryValue, weight);
}


std::map< std::string,     // process
    std::map< std::string, // event selection
    std::map< std::string, // selection type
    std::map< std::string, // variable
    std::map< std::string, // systematic
    std::shared_ptr<TH1D> > > > > > initHistMap(
    const std::vector<std::string>& processNames,
    const bool isData,
    const bool doSplitParticleLevel,
    const std::vector<std::string>& eventSelections,
    const std::vector<std::string>& selectionTypes,
    const std::vector<DoubleHistogramVariable>& histVars,
    const std::vector<std::string>& systematics,
    unsigned numberOfPdfVariations=100,
    unsigned numberOfQcdScaleVariations=6,
    const std::vector<std::string> allJecVariations={},
    const std::vector<std::string> groupedJecVariations={},
    const std::vector<std::string> bTagShapeSystematics={} ){
    // make map of histograms
    // the resulting map has five levels: 
    // map[process name][event selection][selection type][variable name][systematic]
    // notes:
    // - processNames should be just one name (per file) in most cases,
    //   but can be multiple if a sample is split in sub-categories.
    //   note: for splitting at particle level, still use one name, 
    //         and set doSplitParticleLevel to true (see below)!
    // - isData should be true for data samples and false for MC samples;
    //   systematics are skipped for data except for fake rate selection type.
    // - doSplitParticleLevel will split the processNames into bins of secondary variable;
    //   the resulting split process names are formatted as follows: TTW -> TTW0, TTW1, ...
    
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
	    std::string modifiedProcessName = modProcessName(processName,selectionType);
	    // loop over variables
	    for(DoubleHistogramVariable histVar: histVars){
		std::string variableName = histVar.name();
                // make process names split on particle level (if needed)
		std::vector<std::string> splittedProcessNames = splitProcessNames(
		    modifiedProcessName, histVar, doSplitParticleLevel);
		// loop over particle level split process names
		for( std::string thisProcessName: splittedProcessNames ){
		    // form the histogram name
		    std::string baseName = thisProcessName+"_"+eventSelection+"_"+selectionType+"_"+variableName;
		    baseName = stringTools::removeOccurencesOf(baseName," ");
		    baseName = stringTools::removeOccurencesOf(baseName,"/");
		    baseName = stringTools::removeOccurencesOf(baseName,"#");
		    baseName = stringTools::removeOccurencesOf(baseName,"{");
		    baseName = stringTools::removeOccurencesOf(baseName,"}");
		    baseName = stringTools::removeOccurencesOf(baseName,"+");
		    // add nominal histogram
		    histMap[thisProcessName][eventSelection][selectionType][variableName]["nominal"] = histVar.initializeHistogram( baseName+"_nominal" );
		    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at("nominal")->SetTitle(thisProcessName.c_str());
		    // for data, skip initialization of histograms for systematics
                    // (except for selection type "fakerate", in which case systematic histograms
                    // should be added and filled with nominal values,
                    // in order to correctly merge with MC with the same selection type).
		    if( isData && selectionType!="fakerate" ) continue;
                    // for MC, specifically for selection type "chargeflips",
                    // skip initialization of systematic histograms as well,
                    // in order to correctly merge with data with the same selection type.
                    if( selectionType=="chargeflips" ) continue;
		    // loop over systematics
		    for(std::string systematic : systematics){
			// special case for PDF variations: store individual variations 
			// as well as envelope and rms
			if(systematic=="pdfShapeVar"){
			    for(unsigned i=0; i<numberOfPdfVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			    std::vector<std::string> temps = {"pdfShapeEnvUp", "pdfShapeEnvDown",
							      "pdfShapeRMSUp", "pdfShapeRMSDown"};
			    for(std::string temp: temps){
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for QCD scales: store individual variations
			// as well as envelope
			else if(systematic=="qcdScalesShapeVar"){
			    for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			    std::vector<std::string> temps = {"qcdScalesShapeEnvUp", "qcdScalesShapeEnvDown"};
			    for(std::string temp: temps){
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for split JEC variations: store all variations
			else if(systematic=="JECAll" || systematic=="JECGrouped"){
			    std::vector<std::string> variations = allJecVariations;
			    if( systematic=="JECGrouped" ) variations = groupedJecVariations;
			    for(std::string jecvar: variations){
				std::string temp = systematic + "_" + jecvar + "Up";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
				temp = systematic + "_" + jecvar + "Down";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for bTag shape variations: store multiple systematics
			else if(systematic=="bTag_shape"){
			    for(std::string btagsys: bTagShapeSystematics){
				std::string temp = systematic + "_" + btagsys + "Up";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
				temp = systematic + "_" + btagsys + "Down";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// now general case: store up and down variation
			else{
			    std::string temp = systematic + "Up";
			    histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
			    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    temp = systematic + "Down";
			    histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar.initializeHistogram( baseName+"_"+temp );
			    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			}
		    } // end loop over systematics
		} // end loop over split process names
	    } // end loop over variables
    } } } // end loop over process names, event selections and selection types
    return histMap;
}


void fillSystematicsHistograms(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<DoubleHistogramVariable>& histVars, 
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
            const std::string& muonFRMap, 
	    const std::string& electronFRMap,
            const std::string& electronCFMap,
	    unsigned long nEntries,
            bool forceNEntries,
	    bool doSplitParticleLevel,
	    const std::string& bdtWeightsFile,
	    double bdtCutValue,
            std::vector<std::string>& systematics ){
    // initialize TreeReader from input file
    std::cout << "=== start function fillSystematicsHistograms ===" << std::endl;;
    std::cout << "initializing TreeReader and setting to sample n. " << sampleIndex << "..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(unsigned int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"fakerate")!=selection_types.end()){
	std::cout << "reading fake rate maps..." << std::endl;
        frmap_muon = readFakeRateTools::readFRMap(muonFRMap, "muon", year);
        frmap_electron = readFakeRateTools::readFRMap(electronFRMap, "electron", year);
    }

    // load charge flip maps if needed
    std::shared_ptr<TH2D> cfmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"chargeflips")!=selection_types.end()){
        std::cout << "reading charge flip maps..." << std::endl;
	cfmap_electron = readChargeFlipTools::readChargeFlipMap(
			    electronCFMap, year, "electron");
    }

    // make reweighter
    std::cout << "initializing Reweighter..." << std::endl;;
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    // FOR TESTING //
    //reweighterFactory = std::shared_ptr<ReweighterFactory>( new EmptyReweighterFactory() );
    // FOR REAL //
    reweighterFactory = std::shared_ptr<ReweighterFactory>( new Run2ULReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    
    // initialize the b-tagging shape reweighter if needed
    bool hasBTagShapeReweighter = reweighter.hasReweighter("bTag_shape");
    std::vector<std::string> bTagShapeSystematics;
    std::map< std::string, std::map< std::string, std::map< int, double >>> bTagWeightMap;
    if( hasBTagShapeReweighter and !treeReader.isData() ){
        // find available b-tagging systematics
        std::cout << "finding available b-tagging systematics..." << std::endl;
        bTagShapeSystematics = dynamic_cast<const ReweighterBTagShape*>(
            reweighter["bTag_shape"] )->availableSystematics();
        std::cout << "found following b-tagging systematics:" << std::endl;
        for(std::string el: bTagShapeSystematics){
            std::cout << "  - " << el << std::endl;
        }
        // read b-tagging shape reweighting normalization factors from txt file
        std::string txtInputFile = "../btagging/output_20230215/";
        // (hard-coded for now, maybe replace by argument later)
        txtInputFile += year + "/" + inputFileName;
        if( !systemTools::fileExists(txtInputFile) ){
            std::string msg = "ERROR in loading b-tagging normalization factors:";
            msg += " file " + txtInputFile + " does not exist.";
            throw std::runtime_error(msg);
        }
        std::vector<std::string> variationsToRead = {"central"};
        for( std::string var: bTagShapeSystematics ){
            variationsToRead.push_back("up_"+var);
            variationsToRead.push_back("down_"+var);
        }
        bTagWeightMap = bTaggingTools::textToMap( txtInputFile, event_selections, variationsToRead );
    }   
 
    // determine global sample properties related to pdf and scale variations
    unsigned numberOfScaleVariations = 0;
    unsigned numberOfPdfVariations = 0;
    std::shared_ptr< SampleCrossSections > xsecs;
    std::vector< double > qcdScalesXSecRatios;
    double qcdScalesMinXSecRatio = 1.;
    double qcdScalesMaxXSecRatio = 1.;
    std::vector<double> pdfXSecRatios;
    double pdfMinXSecRatio = 1.;
    double pdfMaxXSecRatio = 1.;
    // check whether lhe systematics are needed
    bool considerlhe = false;
    for( std::string systematic : systematics){
	if( systematicTools::systematicType(systematic)=="lhe"
	    || systematicTools::systematicType(systematic)=="scale" ){ considerlhe=true; break; }
    }
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
	xsecs = std::make_shared<SampleCrossSections>( treeReader.currentSample() );
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
		pdfXSecRatios.push_back( xsecs.get()->crossSectionRatio_pdfVar(i) ); }
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
    bool considerps = false;
    for( std::string systematic : systematics){
        if( systematicTools::systematicType(systematic)=="ps" ){ considerps=true; break; }
    }
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
        xsecs = std::make_shared<SampleCrossSections>( treeReader.currentSample() );
        if(numberOfPSVariations==46){ hasValidPSs = true; }
        std::cout << "- hasValidPSs: " << hasValidPSs << std::endl;
    }

    // determine global sample properties related to split JEC variations
    std::vector<std::string> allJECVariations;
    std::vector<std::string> groupedJECVariations;
    // check which jec variations are needed
    bool considerjecall = false;
    bool considerjecgrouped = false;
    for( std::string systematic: systematics ){
	if( systematic=="JECAll") considerjecall = true;
	else if( systematic=="JECGrouped" ) considerjecgrouped = true;
    }
    if( treeReader.numberOfEntries()>0
	&& (considerjecall || considerjecgrouped)
        && !treeReader.isData() ){
	std::cout << "finding available JEC uncertainty sources..." << std::endl;
	Event event = treeReader.buildEvent(0,false,false,considerjecall,considerjecgrouped);
	allJECVariations = event.jetInfo().allJECVariations();
	groupedJECVariations = event.jetInfo().groupedJECVariations();
	std::cout << "found following JEC uncertainty sources:" << std::endl;
	event.jetInfo().printAllJECVariations();
	event.jetInfo().printGroupedJECVariations();
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
	processNames, treeReader.isData(), doSplitParticleLevel,
	event_selections, selection_types, histVars, systematics,
	numberOfPdfVariations, 6, 
	allJECVariations, groupedJECVariations,
	bTagShapeSystematics);

    // load the BDT
    // default value is nullptr, 
    // in which case the BDT will not be evaluated.
    std::shared_ptr<TMVA::Experimental::RBDT<>> bdt;
    if( bdtWeightsFile.size()!=0 ){
        std::cout << "reading BDT evaluator..." << std::endl;
        bdt = std::make_shared<TMVA::Experimental::RBDT<>>("XGB", bdtWeightsFile);
        std::cout << "successfully loaded BDT evaluator." << std::endl;
    }

    // initialize pass nominal counter
    std::map<std::string,     // event selection
	std::map<std::string, // selection type
	long unsigned > > passNominalCounter;
    for(std::string event_selection: event_selections){
	for(std::string selection_type: selection_types){
	    passNominalCounter[event_selection][selection_type] = 0;
	}
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    double nEntriesReweight = 1;
    if( nEntries!=0 && nEntries<numberOfEntries ){
	if( !treeReader.isData() || forceNEntries ){
	    // loop over a smaller number of entries if samples are impractically large
	    std::cout << "limiting number of entries to " << nEntries << std::endl;
	    nEntriesReweight = (double)numberOfEntries/nEntries;
	    std::cout << "(with corresponding reweighting factor " << nEntriesReweight << ")" << std::endl;
	    numberOfEntries = nEntries;
	}
    }
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
	// printouts for progress checking
        if(entry%1000 == 0){
	    if(entry < 10000 || (entry%10000 == 0)){
		std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl; }
	}

	// build the event
	Event event = treeReader.buildEvent(
                        entry,
                        false,false,
                        considerjecall,
                        considerjecgrouped,
                        doSplitParticleLevel );

	// initialize map of variables
        std::map<std::string,double> varmap = eventFlattening::initVarMap();
        // (store nominal event variables, call only once and use for all weight systematics)
        std::map<std::string,double> accvarmap = eventFlattening::initVarMap();
        // (store acceptance-modified event variables, can be overwritten per acceptance variation)
	std::map<std::string,double> varmapParticleLevel = eventFlatteningParticleLevel::initVarMap();
	// (store variables at particle level)

	// loop over event selections
	for( std::string event_selection: event_selections ){

	// calculate particle-level event variables
	bool passParticleLevel = false;
        if(doSplitParticleLevel){
            if( eventSelectionsParticleLevel::passES(event, event_selection) ){
		passParticleLevel = true;
		varmapParticleLevel = eventFlatteningParticleLevel::eventToEntry(event);
	    }
        }

	// set  the correct normalization factors for b-tag reweighting
        if( hasBTagShapeReweighter && !treeReader.isData() ){
            dynamic_cast<ReweighterBTagShape*>(
                reweighter.getReweighter("bTag_shape") )->setNormFactors( treeReader.currentSample(),
                bTagWeightMap[event_selection] );
        }

	// loop over selection types
	for( std::string selection_type: selection_types ){
	
	// modify the process name for some selection types
	std::string thisProcessName = modProcessName(processName,selection_type);
	
	// fill nominal histograms
	bool passnominal = true;
	double nominalWeight = 0;
        if(!passES(event, event_selection, selection_type, "nominal")) passnominal = false;
	if(passnominal){
	    varmap = eventFlattening::eventToEntry(event, 
			reweighter, selection_type, 
			frmap_muon, frmap_electron, cfmap_electron, "nominal",
                        bdt, year);
	    nominalWeight = varmap.at("_normweight")*nEntriesReweight;
	    if( bdtCutValue>-1 ){
                // do additional selection on final BDT score
                // note: to be 100% correct, this should be re-evaluated for every jec systematic,
                // but for now just do a cut-and-continue on nominal.
                if( varmap["_eventBDT"]<bdtCutValue ) continue;
            }
            passNominalCounter.at(event_selection).at(selection_type)++;
	    for(DoubleHistogramVariable histVar: histVars){
		std::string variableName = histVar.name();
		std::string primaryVariable = histVar.primaryVariable();
                std::string secondaryVariable = histVar.secondaryVariable();
		fillHistogram( histMap, thisProcessName, event_selection, selection_type, "nominal",
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), nominalWeight,
			       doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
		// for fake rates from data, need to create systematic histograms
		// (in order for the merging to work properly);
		// loop over all systematics and fill with nominal values
		if(event.isData() && selection_type=="fakerate"){
		    for(auto mapelement: histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName) ){
			if(stringTools::stringContains(mapelement.first,"nominal")) continue;
			fillHistogram( mapelement.second, histVar,
				       varmap.at(primaryVariable), varmap.at(secondaryVariable), nominalWeight );
		    }
		}
	    }
	}

	// stop further event processing in case of data
	if(event.isData()) continue;
        // also stop further event processing for MC in case of charge flip selection
        if(selection_type=="chargeflips") continue;

	// loop over systematics
	for(std::string systematic : systematics){
	    // determine type of systematic (acceptance or weight)
	    std::string sysType = systematicTools::systematicType(systematic);
	    if(sysType=="ignore" || sysType=="ERROR") continue;
	    std::string upvar = systematic + "Up";
	    std::string downvar = systematic + "Down";
	    
	    // IF type is acceptance, special event selections are needed.
	    if(sysType=="acceptance"){
		
		// do event selection and flattening with up variation
		bool passup = true;
		if(!passES(event, event_selection, selection_type, upvar)) passup = false;
		if(passup){
		    accvarmap = eventFlattening::eventToEntry(event, 
				    reweighter, selection_type, 
				    frmap_muon, frmap_electron, cfmap_electron, upvar,
                                    bdt, year);
		    double weight = accvarmap["_normweight"]*nEntriesReweight;
		    // for JEC: propagate into b-tag shape reweighting
		    /*if( systematic=="JEC" && considerbtagshape ){
			weight *= dynamic_cast<const ReweighterBTagShape*>( 
				    reweighter["bTag_shape"] )->weightJecVar( event, "JECUp" )
				    / reweighter["bTag_shape"]->weight( event );
		    }*/
		    // fill the variables
		    for(DoubleHistogramVariable histVar: histVars){
			std::string variableName = histVar.name();
                        std::string primaryVariable = histVar.primaryVariable();
                        std::string secondaryVariable = histVar.secondaryVariable();
			fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, accvarmap.at(primaryVariable), accvarmap.at(secondaryVariable), weight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
		    }
		}
		// and with down variation
		bool passdown = true;
		if(!passES(event, event_selection, selection_type, downvar)) passdown = false;
		if(passdown){
		    accvarmap = eventFlattening::eventToEntry(event, 
				    reweighter, selection_type, 
				    frmap_muon, frmap_electron, cfmap_electron, downvar,
                                    bdt, year);
		    double weight = accvarmap["_normweight"]*nEntriesReweight;
		    // for JEC: propagate into b-tag shape reweighting
                    /*if( systematic=="JEC" && considerbtagshape ){
                        weight *= dynamic_cast<const ReweighterBTagShape*>(
                                    reweighter["bTag_shape"] )->weightJecVar( event, "JECDown" )
                                    / reweighter["bTag_shape"]->weight( event );
                    }*/
                    // fill the variables
		    for(DoubleHistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string primaryVariable = histVar.primaryVariable();
                        std::string secondaryVariable = histVar.secondaryVariable();
			fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, accvarmap.at(primaryVariable), accvarmap.at(secondaryVariable), weight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    }
		}
		// skip checking other systematics as they are mutually exclusive
		continue;
	    }
	    // IF type is jecsplit, special event selections are needed as well
	    else if(sysType=="jecsplit"){
		std::vector<std::string> varvector;
		if(systematic=="JECAll") varvector = allJECVariations;
		else if(systematic=="JECGrouped") varvector = groupedJECVariations;
		for(std::string jecvar: varvector){
		    std::string thisupvar = jecvar+"Up";
		    std::string thisdownvar = jecvar+"Down";
		    // do event selection and flattening with up variation
		    bool passup = true;
		    if(!passES(event, event_selection, selection_type, thisupvar)) passup = false;
		    if(passup){
			accvarmap = eventFlattening::eventToEntry(event,
				    reweighter, selection_type, 
				    frmap_muon, frmap_electron, 
				    cfmap_electron, thisupvar,
                                    bdt, year);
			double weight = accvarmap["_normweight"]*nEntriesReweight;
			// for JEC: propagate into b-tag shape reweighting
			/*if( considerbtagshape && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
			    if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar) ){
				weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightJecVar( event, thisupvar )
					/reweighter["bTag_shape"]->weight( event );
			    } else{
				std::cerr << "WARNING: variation '"<<jecvar<<"' for bTag shape";
				std::cerr << "reweighter but not recognized" << std::endl;
			    }
			}*/
			// fill the variables
			for(DoubleHistogramVariable histVar: histVars){
			    std::string variableName = histVar.name();
                            std::string primaryVariable = histVar.primaryVariable();
                            std::string secondaryVariable = histVar.secondaryVariable();
			    fillHistogram( histMap, thisProcessName, event_selection, selection_type, systematic+"_"+thisupvar,
                               histVar, accvarmap.at(primaryVariable), accvarmap.at(secondaryVariable), weight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
			}
		    }
		    // and with down variation
		    bool passdown = true;
		    if(!passES(event, event_selection, selection_type, thisdownvar)) passdown=false;
		    if(passdown){
			accvarmap = eventFlattening::eventToEntry(event,
				reweighter, selection_type, 
				frmap_muon, frmap_electron, 
				cfmap_electron, thisdownvar,
                                bdt, year);
			double weight = accvarmap["_normweight"]*nEntriesReweight;
                        // for JEC: propagate into b-tag shape reweighting
                        /*if( considerbtagshape && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
                            if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar) ){
                                weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightJecVar( event,thisdownvar )
                                        /reweighter["bTag_shape"]->weight( event );
                            } else{
                                std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
                                std::cerr << "reweighter but not recognized" << std::endl;
                            }
                        }*/
			// fill the variables
			for(DoubleHistogramVariable histVar: histVars){
			    std::string variableName = histVar.name();
                            std::string primaryVariable = histVar.primaryVariable();
                            std::string secondaryVariable = histVar.secondaryVariable();
			    fillHistogram( histMap, thisProcessName, event_selection, selection_type, systematic+"_"+thisdownvar,
                               histVar, accvarmap.at(primaryVariable), accvarmap.at(secondaryVariable), weight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
			}
		    }
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }

	    // ELSE do nominal event selection
	    else if(!passnominal) continue;
	    // (nominal event already stored in varmap)
	    
	    // IF type is weight, apply reweighter with up and down weight
	    if(sysType=="weight"){
		double reWeight = reweighter.singleWeight(event, systematic);
		// skip events for which the reweighting factor is zero
		if( reWeight<1e-12 ) continue;
                double upWeight = nominalWeight / reWeight * reweighter.singleWeightUp(event, systematic);
		double downWeight = nominalWeight / reWeight * reweighter.singleWeightDown(event, systematic);
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // special case for bTag_shape reweighting (several systematics)
	    else if(systematic=="bTag_shape"){
		double nombweight = reweighter["bTag_shape"]->weight( event );
                for(std::string btagsys: bTagShapeSystematics){
                    double upWeight = varmap["_normweight"]*nEntriesReweight / nombweight
                                        * dynamic_cast<const ReweighterBTagShape*>(
                                            reweighter["bTag_shape"])->weightUp( event, btagsys );
                    double downWeight = varmap["_normweight"]*nEntriesReweight / nombweight
                                        * dynamic_cast<const ReweighterBTagShape*>(
                                            reweighter["bTag_shape"])->weightDown( event, btagsys );
                    for(DoubleHistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string primaryVariable = histVar.primaryVariable();
			std::string secondaryVariable = histVar.secondaryVariable();
                        fillHistogram( histMap, thisProcessName, event_selection, selection_type, systematic+"_"+btagsys+"Up",
                                    histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
				    doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                        fillHistogram( histMap, thisProcessName, event_selection, selection_type, systematic+"_"+btagsys+"Down",
                                    histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                                    doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    }
                }
	    }
	    // ELSE apply nominal weight (already in variable nominalWeight)
	    else if(systematic=="fScale" && hasValidQcds){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
		double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_1_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
		// prints for testing:
		/*std::cout << "nominal weight: " << nominalWeight << std::endl;
		std::cout << "fScale:" << std::endl;
		std::cout << "  relative upweight: ";
		std::cout << event.generatorInfo().relativeWeight_MuR_1_MuF_2() << std::endl;
		std::cout << "  relative downweight ";
		std::cout << event.generatorInfo().relativeWeight_MuR_1_MuF_0p5() << std::endl;*/
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fScaleNorm" && hasValidQcds){
		double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScale" && hasValidQcds){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
		// prints for testing:
		/*std::cout << "nominal weight: " << nominalWeight << std::endl;
		std::cout << "rScale:" << std::endl;
                std::cout << "  relative upweight: ";
		std::cout << event.generatorInfo().relativeWeight_MuR_2_MuF_1() << std::endl;
                std::cout << "  relative downweight ";
		std::cout << event.generatorInfo().relativeWeight_MuR_0p5_MuF_1() << std::endl;*/
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScaleNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rfScales" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rfScalesNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="isrShape" && hasValidPSs){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_2()
							    / xsecs.get()->crossSectionRatio_ISR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_ISR_0p5()
							    / xsecs.get()->crossSectionRatio_ISR_0p5();
		// prints for testing:
		/*std::cout << "--- event ---" << std::endl;
		std::cout << nominalWeight << std::endl;
		std::cout << upweight << std::endl;
		std::cout << downweight << std::endl;*/
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="isrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_0p5();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="fsrShape" && hasValidPSs){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_2()
							    / xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_0p5()
							    / xsecs.get()->crossSectionRatio_FSR_0p5();
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fsrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_0p5();
                for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
                    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }

	    // apply electron reco uncertainty
	    else if(systematic=="electronReco"){
		double upWeight = nominalWeight / reweighter.singleWeight(event,"electronReco_pTBelow20")
                                                  * reweighter.singleWeightUp(event,"electronReco_pTBelow20")
                                                  / reweighter.singleWeight(event,"electronReco_pTAbove20")
                                                  * reweighter.singleWeightUp(event,"electronReco_pTAbove20");
		double downWeight = nominalWeight / reweighter.singleWeight(event,"electronReco_pTBelow20")
                                                  * reweighter.singleWeightDown(event,"electronReco_pTBelow20")
                                                  / reweighter.singleWeight(event,"electronReco_pTAbove20")
                                                  * reweighter.singleWeightDown(event,"electronReco_pTAbove20");
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
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
		    for(DoubleHistogramVariable histVar: histVars){
			std::string variableName = histVar.name();
		        std::string primaryVariable = histVar.primaryVariable();
                        std::string secondaryVariable = histVar.secondaryVariable();
			fillHistogram( histMap, thisProcessName, event_selection, selection_type, temp,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), qcdweight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    }
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="qcdScalesNorm" && hasValidQcds){
		double upWeight = nominalWeight*qcdScalesMaxXSecRatio;
                double downWeight = nominalWeight*qcdScalesMinXSecRatio;
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // run over pdf variations to calculate envelope later
	    else if(systematic=="pdfShapeVar" && hasValidPdfs){
		for(unsigned i=0; i<numberOfPdfVariations; ++i){
                    std::string temp = systematic + std::to_string(i);
		    double reweight = event.generatorInfo().relativeWeightPdfVar(i)
                                        / xsecs.get()->crossSectionRatio_pdfVar(i);
                    double pdfweight = nominalWeight * reweight;
		    for(DoubleHistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string primaryVariable = histVar.primaryVariable();
                        std::string secondaryVariable = histVar.secondaryVariable();
			fillHistogram( histMap, thisProcessName, event_selection, selection_type, temp,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), pdfweight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    }		    

		    // printouts for testing: 
                    /*double relweight = event.generatorInfo().relativeWeightPdfVar(i);
                    double xsecrat = xsecs.get()->crossSectionRatio_pdfVar(i);
		    double reweight = relweight / xsecrat;
                    if(reweight<0){
                        std::cout << "event: " << event.eventNumber() << ", ";
                        std::cout << "variation: " << i << ": ";
                        std::cout << "relative weight: " << relweight << ", ";
			std::cout << "xsec ratio: " << xsecrat << std::endl;
                    }*/
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="pdfNorm" && hasValidPdfs){
		double upWeight = nominalWeight*pdfMaxXSecRatio;
                double downWeight = nominalWeight*pdfMinXSecRatio;
		for(DoubleHistogramVariable histVar: histVars){
		    std::string variableName = histVar.name();
		    std::string primaryVariable = histVar.primaryVariable();
                    std::string secondaryVariable = histVar.secondaryVariable();
		    fillHistogram( histMap, thisProcessName, event_selection, selection_type, upvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), upWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                    fillHistogram( histMap, thisProcessName, event_selection, selection_type, downvar,
                               histVar, varmap.at(primaryVariable), varmap.at(secondaryVariable), downWeight,
                               doSplitParticleLevel, passParticleLevel, varmapParticleLevel.at(secondaryVariable) );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
        } // end loop over systematics
	} } // end loop over event selections and selection types
    } // end loop over events

    // print number of events passing nominal selection
    std::cout << "number of events passing nominal selection: " << std::endl;
    for(std::string event_selection: event_selections){
        for(std::string selection_type: selection_types){
            std::cout << "  - " << event_selection << " " << selection_type;
	    std::cout << " " << passNominalCounter.at(event_selection).at(selection_type) << std::endl;;
        }
    }

    // make envelopes and/or RMS for systematics where this is needed
    for(std::string es: event_selections){
    for(std::string st: selection_types){
	// skip data (except for fake rate selection) and charge flips
	if( treeReader.isData() && st!="fakerate" ) continue;
        if( st=="chargeflips" ) continue;
	// modify the process name for some selection types
        std::string modifiedProcessName = modProcessName(processName, st);
	for( std::string systematic : systematics ){
	    if(systematic=="pdfShapeVar"){
		// do envelope
		std::string upvar = "pdfShapeEnvUp";
		std::string downvar = "pdfShapeEnvDown";
		for(DoubleHistogramVariable histVar: histVars){
		    // find split process names on particle level
		    std::vector<std::string> splittedProcessNames = splitProcessNames(
			modifiedProcessName, histVar, doSplitParticleLevel );
		    // loop over split process names
		    for( std::string thisProcessName: splittedProcessNames){
			// first initialize the up and down variations to be equal to nominal
			// (needed for correct envelope computation)
			std::shared_ptr<TH1D> nominalHist;
			nominalHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at("nominal");
			for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			    std::shared_ptr<TH1D> upHist;
			    upHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at(upvar);
			    upHist->SetBinContent(i,nominalHist->GetBinContent(i));
			    std::shared_ptr<TH1D> downHist;
			    downHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at(downvar);
			    downHist->SetBinContent(i,nominalHist->GetBinContent(i));
			}
			// print for testing
			/*std::cout << variable << " before enveloping" << std::endl;
			for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			    std::cout << "bin " << i << std::endl;
			    std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			    std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
			}*/
			// then fill envelope in case valid pdf variations are present
			if(hasValidPdfs){
			    systematicTools::fillEnvelope( 
				histMap.at(thisProcessName).at(es).at(st).at(histVar.name()), 
				upvar, downvar, "pdfShapeVar");
			}
			// print for testing
			/*std::cout << variable << " after enveloping" << std::endl;
			for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			    std::cout << "bin " << i << std::endl;
			    std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			    std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
			}*/
		    } // end loop over split process names
		} // end loop over variables
		// do rms
		upvar = "pdfShapeRMSUp";
		downvar = "pdfShapeRMSDown";
		for(DoubleHistogramVariable histVar: histVars){
		    // find split process names on particle level
		    std::vector<std::string> splittedProcessNames = splitProcessNames(
			modifiedProcessName, histVar, doSplitParticleLevel );
		    // loop over split process names
		    for( std::string thisProcessName: splittedProcessNames){
			std::shared_ptr<TH1D> nominalHist;
			nominalHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at("nominal");
			// print for testing
			/*std::cout << variable << " before rmsing" << std::endl;
			for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			    std::cout << "bin " << i << std::endl;
			    std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			    std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
			}*/
			// then fill rms in case valid pdf variations are present
			if(hasValidPdfs){
			    systematicTools::fillRMS( 
				histMap.at(thisProcessName).at(es).at(st).at(histVar.name()),
				upvar, downvar, "pdfShapeVar"); 
			}
			// print for testing
			/*std::cout << variable << " after rmsing" << std::endl;
			for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			    std::cout << "bin " << i << std::endl;
			    std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			    std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
			}*/
		    } // end loop over split process names
		} // end loop over variables
	    } // end if systematic=="pdfShapeVar"
	    else if(systematic=="qcdScalesShapeVar"){
		std::string upvar = "qcdScalesShapeEnvUp";
		std::string downvar = "qcdScalesShapeEnvDown";
		for(DoubleHistogramVariable histVar: histVars){
		    // find split process names on particle level
		    std::vector<std::string> splittedProcessNames = splitProcessNames(
			modifiedProcessName, histVar, doSplitParticleLevel );
		    // loop over split process names
		    for( std::string thisProcessName: splittedProcessNames){
			// first initialize the up and down variations to be equal to nominal
			// (needed for correct envelope computation)
			std::shared_ptr<TH1D> nominalHist;
			nominalHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at("nominal");
			std::shared_ptr<TH1D> upHist;
                        upHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at(upvar);
			std::shared_ptr<TH1D> downHist;
                        downHist = histMap.at(thisProcessName).at(es).at(st).at(histVar.name()).at(downvar);
			for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			    upHist->SetBinContent(i,nominalHist->GetBinContent(i));
			    downHist->SetBinContent(i,nominalHist->GetBinContent(i));
			}
			// prints for testing:
			/*std::cout << "---------------------" << std::endl;
			std::cout << "process: " << thisProcessName << std::endl;
			std::cout << "variable: " << histVar.name() << std::endl;
			std::cout << "scale up:" << std::endl;
			for(int i=1; i<upHist->GetNbinsX()+1; ++i){
			    std::cout << upHist->GetBinContent(i) << std::endl; }
			std::cout << "scale down:" << std::endl;
			for(int i=1; i<downHist->GetNbinsX()+1; ++i){ 
			    std::cout << downHist->GetBinContent(i) << std::endl; }*/
			// then fill envelope in case valid qcd variations are present
			if(hasValidQcds){ 
			    systematicTools::fillEnvelope(
				histMap.at(thisProcessName).at(es).at(st).at(histVar.name()),
				upvar, downvar, "qcdScalesShapeVar"); 
			}
			// prints for testing
			/*std::cout << "scale up:" << std::endl;
			for(int i=1; i<upHist->GetNbinsX()+1; ++i){
			    std::cout << upHist->GetBinContent(i) << std::endl; }
			std::cout << "scale down:" << std::endl;
			for(int i=1; i<downHist->GetNbinsX()+1; ++i){
			    std::cout << downHist->GetBinContent(i) << std::endl; }*/
		    } // end loop over split process names
		} // end loop over variables
	    } // end if systematic=="qcdScalesShapeVar"
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
	// modify the process name for some selection types
        std::string modifiedProcessName = modProcessName(processName,selection_type);
	// loop over variables
	for(DoubleHistogramVariable histVar: histVars){
	    std::string variableName = histVar.name();
	    // need to distinguish between normal histograms and finely binned ones
	    // the latter will be rebinned in a later stage
	    // to do this correctly, they must not be clipped and all pdf and qcd variations are needed
	    bool storeLheVars = false;
	    bool doClip = true;
	    if( stringTools::stringContains(variableName,"fineBinned") ){
		storeLheVars = true;
		doClip = false;
	    }
	    if( selection_type == "fakerate" ) doClip = false;
	    // find split process names on particle level
	    std::vector<std::string> splittedProcessNames = splitProcessNames(
		modifiedProcessName, histVar, doSplitParticleLevel );
	    // loop over split process names
	    for( std::string thisProcessName: splittedProcessNames){
		// first find nominal histogram for this variable
		std::shared_ptr<TH1D> nominalhist = histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at("nominal");
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
		    for(auto mapelement : histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName)){
			if(stringTools::stringContains(mapelement.first,"nominal")) continue;
			histogram::copyHistogram(mapelement.second,nominalhist);
		    }
		}
		// clip and write nominal histogram
		if( doClip ) histogram::clipHistogram(nominalhist.get());
		nominalhist->Write();
		// loop over rest of histograms
		for(auto mapelement : histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName)){
		    if( stringTools::stringContains(mapelement.first,"nominal")) continue;
		    std::shared_ptr<TH1D> hist = mapelement.second;
		    // selection: do not store all individual pdf variations
		    if(stringTools::stringContains(hist->GetName(),"pdfShapeVar") 
			&& !storeLheVars) continue;
		    // selection: do not store all individual qcd scale variations
		    if(stringTools::stringContains(hist->GetName(),"qcdScalesShapeVar")
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
	    } // end loop over split process names for writing histograms
	} // end loop over variables for writing histograms
	outputFilePtr->Close();
    } } // end loop over event selections and selection types for writing histograms
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 16;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: runanalysis2.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input_directory" << std::endl;
	std::cerr << "sample_list" << std::endl;
	std::cerr << "sample_index" << std::endl;
	std::cerr << "output_directory" << std::endl;
	std::cerr << "variable_file" << std::endl;
	std::cerr << "event_selection" << std::endl;
	std::cerr << "selection_type" << std::endl;
	std::cerr << "muonfrmap" << std::endl;
	std::cerr << "electronfrmap" << std::endl;
        std::cerr << "electroncfmap" << std::endl;
	std::cerr << "nevents" << std::endl;
	std::cerr << "forcenevents" << std::endl;
	std::cerr << "bdt weight file (use 'nobdt' to not evaluate the BDT)" << std::endl;
        std::cerr << "bdt cut value (use a small value to not cut on BDT score)" << std::endl;
	std::cerr << "splitparticlelevel" << std::endl;
	std::cerr << "systematics (comma-separated list) (use 'none' for no systematics)" << std::endl;
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
    std::string& muonfrmap = argvStr[8];
    std::string& electronfrmap = argvStr[9];
    std::string& electroncfmap = argvStr[10];
    unsigned long nevents = std::stoul(argvStr[11]);
    bool forcenevents = ( argvStr[12]=="true" || argvStr[12]=="True" );
    std::string& bdtWeightsFile = argvStr[13];
    double bdtCutValue = std::stod(argvStr[14]);
    bool doSplitParticleLevel = ( argvStr[15]=="true" || argvStr[15]=="True" );
    std::string& systematicstr = argvStr[16];
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
    std::cout << "  - muon FR map: " << muonfrmap << std::endl;
    std::cout << "  - electron FR map: " << electronfrmap << std::endl;
    std::cout << "  - electron CF map: " << electroncfmap << std::endl;
    std::cout << "  - number of events: " << std::to_string(nevents) << std::endl;
    std::cout << "  - force number of events: " << std::to_string(forcenevents) << std::endl;
    std::cout << "  - BDT weights file: " << bdtWeightsFile << std::endl;
    std::cout << "  - BDT cut value: " << bdtCutValue << std::endl;
    std::cout << "  - do split particle level: " << std::to_string(doSplitParticleLevel) << std::endl;
    std::cout << "  - systematics:" << std::endl;
    for( std::string systematic: systematics ) std::cout << "      " << systematic << std::endl;

    // read variables
    std::vector<DoubleHistogramVariable> histVars = variableTools::readDoubleVariables( variable_file );
    std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( DoubleHistogramVariable var: histVars ){
        std::cout << var.toString() << std::endl;       
    }

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    std::map<std::string,double> emptymapPL = eventFlatteningParticleLevel::initVarMap();
    for(DoubleHistogramVariable histVar: histVars){
        std::string primaryVariable = histVar.primaryVariable();
        std::string secondaryVariable = histVar.secondaryVariable();
        // check primary variable (e.g. MVA score) at detector level
        if( emptymap.find(primaryVariable)==emptymap.end() ){
            std::string msg = "ERROR: variable '"+primaryVariable+"'";
            msg.append(" not recognized in particle-level variable map.");
            throw std::runtime_error(msg);
        }
        // check secondary variable (e.g. number of muons) at detector level
        if( emptymap.find(secondaryVariable)==emptymap.end() ){
            std::string msg = "ERROR: variable '"+secondaryVariable+"'";
            msg.append(" not recognized in detector-level variable map." );
            throw std::runtime_error(msg);
        }
        // check secondary variable (e.g. number of muons) at particle level
	if( doSplitParticleLevel ){
	    if( emptymapPL.find(secondaryVariable)==emptymapPL.end() ){
		std::string msg = "ERROR: variable '"+secondaryVariable+"'";
		msg.append(" not recognized in particle-level variable map.");
		throw std::invalid_argument(msg);
	    }
        }
    }

    // parse BDT weight file
    if( bdtWeightsFile=="nobdt" ){ bdtWeightsFile = ""; }

    // check validity of systematics
    for(std::string systematic : systematics){
	std::string testsyst = systematicTools::systematicType(systematic);
	if(testsyst=="ERROR") return -1;
    }

    // fill the histograms
    fillSystematicsHistograms( input_directory, sample_list, sample_index, output_directory,
			       histVars, event_selections, selection_types, 
			       muonfrmap, electronfrmap, electroncfmap, 
                               nevents, forcenevents, doSplitParticleLevel,
			       bdtWeightsFile, bdtCutValue, systematics );

    std::cerr << "###done###" << std::endl;
    return 0;
}
