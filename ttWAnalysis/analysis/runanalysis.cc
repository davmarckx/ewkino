// This is the main C++ class used to run the analysis,
// i.e. produce distributions + systematic variations as input for combine.

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
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ReweighterBTagShape.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

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
    unsigned numberOfQcdScaleVariations=6,
    const std::vector<std::string> allJecVariations={},
    const std::vector<std::string> groupedJecVariations={},
    const std::vector<std::string> bTagShapeSystematics={} ){
    // make map of histograms
    // the resulting map has five levels: 
    // map[process name][event selection][selection type][variable name][systematic]
    // notes:
    // - processNames is just one name (per file) in most cases,
    //   but can be multiple if a sample is split in sub-categories
    
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
		    // make the histInfo object
		    HistInfo histInfo( "", "", histVar.nbins(), histVar.xlow(), histVar.xhigh() );
		    // add nominal histogram
		    histMap[processName][eventSelection][selectionType][variableName]["nominal"] = histInfo.makeHist( baseName+"_nominal" );
		    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at("nominal")->SetTitle(processName.c_str());
		    // loop over systematics
		    for(std::string systematic : systematics){
			// special case for PDF variations: store individual variations 
			// as well as envelope and rms
			if(systematic=="pdfShapeVar"){
			    for(unsigned i=0; i<numberOfPdfVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			    std::vector<std::string> temps = {"pdfShapeEnvUp", "pdfShapeEnvDown",
							      "pdfShapeRMSUp", "pdfShapeRMSDown"};
			    for(std::string temp: temps){
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			// special case for QCD scales: store individual variations
			// as well as envelope
			else if(systematic=="qcdScalesShapeVar"){
			    for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			    std::vector<std::string> temps = {"qcdScalesShapeEnvUp", "qcdScalesShapeEnvDown"};
			    for(std::string temp: temps){
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			// special case for split JEC variations: store all variations
			else if(systematic=="JECAll" || systematic=="JECGrouped"){
			    std::vector<std::string> variations = allJecVariations;
			    if( systematic=="JECGrouped" ) variations = groupedJecVariations;
			    for(std::string jecvar: variations){
				std::string temp = systematic + "_" + jecvar + "Up";
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
				temp = systematic + "_" + jecvar + "Down";
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			// special case for bTag shape variations: store multiple systematics
			else if(systematic=="bTag_shape"){
			    for(std::string btagsys: bTagShapeSystematics){
				std::string temp = systematic + "_" + btagsys + "Up";
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
				temp = systematic + "_" + btagsys + "Down";
				histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
				histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    }
			}
			// now general case: store up and down variation
			else{
			    std::string temp = systematic + "Up";
			    histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
			    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			    temp = systematic + "Down";
			    histMap[processName][eventSelection][selectionType][variableName][temp] = histInfo.makeHist( baseName+"_"+temp );
			    histMap.at(processName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(processName.c_str());
			}
		    }
		}
	    }
	}
    }
    return histMap;
}

void setBTagShapeNorm( ){
    // normalize the b-tag shape reweighter
    // to implement later
}


void fillSystematicsHistograms(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<HistogramVariable>& histVars, 
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
            const std::string& muonFRMap, 
	    const std::string& electronFRMap, 
	    unsigned long nEntries,
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

    // when considering the data sample, disregard all systematics
    bool processNameIsData = (processName=="data" || processName=="Data"
			      || processName=="obs" || processName=="Obs"); 
    if( processNameIsData) systematics.clear();
    // warning: do not use isData() in the condition above, 
    // as systematic histograms must be created for fakes from data (see below)

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"fakerate")!=selection_types.end()){
	std::cout << "reading fake rate maps..." << std::endl;
        frmap_muon = readFakeRateTools::readFRMap(muonFRMap, "muon", year);
        frmap_electron = readFakeRateTools::readFRMap(electronFRMap, "electron", year);
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
    
    // normalize the bTag shape reweighter if present
    bool hasBTagShapeReweighter = reweighter.hasReweighter("bTag_shape");
    if( hasBTagShapeReweighter ){
	setBTagShapeNorm( ); // to implement
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
	&& (considerjecall || considerjecgrouped) ){
	std::cout << "finding available JEC uncertainty sources..." << std::endl;
	Event event = treeReader.buildEvent(0,false,false,considerjecall,considerjecgrouped);
	allJECVariations = event.jetInfo().allJECVariations();
	groupedJECVariations = event.jetInfo().groupedJECVariations();
	std::cout << "found following JEC uncertainty sources:" << std::endl;
	event.jetInfo().printAllJECVariations();
	event.jetInfo().printGroupedJECVariations();
    }

    // determine global sample properties related to bTag shape systematics
    std::vector<std::string> bTagShapeSystematics;
    bool considerbtagshape = (std::find(systematics.begin(),systematics.end(),"bTag_shape")
				!=systematics.end());
    if( considerbtagshape && !hasBTagShapeReweighter ){
	std::string msg = "ERROR: bTag_shape uncertainties requested,";
	msg.append( " but no bTag_shape reweighter present" );
	throw std::invalid_argument( msg );
    }
    if( considerbtagshape ){
	std::cout << "finding available b-tagging systematics..." << std::endl;
	bTagShapeSystematics = dynamic_cast<const ReweighterBTagShape*>( 
	    reweighter["bTag_shape"] )->availableSystematics();
	std::cout << "found following b-tagging systematics:" << std::endl;
	for(std::string el: bTagShapeSystematics){
	    std::cout << "  - " << el << std::endl;
	}
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
	processNames, event_selections, selection_types, histVars, systematics,
	numberOfPdfVariations, 6, 
	allJECVariations, groupedJECVariations,
	bTagShapeSystematics);

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
    if( nEntries!=0 && nEntries<numberOfEntries && !treeReader.isData() ){
	// loop over a smaller number of entries if samples are impractically large
        std::cout << "limiting number of entries to " << nEntries << std::endl;
        nEntriesReweight = (double)numberOfEntries/nEntries;
        std::cout << "(with corresponding reweighting factor " << nEntriesReweight << ")" << std::endl;
        numberOfEntries = nEntries;
    }
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;

	// loop over event selections and selection types
	for( std::string event_selection: event_selections ){
	for( std::string selection_type: selection_types ){
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();
	// (store nominal event variables, call only once and use for all weight systematics)
	std::map<std::string,double> accvarmap = eventFlattening::initVarMap();
	// (store acceptance-modified event variables, can be overwritten per acceptance variation)

	// fill nominal histograms
	bool passnominal = true;
	double lumiWeight = 0;
	double nominalWeight = 0;
        Event event = treeReader.buildEvent(
			entry,
			false,false,
			considerjecall,
			considerjecgrouped);
        if(!passES(event, event_selection, selection_type, "nominal")) passnominal = false;
	if(passnominal){
	    passNominalCounter.at(event_selection).at(selection_type)++;
	    varmap = eventFlattening::eventToEntry(event, 
			reweighter, selection_type, 
			frmap_muon, frmap_electron, "nominal");
	    lumiWeight = varmap.at("_weight")*nEntriesReweight;
	    nominalWeight = varmap.at("_normweight")*nEntriesReweight;
	    for(HistogramVariable histVar: histVars){
		std::string variableName = histVar.name();
		std::string variable = histVar.variable();
		histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at("nominal").get(),
				      varmap.at(variable), nominalWeight );
		// for data carrying a different processName than "data" (e.g. fakes from data),
		// loop over all systematics and fill with nominal values
		if(event.isData() and !processNameIsData){
		    for(auto mapelement: histMap.at(processName).at(event_selection).at(selection_type).at(variableName) ){
			if(stringTools::stringContains(mapelement.first,"nominal")) continue;
			histogram::fillValue( mapelement.second.get(),
					       varmap.at(variable), nominalWeight );
		    }
		}
	    }
	}

	// stop further event processing in case of data
	if(event.isData()) continue;

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
				    frmap_muon, frmap_electron, upvar);
		    double weight = accvarmap["_normweight"]*nEntriesReweight;
		    // for JEC: propagate into b-tag shape reweighting
		    /*if( systematic=="JEC" && considerbtagshape ){
			weight *= dynamic_cast<const ReweighterBTagShape*>( 
				    reweighter["bTag_shape"] )->weightJecVar( event, "JECUp" )
				    / reweighter["bTag_shape"]->weight( event );
		    }*/
		    // fill the variables
		    for(HistogramVariable histVar: histVars){
			std::string variableName = histVar.name();
			std::string variable = histVar.variable();
			histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(upvar).get(),
					      accvarmap.at(variable), weight);
		    }
		}
		// and with down variation
		bool passdown = true;
		if(!passES(event, event_selection, selection_type, downvar)) passdown = false;
		if(passdown){
		    accvarmap = eventFlattening::eventToEntry(event, 
				    reweighter, selection_type, 
				    frmap_muon, frmap_electron, downvar);
		    double weight = accvarmap["_normweight"]*nEntriesReweight;
		    // for JEC: propagate into b-tag shape reweighting
                    /*if( systematic=="JEC" && considerbtagshape ){
                        weight *= dynamic_cast<const ReweighterBTagShape*>(
                                    reweighter["bTag_shape"] )->weightJecVar( event, "JECDown" )
                                    / reweighter["bTag_shape"]->weight( event );
                    }*/
                    // fill the variables
		    for(HistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(downvar).get(),
                                              accvarmap.at(variable), weight);
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
				    reweighter, selection_type, frmap_muon, frmap_electron, thisupvar);
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
			for(HistogramVariable histVar: histVars){
			    std::string variableName = histVar.name();
			    std::string variable = histVar.variable();
			    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(systematic+"_"+thisupvar).get(),
						  accvarmap.at(variable), weight);
			}
		    }
		    // and with down variation
		    bool passdown = true;
		    if(!passES(event, event_selection, selection_type, thisdownvar)) passdown=false;
		    if(passdown){
			accvarmap = eventFlattening::eventToEntry(event,
				reweighter, selection_type, 
				frmap_muon, frmap_electron, thisdownvar);
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
			for(HistogramVariable histVar: histVars){
			    std::string variableName = histVar.name();
			    std::string variable = histVar.variable();
			    histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(systematic+"_"+thisdownvar).get(),
						  accvarmap.at(variable), weight);
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
		double upWeight = lumiWeight * reweighter.weightUp(event, systematic);
		double downWeight = lumiWeight * reweighter.weightDown(event, systematic);
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
	    /*// special case for bTag_shape reweighting (several systematics)
	    else if(systematic=="bTag_shape"){
		double nombweight = reweighter["bTag_shape"]->weight( event );
		for(std::string btagsys: bTagShapeSystematics){
		    double upweight = varmap["_normweight"]*nEntriesReweight / nombweight
					* dynamic_cast<const ReweighterBTagShape*>(
					    reweighter["bTag_shape"])->weightUp( event, btagsys );
		    double downweight = varmap["_normweight"]*nEntriesReweight / nombweight
                                        * dynamic_cast<const ReweighterBTagShape*>(
                                            reweighter["bTag_shape"])->weightDown( event, btagsys );
		    for(std::string variable : variables){
			fillVarValue(histMap[thisPName][variable][systematic+"_"+btagsys+"Up"],
			    getVarValue(variable,varmap),upweight);
			fillVarValue(histMap[thisPName][variable][systematic+"_"+btagsys+"Down"],
			    getVarValue(variable,varmap),downweight);
		    }
		}
	    }*/
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
	    else if(systematic=="rfScales" && hasValidQcds){
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
		    double reweight = event.generatorInfo().relativeWeightPdfVar(i)
                                        / xsecs.get()->crossSectionRatio_pdfVar(i);
                    double pdfweight = nominalWeight * reweight;
		    for(HistogramVariable histVar: histVars){
                        std::string variableName = histVar.name();
                        std::string variable = histVar.variable();
                        histogram::fillValue( histMap.at(processName).at(event_selection).at(selection_type).at(variableName).at(temp).get(),
                                              varmap.at(variable), pdfweight);
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
    for(std::string event_selection: event_selections){
    for(std::string selection_type: selection_types){
	for( std::string systematic : systematics ){
	    if(systematic=="pdfShapeVar"){
		// do envelope
		std::string upvar = "pdfShapeEnvUp";
		std::string downvar = "pdfShapeEnvDown";
		for(HistogramVariable histVar: histVars){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    std::shared_ptr<TH1D> nominalHist = histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at("nominal");
		    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(upvar)->SetBinContent(i,nominalHist->GetBinContent(i));
			histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at(downvar)->SetBinContent(i,nominalHist->GetBinContent(i));
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
			systematicTools::fillEnvelope( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()), 
						       upvar, downvar, "pdfShapeVar");
		    }
		    // print for testing
		    /*std::cout << variable << " after enveloping" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		}
		// do rms
		upvar = "pdfShapeRMSUp";
		downvar = "pdfShapeRMSDown";
		for(HistogramVariable histVar: histVars){
		    std::shared_ptr<TH1D> nominalHist = histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()).at("nominal");
		    // print for testing
		    /*std::cout << variable << " before rmsing" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		    // then fill rms in case valid pdf variations are present
		    if(hasValidPdfs){
			systematicTools::fillRMS( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
						  upvar, downvar, "pdfShapeVar"); 
		    }
		    // print for testing
		    /*std::cout << variable << " after rmsing" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
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
		    // prints for testing:
		    /*std::cout << "---------------------" << std::endl;
		    std::cout << "variable: " << variable << std::endl;
		    std::cout << "scale up:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][upvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl; }
		    std::cout << "scale down:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][downvar]->GetNbinsX()+1; ++i){ 
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl; }*/
		    // then fill envelope in case valid qcd variations are present
		    if(hasValidQcds){ 
			systematicTools::fillEnvelope( histMap.at(processName).at(event_selection).at(selection_type).at(histVar.name()),
						       upvar, downvar, "qcdScalesShapeVar"); 
		    }
		    // prints for testing
		    /*std::cout << "scale up:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][upvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl; }
		    std::cout << "scale down:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][downvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl; }*/
		}
	    }
	} // end loop over systematics to fill envelope and/or RMS
    } } // end loop over event selections and selection types to fill envelope and/or RMS

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // save histograms to the output file
    for(std::string event_selection: event_selections){
    for(std::string selection_type: selection_types){
	// loop over variables
	for(HistogramVariable histVar: histVars){
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
	    if( selection_type == "fakerate") doClip = false;
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
	} // end loop over variables for writing histograms
    } } // end loop over event selections and selection types for writing histograms
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 11;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: runanalysis.cc requires " << std::to_string(nargs) << " arguments to run...: " << std::endl;
        std::cerr << "input_directory" << std::endl;
	std::cerr << "sample_list" << std::endl;
	std::cerr << "sample_index" << std::endl;
	std::cerr << "output_directory" << std::endl;
	std::cerr << "variable_file" << std::endl;
	std::cerr << "event_selection" << std::endl;
	std::cerr << "selection_type" << std::endl;
	std::cerr << "muonfrmap" << std::endl;
	std::cerr << "electronfrmap" << std::endl;
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
    std::string& muonfrmap = argvStr[8];
    std::string& electronfrmap = argvStr[9];
    unsigned long nevents = std::stoul(argvStr[10]);
    std::string& systematicstr = argvStr[11];
    std::vector<std::string> systematics = stringTools::split(systematicstr,",");

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
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(HistogramVariable histVar: histVars){
        std::string variable = histVar.variable();
        if( emptymap.find(variable)==emptymap.end() ){
            std::string msg = "ERROR: variable '"+variable+"' not recognized.";
            throw std::runtime_error(msg);
        }
    }

    // check validity of systematics
    for(std::string systematic : systematics){
	std::string testsyst = systematicTools::systematicType(systematic);
	if(testsyst=="ERROR") return -1;
    }

    // fill the histograms
    fillSystematicsHistograms( input_directory, sample_list, sample_index, output_directory,
			       histVars, event_selections, selection_types, 
			       muonfrmap, electronfrmap, nevents,
			       systematics );

    std::cerr << "###done###" << std::endl;
    return 0;
}
