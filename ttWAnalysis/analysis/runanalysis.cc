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
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../Tools/interface/EFTCrossSections.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ReweighterBTagShape.h"

// include analysis tools
#include "../btagging/interface/bTaggingTools.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"
#include "../eventselection/interface/eventFlatteningParticleLevel.h"


// help function for loading training weights from ML directory

std::map< std::string, double > loadWeights(
    const std::string& processName,
    int sampleInd ,
    const std::string& year ){
    // function to find the reweighting factors needed for taking into account
    // the missing training sample in the dilepton signalregions.
    // these factors are stored in the ML directory and are made by make_analysisdatasets.py.

   int sampleIndex = sampleInd;
   if (processName == "TTW" && sampleIndex == 0){ sampleIndex = 16;}
   if (processName == "TTW" && sampleIndex == 1){ sampleIndex = 18;}

   std::fstream newfile;
   std::string location = "../../ML/trainindices/";
   location += "weights_" + processName + "_" + std::to_string(sampleIndex) + "_" + year + ".txt";

   newfile.open(location);

   std::map< std::string, double > weightMap;

   if (newfile.is_open()){ //checking whether the file is open
      std::string tp;
      while(getline(newfile, tp)){ //read data from file object and put it into string.
        if( tp == "" ) continue;
        std::stringstream ss(tp);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> vstrings(begin, end);

        std::string region = vstrings[0];
        double val = std::stod(vstrings[1]);
        //std::cout << "region="+region+"=" << std::endl;
        //std::cout << "weight="+std::to_string(val)+"=" << std::endl;

        weightMap.insert(make_pair(region, val));
   }
      newfile.close(); //close the file object.
   }

   return weightMap;
}

// help functions for modifying or splitting the process name

std::string modProcessName( const std::string& processName, const std::string& selectionType ){
    // small internal helper function to modify the process name.
    // the process name is changed to
    // - "nonprompt" for selection type "fakerate"
    // - "chargeflips" for selection type "chargeflips"
    // - else left unmodified.
    std::string modProcessName = processName;
    if( selectionType=="fakerate" ) modProcessName = "nonprompt";
    if( selectionType=="efakerate" ) modProcessName = "nonprompte";
    if( selectionType=="mfakerate" ) modProcessName = "nonpromptm";
    if( selectionType=="chargeflips" ) modProcessName = "chargeflips";
    return modProcessName;
}


std::vector<std::string> splitProcessNames(
    const std::string& processName,
    const std::vector<HistogramVariable>& particleLevelVars,
    const bool doSplitParticleLevel ){
    // make process names split on particle level (if requested).
    // the following process names are made (starting from process TTW and variable _nMuons as example):
    // TTW0, TTW_nMuons1, ..., TTW_nMuonsN where N is the number of bins.
    // note: TTW0 is intended for events that do not pass particle level selection.
    // note: if the process name is "nonprompt" or "chargeflips",
    //       the doSplitParticleLevel bool is bypassed to be always false
    //       (as nonprompt and chargeflips are taken from data).
    // note: if only one variable is needed for particle level split,
    //       use the utility function below.
    std::vector<std::string> splitProcessNames;
    if( !doSplitParticleLevel
        || processName=="nonprompt"
        || processName=="nonprompte"
        || processName=="nonpromptm"
        || processName=="chargeflips" ){
        splitProcessNames = {processName};
    }
    else{
	splitProcessNames.push_back( processName+"0" );
	for(const HistogramVariable& particleLevelVar: particleLevelVars){
	    for(int i=1; i<=particleLevelVar.nbins(); i++){
                std::string name = processName + std::to_string(i) + stringTools::removeOccurencesOf(particleLevelVar.name(),"_");
		splitProcessNames.push_back( name );
	    }
        }
    }
    return splitProcessNames;
}


// help function for initializing all histograms

std::map< std::string,     // process
    std::map< std::string, // event selection
    std::map< std::string, // selection type
    std::map< std::string, // variable
    std::map< std::string, // systematic
    std::shared_ptr<TH1D> > > > > > initHistMap(
    const std::vector<std::string>& processNames,
    const bool isData,
    const bool doSplitParticleLevel,
    const std::map<std::string, std::vector<HistogramVariable>>& splitParticleLevelVars,
    const std::vector<std::string>& eventSelections,
    const std::vector<std::string>& selectionTypes,
    const std::vector<std::shared_ptr<Variable>>& histVars,
    const std::vector<std::string>& systematics,
    unsigned numberOfPdfVariations=100,
    unsigned numberOfQcdScaleVariations=6,
    const std::vector<std::string>& allJecVariations={},
    const std::vector<std::string>& groupedJecVariations={},
    const std::vector<std::string>& bTagShapeSystematics={},
    const std::vector<std::string>& eftVariations={} ){
    // make map of histograms
    // the resulting map has five levels: 
    // map[process name][event selection][selection type][variable name][systematic]
    // notes:
    // - processNames should be just one name (per file) in most cases,
    //   but can be multiple if a sample is split in sub-categories.
    //   note that it will be split further internally for particle level splits.
    // - isData should be true for data samples and false for MC samples;
    //   systematics are skipped for data except for fake rate selection type.
    // - splitParticleLevelVars should map variable names to lists of particle level variables,
    //   indicating for each detector level variable which splits at particle level are needed.
    
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
		for(std::shared_ptr<Variable> histVar: histVars){
		    std::string variableName = histVar->name();
		    // make process names split on particle level (if needed)
		    std::vector<std::string> splittedProcessNames = splitProcessNames(
			modifiedProcessName,
			splitParticleLevelVars.at(variableName),
			doSplitParticleLevel);
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
		    histMap[thisProcessName][eventSelection][selectionType][variableName]["nominal"] = histVar->initializeHistogram( baseName+"_nominal" );
		    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at("nominal")->SetTitle(thisProcessName.c_str());
		    // for data, skip initialization of histograms for systematics
                    // (except for selection type "fakerate", in which case systematic histograms
                    // should be added and filled with nominal values,
                    // in order to correctly merge with MC with the same selection type).
		    if( isData && !(selectionType=="fakerate"
			|| selectionType=="efakerate" || selectionType=="mfakerate") ) continue;
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
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			    std::vector<std::string> temps = {"pdfShapeEnvUp", "pdfShapeEnvDown",
							      "pdfShapeRMSUp", "pdfShapeRMSDown"};
			    for(std::string temp: temps){
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for QCD scales: store individual variations
			// as well as envelope
			else if(systematic=="qcdScalesShapeVar"){
			    for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
				std::string temp = systematic + std::to_string(i);
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			    std::vector<std::string> temps = {"qcdScalesShapeEnvUp", "qcdScalesShapeEnvDown"};
			    for(std::string temp: temps){
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for split JEC variations: store all variations
			else if(systematic=="JECAll" || systematic=="JECGrouped"){
			    std::vector<std::string> variations = allJecVariations;
			    if( systematic=="JECGrouped" ) variations = groupedJecVariations;
			    for(std::string jecvar: variations){
				std::string temp = systematic + "_" + jecvar + "Up";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
				temp = systematic + "_" + jecvar + "Down";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for bTag shape variations: store multiple systematics
			else if(systematic=="bTag_shape"){
			    for(std::string btagsys: bTagShapeSystematics){
				std::string temp = systematic + "_" + btagsys + "Up";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
				temp = systematic + "_" + btagsys + "Down";
				histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
				histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    }
			}
			// special case for EFT variations: store individual variations
                        else if(systematic=="eft"){
                            for(std::string eftVariation: eftVariations){
				std::string temp = "EFT" + eftVariation;
                                histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
                                histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
                            }
                        }
			// now general case: store up and down variation
			else{
			    std::string temp = systematic + "Up";
			    histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
			    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			    temp = systematic + "Down";
			    histMap[thisProcessName][eventSelection][selectionType][variableName][temp] = histVar->initializeHistogram( baseName+"_"+temp );
			    histMap.at(thisProcessName).at(eventSelection).at(selectionType).at(variableName).at(temp)->SetTitle(thisProcessName.c_str());
			}
		    } // systematics
		    } // particle level process names
		} // variables
	    } // selection types
	} // event selections
    } // process names
    return histMap;
}


// help function for filling histograms

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
    const std::shared_ptr<Variable>& variable,
    bool doSplitParticleLevel,
    const HistogramVariable& splitParticleLevelVar,
    bool passParticleLevel,
    double valueParticleLevel ){
    // helper function to find the correct histogram to fill in the map.
    // for the case where no split in particle level is made, this is straightforward;
    // but in case of split, need to find the correctly modified process name.
    // note: if the process name is "nonprompt" or "chargeflips",
    //       the doSplitParticleLevel bool is bypassed to be always false
    //       (as nonprompt and chargeflips are taken from data).
    std::string thisProcessName = processName;
    std::string variableName = variable->name();
    if( doSplitParticleLevel
	&& processName!="nonprompt" 
	&& processName!="nonprompte" 
	&& processName!="nonpromptm" 
	&& processName!="chargeflips" ){
        // if event does not pass particle level, use appendix 0
        if( !passParticleLevel ){ thisProcessName = processName + "0"; }
        // else use appendix bin number
	// note: keep this naming convention in sync with the one defined
	//       in splitProcessNames, else errors will occur.
        else{
            int binNumber = splitParticleLevelVar.findBinNumber( valueParticleLevel );
            thisProcessName = processName + std::to_string(binNumber) + stringTools::removeOccurencesOf(splitParticleLevelVar.name(),"_");
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
    std::shared_ptr<TH1D> hist,
    const std::shared_ptr<Variable>& variable,
    double primaryValue, double secondaryValue,
    double weight ){
    // small helper function to fill a given histogram
    // note: secondaryValue is ignored for single histogram variables!
    if( variable->type()=="single" ){ histogram::fillValue(hist.get(), primaryValue, weight); }
    else if( variable->type()=="double"){
	int binNb = variable->findBinNumber( primaryValue, secondaryValue );
	hist->Fill( binNb, weight );
    }
    else{
	std::string msg = "ERROR: variable type " + variable->type() + " not recognized.";
	throw std::runtime_error(msg);
    }
}


void fillHistogram(
    std::shared_ptr<TH1D> hist,
    const std::shared_ptr<Variable>& variable,
    const std::map<std::string, double>& values,
    double weight ){
    // same as above, but providing a full value map instead of specific values
    // (the specific values are extracted automatically).
    // warning: no check is done in this function to make sure that the correct variable
    //          is in the value map, this should be done outside this function!
    if( variable->type()=="single" ){
	double value = values.at(variable->variable());
	fillHistogram(hist, variable, value, 0., weight);
    }
    else if( variable->type()=="double"){
        double primaryValue = values.at(variable->primaryVariable());
	double secondaryValue = values.at(variable->secondaryVariable());
	fillHistogram(hist, variable, primaryValue, secondaryValue, weight);
    }
    else{
        std::string msg = "ERROR: variable type " + variable->type() + " not recognized.";
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
    const std::shared_ptr<Variable>& variable,
    double primaryValue, double secondaryValue, double weight,
    bool doSplitParticleLevel, 
    const HistogramVariable& splitParticleLevelVar,
    bool passParticleLevel,
    double valueParticleLevel){
    // small helper function to find correct histogram in the map and fill it.
    std::shared_ptr<TH1D> histogramToFill = findHistogramToFill(
        histMap, processName, eventSelection, selectionType, systematic,
        variable, doSplitParticleLevel, splitParticleLevelVar, 
	passParticleLevel, valueParticleLevel );
    fillHistogram(histogramToFill, variable, primaryValue, secondaryValue, weight);
}


void fillHistograms(
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
    const std::shared_ptr<Variable>& variable,
    double primaryValue, double secondaryValue, double weight,
    bool doSplitParticleLevel,
    const std::vector<HistogramVariable>& splitParticleLevelVars,
    bool passParticleLevel,
    const std::map<std::string, double>& varmapParticleLevel){
    // extension of the above, looping over multiple variables at particle level
    if( !doSplitParticleLevel || !passParticleLevel ){
        fillHistogram( histMap, processName, eventSelection, selectionType, systematic,
            variable, primaryValue, secondaryValue, weight,
	    doSplitParticleLevel, HistogramVariable(), false, 0. );
	    // (last three arguments are just dummy values needed for syntax but not used.)
    }
    else{
        for( const HistogramVariable& splitParticleLevelVar: splitParticleLevelVars ){
	    double valueParticleLevel;
	    try{ valueParticleLevel = varmapParticleLevel.at(splitParticleLevelVar.variable()); }
	    catch(...){
		std::string msg = "ERROR in fillHistograms: particle level variable not found in the map!";
		msg.append(" Error occurred for following key: "+splitParticleLevelVar.variable()+"\n");
		throw std::runtime_error(msg);
	    }
            fillHistogram( histMap, processName, eventSelection, selectionType, systematic,
                variable, primaryValue, secondaryValue, weight,
		doSplitParticleLevel, splitParticleLevelVar,
                passParticleLevel, valueParticleLevel );
        }
    }
}


void fillHistograms(
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
    const std::vector<std::shared_ptr<Variable>>& variables,
    const std::map<std::string, double>& values,
    double weight,
    bool doSplitParticleLevel,
    const std::map<std::string, std::vector<HistogramVariable>>& splitParticleLevelVars,
    bool passParticleLevel,
    const std::map<std::string, double>& varmapParticleLevel){
    // same as above but loop over detector-level variables
    for(std::shared_ptr<Variable> var: variables){
        std::string variableName = var->name();
	std::string type = var->type();
	if( type=="single" ){
	    std::string variable = var->variable();
            fillHistograms( histMap, processName,
                    eventSelection, selectionType, systematic,
                    var, values.at(variable), 0., weight,
                    doSplitParticleLevel, splitParticleLevelVars.at(variableName),
                    passParticleLevel, varmapParticleLevel );
        }
	else if( type=="double" ){
	    std::string primaryVariable = var->primaryVariable();
	    std::string secondaryVariable = var->secondaryVariable();
            fillHistograms( histMap, processName,
                    eventSelection, selectionType, systematic,
                    var, values.at(primaryVariable), values.at(secondaryVariable),
		    weight,
                    doSplitParticleLevel, splitParticleLevelVars.at(variableName),
                    passParticleLevel, varmapParticleLevel );
	}
    }
}


void fillSystematicsHistograms(
	    const std::string& inputDirectory, 
	    const std::string& sampleList, 
	    unsigned int sampleIndex, 
	    const std::string& outputDirectory,
	    const std::vector<std::shared_ptr<Variable>>& histVars, 
	    const std::vector<std::string>& event_selections, 
	    const std::vector<std::string>& selection_types,
            const std::string& muonFRMap, 
	    const std::string& electronFRMap,
            const std::string& electronCFMap,
	    unsigned long nEntries,
            bool forceNEntries,
	    bool doSplitParticleLevel,
	    const std::map<std::string, std::vector<HistogramVariable>>& splitParticleLevelVars,
	    bool doBDT,
	    const std::string& bdtWeightsFile,
	    double bdtCutValue,
            bool trainingreweight,
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

    //load training reweights if needed
    std::vector<std::string> trainingSampleVector = {"TT", "TTZ", "TTH", "TTW", "TTG"};
    std::map< std::string, double > weightMap;
    if( trainingreweight && std::count(trainingSampleVector.begin(), trainingSampleVector.end(), processName)){
       weightMap = loadWeights(processName, sampleIndex, year);
    }

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(std::find(selection_types.begin(),selection_types.end(),"fakerate")!=selection_types.end()
	|| std::find(selection_types.begin(),selection_types.end(),"efakerate")!=selection_types.end()
	|| std::find(selection_types.begin(),selection_types.end(),"mfakerate")!=selection_types.end()){
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
    std::vector<std::string> bTagShapeVariations;
    std::map< std::string, std::map< std::string, std::map< int, double >>> bTagWeightMap;
    if( hasBTagShapeReweighter ){
	// find available b-tagging systematics
	// note: also do this for data to make sure that copies of nominal
	//       are correctly created for this systematic
	std::cout << "finding available b-tagging systematics..." << std::endl;
        bTagShapeSystematics = dynamic_cast<const ReweighterBTagShape*>(
            reweighter["bTag_shape"] )->availableSystematics();
        std::cout << "found following b-tagging systematics:" << std::endl;
        for(std::string el: bTagShapeSystematics){
            std::cout << "  - " << el << std::endl;
	}
        // also find variations (which include systematics, but also JEC variations)
	bTagShapeVariations = dynamic_cast<const ReweighterBTagShape*>(
            reweighter["bTag_shape"] )->availableVariations();
	// read normalization factors (only needed for simulation, not for data)
	if( !treeReader.isData() ){
	    // read b-tagging shape reweighting normalization factors from txt file
	    std::string txtInputFile = "../btagging/output_20240412/";
	    // (hard-coded for now, maybe replace by argument later)
	    txtInputFile += year + "/" + inputFileName;
	    txtInputFile = stringTools::replace(txtInputFile, ".root", ".txt");
	    if( !systemTools::fileExists(txtInputFile) ){
		std::string msg = "ERROR in loading b-tagging normalization factors:";
		msg += " file " + txtInputFile + " does not exist.";
		throw std::runtime_error(msg);
	    }
	    std::vector<std::string> variationsToRead = {"central"};
	    for( std::string var: bTagShapeVariations ){
		variationsToRead.push_back("up_"+var);
		variationsToRead.push_back("down_"+var);
	    }
	    // (note: in the above, we read normalization factors for all b-tag variations.
	    //  this is important if the JEC variations of b-tag scale factors are used
	    //  when calculating the JEC systematics.
	    //  in case the latter is disabled, i.e. when using nominal b-tagging for each JEC variation,
	    //  it suffices to use bTagShapeSystematics instead of bTagShapeVariations in the loop above.)
	    bTagWeightMap = bTaggingTools::textToMap( txtInputFile, event_selections, variationsToRead );
	}
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
	// find available jec variations
	// note: also need to do this for data (to correctly create copies of nominal),
	//       but variations are not stored in data files, so hard-coded for now.
        if( !treeReader.isData() ){
	    std::cout << "finding available JEC uncertainty sources..." << std::endl;
	    Event event = treeReader.buildEvent(0,false,false,considerjecall,considerjecgrouped);
	    allJECVariations = event.jetInfo().allJECVariations();
	    groupedJECVariations = event.jetInfo().groupedJECVariations();
	    std::cout << "found following all JEC uncertainty sources:" << std::endl;
	    event.jetInfo().printAllJECVariations();
	    std::cout << "found following grouped JEC uncertainty sources:" << std::endl;
	    event.jetInfo().printGroupedJECVariations();
	}
	else{
	    groupedJECVariations = {"Absolute_" + year.substr(0, 4),
		"Absolute", "BBEC1_"+year.substr(0, 4), "BBEC1", "EC2_"+year.substr(0, 4),
		"EC2", "FlavorQCD", "HF_"+year.substr(0, 4), "HF", "RelativeBal",
		"RelativeSample_"+year.substr(0, 4), "Total"};
	}
    }

    // determine global sample properties related to EFT coefficients
    std::vector<std::string> eftVariations;
    std::shared_ptr<EFTCrossSections> eftCrossSections;
    // check if EFT variations are needed
    bool considereft = false;
    for( std::string systematic: systematics ){
        if( systematic=="eft") considereft = true;
    }
    if( treeReader.numberOfEntries()>0 && considereft ){
        // find available EFT variations
        if( treeReader.containsEFTCoefficients() ){
            std::cout << "finding available EFT variations..." << std::endl;
            Event event = treeReader.buildEvent(0,false,false,false,false,false,true);
	    for(std::string wcName: event.eftInfo().wcNames()){
		eftVariations.push_back(wcName);
	    }
	    eftCrossSections = std::make_shared<EFTCrossSections>( treeReader.currentSample() );
            std::cout << "found following EFT variations:" << std::endl;
            for(auto el: eftVariations) std::cout << "  - " << el << std::endl;
        }
    }
    // set magnitude of wilson coefficients
    // (hard-coded for now, maybe extend later)
    // first set (agreed magnitudes with Oviedo)
    std::map<std::string, double> WCConfig = {
        {"ctlTi", 5.},
        {"ctq1", 0.3},
        {"ctq8", 0.3},
        {"cQq83", 1.},
        {"cQq81", 1.},
        {"cQlMi", 5.},
        {"cbW", 5.},
        {"cpQ3", 5.},
        {"ctei", 5.},
        {"cQei", 5.},
        {"ctW", 1.},
        {"cpQM", 5.},
        {"ctlSi", 5.},
        {"ctZ", 5.},
        {"cQl3i", 5.},
        {"ctG", 0.3},
        {"cQq13", 0.1},
        {"cQq11", 0.1},
        {"cptb", 5.},
        {"ctli", 1.},
        {"ctp", 5.},
        {"cpt", 5.},
        {"sm", 0.}
    };
    // second set (for internal testing)
    /*std::map<std::string, double> WCConfig = {
	{"ctlTi", 0.1},
        {"ctq1", 0.5},
        {"ctq8", 0.3},
        {"cQq83", 0.5},
        {"cQq81", 1.},
        {"cQlMi", 1.},
        {"cbW", 5.},
        {"cpQ3", 5.},
        {"ctei", 5.},
        {"cQei", 5.},
        {"ctW", 1.},
        {"cpQM", 5.},
        {"ctlSi", 0.3},
        {"ctZ", 2.5},
        {"cQl3i", 1.},
        {"ctG", 0.6},
        {"cQq13", 0.3},
        {"cQq11", 1.},
        {"cptb", 5.},
        {"ctli", 0.1},
        {"ctp", 5.},
        {"cpt", 5.},
	{"sm", 0.}
    };*/
    for(std::string eftVariation: eftVariations){
	if(WCConfig.find(eftVariation) == WCConfig.end()){
	    std::string msg = "ERROR: eft variation " + eftVariation;
	    msg += " not found in hard-coded config map.";
	    throw std::runtime_error(msg);
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
	processNames, treeReader.isData(), 
	doSplitParticleLevel, splitParticleLevelVars,
	event_selections, selection_types, histVars, systematics,
	numberOfPdfVariations, 6, 
	allJECVariations, groupedJECVariations,
	bTagShapeSystematics,
	eftVariations );

    // load the BDT
    // default value is nullptr, 
    // in which case the BDT will not be evaluated.
    std::shared_ptr<TMVA::Experimental::RBDT<>> bdt;
    if( doBDT ){
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
			doSplitParticleLevel,
                        considereft );

	// initialize map of variables
        std::map<std::string,double> varmap = eventFlattening::initVarMap();
        // (store nominal event variables, call only once and use for all weight systematics)
        std::map<std::string,double> accvarmap = eventFlattening::initVarMap();
        // (store acceptance-modified event variables, can be overwritten per acceptance variation)
	std::map<std::string,double> varmapParticleLevel = eventFlatteningParticleLevel::initVarMap();
	// (store variables at particle level)

	// loop over event selections
	for( std::string event_selection: event_selections ){

        // nEntriesReweight is also used for when samples lost 20% SR training samples, therefore here it's renamed to nEntriesAndTrainingReweight 
        double nEntriesAndTrainingReweight = nEntriesReweight;
        // reweight if requested, if dilepton signalregion and if the sample was in the training set// selection_type == "tight"
        if( trainingreweight && event_selection.find("signalregion_dilepton") != std::string::npos && std::count(trainingSampleVector.begin(), trainingSampleVector.end(), processName)){
          //std::cout << "reweighting" << std::endl;
          double trainingweight = weightMap[event_selection];
          //std::cout << "weight: " + std::to_string(trainingweight) << std::endl;
          nEntriesAndTrainingReweight *= trainingweight;
          //std::cout << "result: " + std::to_string(nEntriesAndTrainingReweight) << std::endl;
        }

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
	    nominalWeight = varmap.at("_normweight")*nEntriesAndTrainingReweight;
	    if( bdtCutValue>-1 ){
		// do additional selection on final BDT score
		// note: to be 100% correct, this should be re-evaluated for every jec systematic,
		// but for now just do a cut-and-continue on nominal.
		if( varmap["_eventBDT"]<bdtCutValue ) continue;
	    }
	    passNominalCounter.at(event_selection).at(selection_type)++;
	    fillHistograms( histMap, thisProcessName, 
		    event_selection, selection_type, "nominal",
		    histVars, varmap, nominalWeight,
		    doSplitParticleLevel, splitParticleLevelVars,
		    passParticleLevel, varmapParticleLevel );
	}
	
	// fill data systematics histograms (for fakerate selection).
	// (they are simply filled with nominal values
	//  except for dedicated fakerate systematics)
	if(event.isData() && passnominal 
	    && (selection_type=="fakerate"
		|| selection_type=="efakerate" || selection_type=="mfakerate")){
	    // loop over systematics
	    std::vector<std::string> alreadyFilled;
	    alreadyFilled.push_back("nominal");
	    for(std::string systematic : systematics){
		if( stringTools::stringStartsWith(systematic, "efakerate") 
                    || stringTools::stringStartsWith(systematic, "mfakerate") ){
		    std::string upvar = systematic+"Up";
		    std::string downvar = systematic+"Down";
		    double upWeight = nominalWeight * reweighter.singleWeightUp(event, systematic);
		    double downWeight = nominalWeight * reweighter.singleWeightDown(event, systematic);
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            false, splitParticleLevelVars,
                            false, varmapParticleLevel );
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            false, splitParticleLevelVars,
                            false, varmapParticleLevel );
		    alreadyFilled.push_back(upvar);
		    alreadyFilled.push_back(downvar);
		}
	    }
	    // default case for all other histograms: just use nominal values
	    for(std::shared_ptr<Variable> histVar: histVars){
		std::string variableName = histVar->name();
		for(auto mapelement: histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName) ){
		    // exclude previously filled histograms
		    bool needToFill = true;
		    for(std::string veto: alreadyFilled){
			if(stringTools::stringContains(mapelement.first,veto)){
			    needToFill = false;
			    break;
			}
		    }
		    if( !needToFill ) continue;
		    // fill this remaining histogram
		    fillHistogram( mapelement.second, histVar, varmap, nominalWeight );
		}
	    }
	} // end if block over data systematics

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
		    double weight = accvarmap["_normweight"]*nEntriesAndTrainingReweight;
		    // for JEC: propagate into b-tag shape reweighting
		    if( systematic=="JEC" && hasBTagShapeReweighter ){
			std::string jesvar = "jes";
                        if( dynamic_cast<const ReweighterBTagShape*>( 
                                    reweighter["bTag_shape"] )->hasVariation( jesvar ) ){
			    weight *= dynamic_cast<const ReweighterBTagShape*>( 
				    reweighter["bTag_shape"] )->weightUp( event, jesvar )
				    / reweighter["bTag_shape"]->weight( event );
			} else{
			    std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
                            std::cerr << "reweighter not recognized" << std::endl;
			}
		    }
		    // fill the variables
		    fillHistograms( histMap, thisProcessName,
			    event_selection, selection_type, upvar,
			    histVars, accvarmap, weight,
			    doSplitParticleLevel, splitParticleLevelVars,
			    passParticleLevel, varmapParticleLevel );
		}
		// and with down variation
		bool passdown = true;
		if(!passES(event, event_selection, selection_type, downvar)) passdown = false;
		if(passdown){
		    accvarmap = eventFlattening::eventToEntry(event, 
				    reweighter, selection_type, 
				    frmap_muon, frmap_electron, cfmap_electron, downvar,
                                    bdt, year);
		    double weight = accvarmap["_normweight"]*nEntriesAndTrainingReweight;
		    // for JEC: propagate into b-tag shape reweighting
                    if( systematic=="JEC" && hasBTagShapeReweighter ){
                        std::string jesvar = "jes";
                        if( dynamic_cast<const ReweighterBTagShape*>(
                                    reweighter["bTag_shape"] )->hasVariation( jesvar ) ){
			    weight *= dynamic_cast<const ReweighterBTagShape*>(
                                    reweighter["bTag_shape"] )->weightDown( event, jesvar )
                                    / reweighter["bTag_shape"]->weight( event );
			} else{
                            std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
                            std::cerr << "reweighter not recognized" << std::endl;
                        }
                    }
                    // fill the variables
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, accvarmap, weight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
			double weight = accvarmap["_normweight"]*nEntriesAndTrainingReweight;
			// for JEC: propagate into b-tag shape reweighting
			if( hasBTagShapeReweighter && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
			    if(jecvar=="Total") jesvar = "jes";
			    if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar ) ){
				weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightUp( event, jesvar )
					/reweighter["bTag_shape"]->weight( event );
			    } else{
				std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
				std::cerr << "reweighter not recognized" << std::endl;
			    }
			}
			// fill the variables
			fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, systematic + "_" + thisupvar,
                            histVars, accvarmap, weight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
			double weight = accvarmap["_normweight"]*nEntriesAndTrainingReweight;
                        // for JEC: propagate into b-tag shape reweighting
                        if( hasBTagShapeReweighter && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
			    if(jecvar=="Total") jesvar = "jes";
                            if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar ) ){
                                weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightDown( event, jesvar )
                                        /reweighter["bTag_shape"]->weight( event );
                            } else{
                                std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
                                std::cerr << "reweighter not recognized" << std::endl;
                            }
                        }
			// fill the variables
			fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, systematic + "_" + thisdownvar,
                            histVars, accvarmap, weight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // special case for bTag_shape reweighting (several systematics)
	    else if(systematic=="bTag_shape"){
		double nombweight = reweighter["bTag_shape"]->weight( event );
		for(std::string btagsys: bTagShapeSystematics){
		    double upWeight = varmap["_normweight"]*nEntriesAndTrainingReweight / nombweight
					* dynamic_cast<const ReweighterBTagShape*>(
					    reweighter["bTag_shape"])->weightUp( event, btagsys );
		    double downWeight = varmap["_normweight"]*nEntriesAndTrainingReweight / nombweight
                                        * dynamic_cast<const ReweighterBTagShape*>(
                                            reweighter["bTag_shape"])->weightDown( event, btagsys );
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, systematic+"_"+btagsys+"Up",
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, systematic+"_"+btagsys+"Down",
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		}
	    }
	    // ELSE apply nominal weight (already in variable nominalWeight)

	    // EFT reweighting
	    else if(systematic=="eft"){
		for(std::string eftVariation: eftVariations){
		    std::map<std::string, double> wcvalues = {{eftVariation, WCConfig.at(eftVariation)}};
		    double eftNominalWeight = event.lumiScale()
						* event.eftInfo().nominalWeight()*nEntriesAndTrainingReweight
						/ eftCrossSections->nominalSumOfWeights()
						* varmap.at("_reweight");
		    double weight = eftNominalWeight * event.eftInfo().relativeWeight(wcvalues);
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, "EFT"+eftVariation,
                            histVars, varmap, weight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		}
	    }

	    // theory reweighting
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fScaleNorm" && hasValidQcds){
		double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScaleNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rfScales" && hasValidQcds){
                double upWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_2_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rfScalesNorm" && hasValidQcds){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="isrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_ISR_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="fsrShape" && hasValidPSs){
		double upWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_2()
							    / xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * event.generatorInfo().relativeWeight_FSR_0p5()
							    / xsecs.get()->crossSectionRatio_FSR_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fsrNorm" && hasValidPSs){
                double upWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_2();
                double downWeight = nominalWeight * xsecs.get()->crossSectionRatio_FSR_0p5();
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, temp,
                            histVars, varmap, qcdweight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="qcdScalesNorm" && hasValidQcds){
		double upWeight = nominalWeight*qcdScalesMaxXSecRatio;
                double downWeight = nominalWeight*qcdScalesMinXSecRatio;
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		    fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, temp,
                            histVars, varmap, pdfweight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
		fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, upvar,
                            histVars, varmap, upWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
                fillHistograms( histMap, thisProcessName,
                            event_selection, selection_type, downvar,
                            histVars, varmap, downWeight,
                            doSplitParticleLevel, splitParticleLevelVars,
                            passParticleLevel, varmapParticleLevel );
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
	// modify the process name for some selection types
        std::string modifiedProcessName = modProcessName(processName,selection_type);
	if( treeReader.isData() && 
	    !(selection_type=="fakerate"
	      || selection_type=="efakerate"
	      || selection_type=="mfakerate") ) continue;
        if( selection_type=="chargeflips" ) continue;
	for( std::string systematic : systematics ){
	    if(systematic=="pdfShapeVar"){
		// do envelope
		std::string upvar = "pdfShapeEnvUp";
		std::string downvar = "pdfShapeEnvDown";
		for(std::shared_ptr<Variable> histVar: histVars){
		std::string variableName = histVar->name();
		std::vector<std::string> splittedProcessNames = splitProcessNames(
                    modifiedProcessName, splitParticleLevelVars.at(variableName), doSplitParticleLevel );
                for( std::string thisProcessName: splittedProcessNames){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    std::shared_ptr<TH1D> nominalHist = histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at("nominal");
		    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at(upvar)->SetBinContent(i,nominalHist->GetBinContent(i));
			histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at(downvar)->SetBinContent(i,nominalHist->GetBinContent(i));
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
			systematicTools::fillEnvelope( histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName), 
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
		for(std::shared_ptr<Variable> histVar: histVars){
		std::string variableName = histVar->name();
		std::vector<std::string> splittedProcessNames = splitProcessNames(
                    modifiedProcessName, splitParticleLevelVars.at(variableName), doSplitParticleLevel );
                for( std::string thisProcessName: splittedProcessNames){
		    // first find nominal
		    std::shared_ptr<TH1D> nominalHist = histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at("nominal");
		    // print for testing
		    /*std::cout << variable << " before rmsing" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		    // then fill rms in case valid pdf variations are present
		    if(hasValidPdfs){
			systematicTools::fillRMS( histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName),
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
	    }
	    else if(systematic=="qcdScalesShapeVar"){
		std::string upvar = "qcdScalesShapeEnvUp";
		std::string downvar = "qcdScalesShapeEnvDown";
		for(std::shared_ptr<Variable> histVar: histVars){
		std::string variableName = histVar->name();
		std::vector<std::string> splittedProcessNames = splitProcessNames(
                    modifiedProcessName, splitParticleLevelVars.at(variableName), doSplitParticleLevel );
                for( std::string thisProcessName: splittedProcessNames){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    std::shared_ptr<TH1D> nominalHist = histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at("nominal");
		    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
			histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at(upvar)->SetBinContent(i,nominalHist->GetBinContent(i));
			histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName).at(downvar)->SetBinContent(i,nominalHist->GetBinContent(i));
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
			systematicTools::fillEnvelope( histMap.at(thisProcessName).at(event_selection).at(selection_type).at(variableName),
						       upvar, downvar, "qcdScalesShapeVar"); 
		    }
		    // prints for testing
		    /*std::cout << "scale up:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][upvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl; }
		    std::cout << "scale down:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][downvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl; }*/
		} // end loop over split process names
		} // end loop over variables
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
	// modify the process name for some selection types
        std::string modifiedProcessName = modProcessName(processName,selection_type);
	// loop over variables
	for(std::shared_ptr<Variable> histVar: histVars){
	    std::string variableName = histVar->name();
	    // need to distinguish between normal histograms and finely binned ones
	    // the latter will be rebinned in a later stage
	    // to do this correctly, they must not be clipped and all pdf and qcd variations are needed
	    bool storeLheVars = false;
	    bool doClip = true;
	    if( stringTools::stringContains(variableName,"fineBinned") ){
		storeLheVars = true;
		doClip = false;
	    }
	    if( selection_type=="fakerate" || selection_type=="efakerate" || selection_type=="mfakerate" ) doClip = false;
	    // loop over split process names
            std::vector<std::string> splittedProcessNames = splitProcessNames(
                modifiedProcessName, splitParticleLevelVars.at(variableName), doSplitParticleLevel );
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
	    } // end loop over rest of histograms
	    } // end loop over split process names for writing histograms
	} // end loop over variables for writing histograms
	outputFilePtr->Close();
    } } // end loop over event selections and selection types for writing histograms
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 17;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: runanalysis.cc requires " << std::to_string(nargs);
	std::cerr << " arguments to run...: " << std::endl;
        std::cerr << "  - input directory" << std::endl;
	std::cerr << "  - sample list" << std::endl;
	std::cerr << "  - sample index" << std::endl;
	std::cerr << "  - output directory" << std::endl;
	std::cerr << "  - variable file" << std::endl;
	std::cerr << "  - event selection (if multiple, separate by commas)" << std::endl;
	std::cerr << "  - selection type (if multiple, separate by comma)" << std::endl;
	std::cerr << "  - muon fake rate map (use dummy if not needed)" << std::endl;
	std::cerr << "  - electron fake rate map (use dummy if not needed)" << std::endl;
        std::cerr << "  - electron charge flip map (use dummy if not needed)" << std::endl;
	std::cerr << "  - number of events (use negative value for all events)" << std::endl;
	std::cerr << "  - force number of events (for data)" << std::endl;
        std::cerr << "  - bdt weight file (use 'none' to not evaluate the BDT)" << std::endl;
	std::cerr << "  - bdt cut value (use 'none' to not cut on BDT score)" << std::endl;
	std::cerr << "  - particle level split variable file" << std::endl;
	std::cerr << "    (spcial case: 'none' to not split at particle level)" << std::endl;
	std::cerr << "    (spcial case: 'auto': use same variable as main variable (for single variables" << std::endl;
	std::cerr << "     OR use secondary variable (for double variables)" << std::endl;
	std::cerr << "  - bool whether to use reweighting of samples with partial removal for training" << std::endl;
	std::cerr << "  - systematics (comma-separated list) (use 'none' for no systematics)" << std::endl;
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
    bool doBDT = true;
    if( bdtWeightsFile=="none" ) doBDT = false;
    std::string& bdtCutValueStr = argvStr[14];
    double bdtCutValue = -99;
    if( bdtCutValueStr!="none" ) bdtCutValue = std::stod(bdtCutValueStr);
    std::string& splitParticleLevelVarFile = argvStr[15];
    bool doSplitParticleLevel = true;
    bool autoSplitParticleLevel = false;
    if( splitParticleLevelVarFile=="none" ) doSplitParticleLevel = false;
    if( splitParticleLevelVarFile=="auto" ) autoSplitParticleLevel = true;
    bool trainingreweight = ( argvStr[16]=="true" || argvStr[16]=="True" );
    std::string& systematicstr = argvStr[17];
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
    std::cout << "  - split particle level variable file: " << splitParticleLevelVarFile << std::endl;
    std::cout << "  - training reweight is used: " << std::to_string(trainingreweight) << std::endl;
    std::cout << "  - systematics:" << std::endl;
    for( std::string systematic: systematics ) std::cout << "      " << systematic << std::endl;

    // read variables
    std::vector<std::shared_ptr<Variable>> histVars;
    histVars = variableTools::readHistogramVariables( variable_file );
    std::cout << "found following variables (from " << variable_file << "):" << std::endl;
    for( std::shared_ptr<Variable> var: histVars ){
        std::cout << var->toString() << std::endl;       
    }

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(std::shared_ptr<Variable> histVar: histVars){
	std::string type = histVar->type();
	if( type=="single" ){
	    std::string variable = histVar->variable();
	    if( emptymap.find(variable)==emptymap.end() ){
		std::string msg = "ERROR: variable " + variable + " not found";
		msg.append( " in the per-event variable map!" );
		throw std::invalid_argument(msg);
	    }
	} else if( type=="double" ){
	    std::string primaryVariable = histVar->primaryVariable();
	    std::string secondaryVariable = histVar->secondaryVariable();
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
	    // (note: need to do this check also if doSplitParticleLevel is false,
	    //  because it will still give map::at() errors if the variable is not found.)
	    // update: is this still true? if so, try to remove that behaviour.
	    /*if( emptymapPL.find(secondaryVariable)==emptymapPL.end() ){
		std::string msg = "ERROR: variable '"+secondaryVariable+"'";
		msg.append(" not recognized in particle-level variable map.");
		throw std::invalid_argument(msg);
	    }*/
	}
	else{
	    std::string msg = "ERROR variable type " + type + "not recognized";
	    msg += " for variable " + histVar->name() + ".";
	    throw std::runtime_error(msg);
	}
    }

    // read variables for split at particle level if requested,
    // and make a map of detector-level variable names to particle-level variables
    std::map<std::string, std::vector<HistogramVariable>> splitParticleLevelVars;
    std::vector<HistogramVariable> allParticleLevelVars;
    if( doSplitParticleLevel ){
	// case 1: automatic from provided HistogramVariables or DoubleHistogramVariables as main variables
	if( autoSplitParticleLevel ){
	    for(std::shared_ptr<Variable> histVar: histVars){
		std::string name = histVar->name();
		std::string type = histVar->type();
		// case 1a: HistogramVariable, use same splitting as on detector-level
		if( type=="single" ){
		    HistogramVariable var = histVar->histogramVariable();
		    std::vector<HistogramVariable> vec = { var };
                    splitParticleLevelVars[name] = vec;
                    allParticleLevelVars.push_back( var );
		}
		// case 1b: DoubleHistogramVariable, use splitting by secondary variable
		else if( type=="double" ){
		    HistogramVariable secondary = histVar->secondary();
		    std::vector<HistogramVariable> vec = { secondary };
		    splitParticleLevelVars[name] = vec;
		    allParticleLevelVars.push_back( secondary );
		}
		else{
		    std::string msg = "ERROR variable type " + type + "not recognized";
		    msg += " for variable " + histVar->name() + ".";
		    throw std::runtime_error(msg);
		}
	    }
	}
	// case 2: using a dedicated list
	else{
	    allParticleLevelVars = variableTools::readVariables( splitParticleLevelVarFile );
	    for(std::shared_ptr<Variable> histVar: histVars){
		std::string name = histVar->name();
		splitParticleLevelVars[name] = allParticleLevelVars;
	    }
	}
        // check variables
        std::map<std::string,double> emptymapPL = eventFlatteningParticleLevel::initVarMap();
        for(const HistogramVariable& particleLevelVar: allParticleLevelVars){
            std::string variable = particleLevelVar.variable();
            if( emptymapPL.find(variable)==emptymapPL.end() ){
                std::string msg = "ERROR: variable " + variable + " not found";
                msg.append(" in the particle-level per-event variable map.");
                throw std::invalid_argument(msg);
            }
        }
    }
    // if no split on particle level is requested,
    // still fill the map with an empty particle-level vector
    // for each detector-level variable (easier for syntax)
    else{
	for(std::shared_ptr<Variable> histVar: histVars){
            std::string name = histVar->name();
	    std::vector<HistogramVariable> dummy;
            splitParticleLevelVars[name] = dummy;
	}
    }

    // parse BDT weight file
    if( !doBDT ){ bdtWeightsFile = ""; }

    // check validity of systematics
    for(std::string systematic : systematics){
	std::string testsyst = systematicTools::systematicType(systematic);
	if(testsyst=="ERROR"){
	    std::string msg = "ERROR: systematic " + testsyst + " returned an error.";
            throw std::invalid_argument(msg);
	}
    }

    // fill the histograms
    fillSystematicsHistograms( input_directory, sample_list, sample_index, output_directory,
			       histVars, event_selections, selection_types, 
			       muonfrmap, electronfrmap, electroncfmap, 
                               nevents, forcenevents,
			       doSplitParticleLevel, splitParticleLevelVars,
			       doBDT, bdtWeightsFile, bdtCutValue,
			       trainingreweight, systematics );

    std::cerr << "###done###" << std::endl;
    return 0;
}
