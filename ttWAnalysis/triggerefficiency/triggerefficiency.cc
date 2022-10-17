// This is the main C++ class used to determine trigger efficiencies.
// It is supposed to run on the output file of a skimming procedure
// and produce a root file containing a histogram of trigger efficiencies.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraphAsymmErrors.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/rootFileTools.h"
#include "../../Tools/interface/variableTools.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"


std::vector<double> ptThresholds( const std::string& id ){
    // return a vector of lepton pt thresholds
    // to be extended for ttW measurement
    std::vector<double> defaultres = {0.};
    if( id=="tzq" ){ return {25., 15., 10.}; }
    else if( id=="ttz" ){ return {40., 20., 10.}; }
    else if( id=="ttwdilep" ){ return {25., 15.}; } // just placeholder values for now
    else if( id=="ttwtrilep"){ return {25., 15., 10.}; } // just placeholder values for now
    else{ return defaultres; }
}

bool passPtThresholds( const std::vector<double>& pts, const std::vector<double>& thresholds ){
    // determine whether a given vector of pts passes a given vector of thresholds.
    // note that both are implicitly assumed to be sorted (in the same way)!
    unsigned ptssize = pts.size();
    unsigned thresholdssize = thresholds.size();
    if( ptssize!=thresholdssize ){
	std::string msg = "ERROR: vectors of pTs and corresponding pT thresholds";
	msg.append( " have different lengths." );
	throw std::runtime_error( msg );
    }
    for( unsigned i=0; i<ptssize; i++ ){
	if( pts[i]<thresholds[i] ){ return false; }
    }
    return true;
}

bool passTriggersRef( const Event& event ){
    // check whether an event passes reference triggers.
    // should be equivalent to event.passTriggers_ref(),
    // but the underlying ntuplizer variable is not in current skimmed samples...
    // see https://github.com/GhentAnalysis/heavyNeutrino/blob/
    //     UL_master/multilep/src/TriggerAnalyzer.cc
    std::vector<std::string> triggerNames;
    if( event.is2018() ){
	triggerNames = { "HLT_CaloMET350_HBHECleaned", 
			 "HLT_CaloJet500_NoJetID", 
			 "HLT_AK8PFJet500", 
			 "HLT_AK8PFJet400_TrimMass30",
			 "HLT_DiJet110_35_Mjj650_PFMET110", 
			 "HLT_PFHT800_PFMET75_PFMHT75_IDTight", 
			 "HLT_PFHT700_PFMET85_PFMHT85_IDTight",
                         "HLT_PFHT500_PFMET100_PFMHT100_IDTight", 
			 "HLT_PFHT1050", 
			 "HLT_PFJet500", 
			 "HLT_PFMET120_PFMHT120_IDTight", 
			 "HLT_PFMET250_HBHECleaned", 
			 "HLT_PFMET200_HBHE_BeamHaloCleaned", 
			 "HLT_PFMETTypeOne140_PFMHT140_IDTight",
                         "HLT_PFMETTypeOne200_HBHE_BeamHaloCleaned", 
			 "HLT_TripleJet110_35_35_Mjj650_PFMET110" };
    } else if( event.is2017() ){
	triggerNames = { "HLT_PFJet500", 
			 "HLT_PFMET140_PFMHT140_IDTight", 
			 "HLT_PFHT500_PFMET100_PFMHT100_IDTight",
                         "HLT_PFHT700_PFMET85_PFMHT85_IDTight", 
			 "HLT_PFHT800_PFMET75_PFMHT75_IDTight", 
			 "HLT_CaloJet500_NoJetID",
                         "HLT_AK8PFJet500" };
    } else{
	triggerNames = { "HLT_MET200", 
			 "HLT_PFMET300", 
			 "HLT_PFMET170_HBHECleaned", 
			 "HLT_PFMET120_PFMHT120_IDTight",
                         "HLT_PFHT300_PFMET110", 
			 "HLT_PFHT350_DiPFJetAve90_PFAlphaT0p53", 
			 "HLT_PFHT400_DiPFJetAve90_PFAlphaT0p52",
                         "HLT_PFHT400_SixJet30_DoubleBTagCSV_p056", 
			 "HLT_PFHT900", 
			 "HLT_PFHT650_WideJetMJJ900DEtaJJ1p5", 
			 "HLT_CaloJet500_NoJetID" };
    }
    for( std::string triggerName: triggerNames ){
	if( event.passTrigger(triggerName) ) return true;
    }
    return false;
}

std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms( 
	const std::string& prefix,
	const std::vector<HistogramVariable>& variables ){
    // make output collection of histograms
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    std::vector< std::string > numdenom = {"_tot", "_trig"};
    for( std::string tag: numdenom ){
	std::map< std::string, std::shared_ptr<TH1D> > temp;
	temp = variableTools::initializeHistograms( variables );
	for( auto el: temp ){
	    std::shared_ptr<TH1D> hist = el.second;
	    hist->SetName( (prefix+"_"+hist->GetName()+tag).c_str() );
	    histMap[el.first+tag] = hist;
	}
    }
    return histMap;
}

void fillEvent(const Event& event, double weight, 
		std::vector<HistogramVariable> variables,
		std::map<std::string,std::shared_ptr<TH1D>> histMap){
    // do all calculations and set variables locally.
    // make sure the naming convention in 'allvars' matches the one in 'variables'!
    std::vector<double> recopt;
    for( auto lepton: event.leptonCollection() ){
	recopt.push_back(lepton->uncorrectedPt());
    }
    std::sort(recopt.begin(),recopt.end(),std::greater<double>());
    std::map< std::string,double > allvars;
    allvars["leptonptleading"] = 0;
    allvars["leptonptsubleading"] = 0;
    allvars["leptonpttrailing"] = 0;
    if( recopt.size()>0 ) allvars["leptonptleading"] = recopt[0];
    if( recopt.size()>1 ) allvars["leptonptsubleading"] = recopt[1];
    if( recopt.size()>2 ) allvars["leptonpttrailing"] = recopt[2];
    allvars["yield"] = 0.5;
    // fill denominator
    for(unsigned i=0; i<variables.size(); ++i){
	std::string varname = variables[i].name();
	histogram::fillValue( histMap[varname+"_tot"].get(), allvars[varname], weight );
    }
    // fill numerator
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_ee() 
			|| event.passTriggers_eee() || event.passTriggers_m() 
			|| event.passTriggers_mm() || event.passTriggers_mmm()
			|| event.passTriggers_em() || event.passTriggers_eem() 
			|| event.passTriggers_emm();
    // printouts for testing:
    /*std::cout << "e: " << event.passTriggers_e() << std::endl;;
    std::cout << "ee: " << event.passTriggers_ee() << std::endl;
    std::cout << "eee: " << event.passTriggers_eee() << std::endl;
    std::cout << "m: " << event.passTriggers_m() << std::endl;
    std::cout << "mm: " << event.passTriggers_mm() << std::endl;
    std::cout << "mmm: " << event.passTriggers_mmm() << std::endl;
    std::cout << "em: " << event.passTriggers_em() << std::endl;
    std::cout << "eem: " << event.passTriggers_eem() << std::endl;
    std::cout << "emm: " << event.passTriggers_emm() << std::endl;
    std::cout << "any: " << passanytrigger << std::endl;*/

    if(!passanytrigger) return;
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = variables[i].name();
        histogram::fillValue( histMap[varname+"_trig"].get(), allvars[varname], weight );
    }
}

void fillTriggerEfficiencyHistograms(
	const std::string& pathToFile, 
	const std::string& outputFilePath,
	const std::string& eventSelection,
	const std::string& ptThresholdId,
	const std::vector<HistogramVariable> variables,
	long unsigned nEvents ){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    bool isData = treeReader.isData();
    std::string prefix;
    if(isData){
	prefix = stringTools::split( pathToFile, "/"  ).back();
	prefix = stringTools::removeOccurencesOf( prefix, ".root" );
    }
    else prefix = "mc";

    // make output histograms
    std::map< std::string, std::shared_ptr<TH1D> > histMap = initializeHistograms(prefix,variables);

    // initialize list of event id's for data overlap removal
    std::set<std::tuple<long,long,long>> evtlist;

    int debugcounter = 0;

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event
        Event event = treeReader.buildEvent(entry,true,true,false,false);
	event.applyLeptonConeCorrection();

	// split the event selection string into a list
	std::vector<std::string> selectionTags = stringTools::split(eventSelection,"_");

	// full event selection -> too little statistics
	// to adapt for ttW (but anyway too little statistics)
	/*if(std::find(selectionTags.begin(),selectionTags.end(),"full")!=selectionTags.end()){
	    if(!passES(event, "signalregion", "tight", "nominal")) continue;
	    int eventcategory = eventCategory(event, "nominal");
	    if(eventcategory == -1) continue;
	}*/

	// select three loose light leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3loose")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.removeTaus();
	    if(event.leptonCollection().size()!=3) continue;
	    if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
                if(event.leptonCollection().numberOfTightLeptons()==3) continue;
            }
	}

	// select three FO light leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3fo")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.removeTaus();
	    event.selectFOLeptons();
	    if(event.leptonCollection().size()!=3) continue;
	    if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
		if(event.leptonCollection().numberOfTightLeptons()==3) continue;
	    }
	}

	// select three tight light leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3tight")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.removeTaus();
	    event.selectTightLeptons();
	    if(event.leptonCollection().size()!=3) continue;
	}

	// select two SS loose light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"2loosess")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            if(event.leptonCollection().size()!=2) continue;
	    if(event.leptonCollection()[0].charge() != event.leptonCollection()[1].charge()) continue;
            if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
                if(event.leptonCollection().numberOfTightLeptons()==2) continue;
            }
        }

        // select two SS FO light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"2foss")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            event.selectFOLeptons();
            if(event.leptonCollection().size()!=2) continue;
	    if(event.leptonCollection()[0].charge() != event.leptonCollection()[1].charge()) continue;
            if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
                if(event.leptonCollection().numberOfTightLeptons()==2) continue;
            }
        }

        // select two SS tight light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"2tightss")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            event.selectTightLeptons();
            if(event.leptonCollection().size()!=2) continue;
	    if(event.leptonCollection()[0].charge() != event.leptonCollection()[1].charge()) continue;
        }

	// additional selection: reco pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"recoptcuts")!=selectionTags.end()){
	    std::vector<double> recopt;
	    for( auto lepton: event.leptonCollection() ){
		recopt.push_back(lepton->uncorrectedPt());
	    }
	    std::sort(recopt.begin(),recopt.end(),std::greater<double>());
	    if( !passPtThresholds( recopt, ptThresholds(ptThresholdId) )) continue;
	}

	// additional selection: cone pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"coneptcuts")!=selectionTags.end()){
	    event.sortLeptonsByPt();
	    std::vector<double> conept;
	    for( auto lepton: event.leptonCollection() ){
                conept.push_back(lepton->pt());
	    }
	    if( !passPtThresholds( conept, ptThresholds(ptThresholdId) )) continue;
	}

	double weight = event.weight();
	// for data: check if event passes orthogonal triggers and remove overlap
        if(isData){
	    // builtin reference trigger boolean (not present in current samples)
            //bool passreftrigger = event.passTriggers_ref();
	    // equivalent alternative: custom function checking individual triggers
	    bool passreftrigger = passTriggersRef(event);
            if(!passreftrigger) continue;
            std::tuple<long,long,long> evtid = std::make_tuple(event.runNumber(), 
						event.luminosityBlock(),
						event.eventNumber());
            if(std::binary_search(evtlist.begin(),evtlist.end(),evtid)){
		std::cout << "### WARNING ###: found duplicate event in data;";
		std::cout << " this should not happen if data was properly merged..." << std::endl;
		continue;
	    }
	    else evtlist.insert(evtid);
	    weight = 1.;
        }
	debugcounter++;
        fillEvent(event, weight, variables, histMap);
    }

    std::cout << debugcounter << " events passed selection." << std::endl;

    // for MC it can happen that #pass > #tot (due to negative weights).
    // this gives errors when doing correct division, so need to manually fix here.
    // it could also happen that #tot < 0 or #trig < 0, so put minimum to zero first.
    if(!isData){
	for(unsigned i=0; i<variables.size(); ++i){
	    std::string varname = variables[i].name();
	    std::shared_ptr<TH1D> tothist = histMap[varname+"_tot"];
            std::shared_ptr<TH1D> trighist = histMap[varname+"_trig"];
	    for(int j=0; j<tothist->GetNbinsX()+2; ++j){
		if(tothist->GetBinContent(j)<0){
		    tothist->SetBinContent(j,0);
		    tothist->SetBinError(j,0);
		}
		if(trighist->GetBinContent(j)<0){
		    trighist->SetBinContent(j,0);
		    trighist->SetBinError(j,0);
		}
		if(trighist->GetBinContent(j)>tothist->GetBinContent(j)){
		    trighist->SetBinContent(j,tothist->GetBinContent(j));
		    trighist->SetBinError(j,tothist->GetBinError(j));
		}
	    }
        }
    }

    // TGraphAsymmErrors::Divide seems to go fail on empty denominator bins.
    // so put an artificial entry there with zero error, 
    // so the division result will be zero content and zero error, as we want for this case
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = variables[i].name();
	std::shared_ptr<TH1D> tothist = histMap[varname+"_tot"];
        for(int j=0; j<tothist->GetNbinsX()+2; ++j){
            if(tothist->GetBinContent(j)==0){
                tothist->SetBinContent(j,1);
                tothist->SetBinError(j,0);
            }
        }
    }

    // make ratio TGraphAsymmErrors
    std::map< std::string,std::shared_ptr<TGraphAsymmErrors> > tGraphMap;
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = variables[i].name();
        tGraphMap[varname+"_eff"] = std::make_shared<TGraphAsymmErrors>();
	tGraphMap[varname+"_eff"].get()->Divide(histMap[varname+"_trig"].get(),
						histMap[varname+"_tot"].get(),
						"cl=0.683 b(1,1) mode");
	tGraphMap[varname+"_eff"].get()->SetName( std::string(prefix+"_"+varname+"_eff").c_str() );
	tGraphMap[varname+"_eff"].get()->SetTitle( std::string(prefix+"_"+varname+"_eff").c_str() );
	// prints for debugging	
	/*for(int j=0; j<histMap[varname+"_trig"]->GetNbinsX()+2; ++j){
	    std::cout << "--------------" <<std::endl;
	    // print bin
	    std::cout << "bin: " << histMap[varname+"_trig"]->GetBinLowEdge(j) << " -> "
	    << histMap[varname+"_trig"]->GetBinLowEdge(j) + histMap[varname+"_trig"]->GetBinWidth(j) 
	    << std::endl;
	    // print bin contents
	    std::cout << "trig: " << histMap[varname+"_trig"]->GetBinContent(j) << std::endl;
	    std::cout << "total: " << histMap[varname+"_tot"]->GetBinContent(j) << std::endl;
	    std::cout << "ratio: " << tGraphMap[varname+"_eff"]->GetY()[j] << std::endl;
	    // print left and right errors
	    std::cout << "center: " << tGraphMap[varname+"_eff"]->GetX()[j] << std::endl;
	    std::cout << "left: " << tGraphMap[varname+"_eff"]->GetErrorXlow(j) << std::endl;
	    std::cout << "right: " << tGraphMap[varname+"_eff"]->GetErrorXhigh(j) << std::endl;
	}
	// print number of points in graph
	std::cout << "npoints: " << tGraphMap[varname+"_eff"]->GetN() << std::endl;
	*/
    }   
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->cd();
    // write histograms
    for(auto mapelement : histMap) mapelement.second->Write();
    // write tgraphs
    for(auto mapelement : tGraphMap) mapelement.second->Write();
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    int nargs = 5;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: triggerefficiency.cc requires " << nargs << " arguments: " << std::endl;
        std::cerr << "- path to input file" << std::endl;
	std::cerr << "- path to output file" << std::endl;
	std::cerr << "- event selection" << std::endl;
	std::cerr << "- pT threshold id" << std::endl;
	std::cerr << "- number of events to process" << std::endl;
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& event_selection = argvStr[3];
    std::string& pt_threshold_id = argvStr[4];
    long unsigned nevents = std::stoul(argvStr[5]);
    // check validity
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput) return -1;
    // define variables (arbitrary names, only used for histogram titles)
    std::vector<HistogramVariable> variables;
    variables.push_back(HistogramVariable("leptonptleading", "Leading lepton p_{T}", 12, 0., 300.));
    variables.push_back(HistogramVariable("leptonptsubleading", "Subleading lepton p_{T}", 12, 0., 180.));
    variables.push_back(HistogramVariable("leptonpttrailing", "Trailing lepton p_{T}", 12, 0., 120));
    variables.push_back(HistogramVariable("yield", "Total yield", 1, 0., 1.));
    // fill the histograms
    fillTriggerEfficiencyHistograms(input_file_path, output_file_path, 
	event_selection, pt_threshold_id, variables, nevents);
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
