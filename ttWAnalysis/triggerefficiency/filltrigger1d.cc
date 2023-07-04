// This is the main C++ class used to determine trigger efficiencies.
// It is supposed to run on the output file of a skimming procedure
// and produce a root file containing a histogram of trigger efficiencies.

// include tools
#include "interface/triggerTools.h"

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
    event.sortLeptonsByPt(false);
    std::map< std::string,double > allvars;
    allvars["leptonptleading"] = 0;
    allvars["leptonptsubleading"] = 0;
    allvars["leptonpttrailing"] = 0;
    if( event.leptonCollection().size()>0 ){ 
	allvars["leptonptleading"] = event.leptonCollection()[0].uncorrectedPt(); }
    if( event.leptonCollection().size()>1 ){
	allvars["leptonptsubleading"] = event.leptonCollection()[1].uncorrectedPt(); }
    if( event.leptonCollection().size()>2 ){
	allvars["leptonpttrailing"] = event.leptonCollection()[2].uncorrectedPt(); }
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

        // select four loose light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"4loose")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            if(event.leptonCollection().size()!=4) continue;
            if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
                if(event.leptonCollection().numberOfTightLeptons()==4) continue;
            }
        }

        // select four FO light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"4fo")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            event.selectFOLeptons();
            if(event.leptonCollection().size()!=4) continue;
            if(std::find(selectionTags.begin(),selectionTags.end(),"tightveto")!=selectionTags.end()){
                if(event.leptonCollection().numberOfTightLeptons()==4) continue;
            }
        }

        // select four tight light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"4tight")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            event.selectTightLeptons();
            if(event.leptonCollection().size()!=4) continue;
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
	    event.sortLeptonsByPt(false);
	    std::vector<double> recopt;
	    std::string flavourStr = triggerTools::getFlavourString(event);
	    for( auto lepton: event.leptonCollection() ){
		recopt.push_back(lepton->uncorrectedPt());
	    }
	    if( !triggerTools::passPtThresholds( recopt, 
		triggerTools::ptThresholds(ptThresholdId,flavourStr) )) continue;
	}

	// additional selection: cone pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"coneptcuts")!=selectionTags.end()){
	    event.sortLeptonsByPt();
	    std::vector<double> conept;
	    std::string flavourStr = triggerTools::getFlavourString(event);
	    for( auto lepton: event.leptonCollection() ){
                conept.push_back(lepton->pt());
	    }
	    if( !triggerTools::passPtThresholds( conept, 
		triggerTools::ptThresholds(ptThresholdId,flavourStr) )) continue;
	}

	double weight = event.weight();
	// for data: check if event passes orthogonal triggers and remove overlap
        if(isData){
	    // builtin reference trigger boolean (not present in current samples)
            //bool passreftrigger = event.passTriggers_ref();
	    // equivalent alternative: custom function checking individual triggers
	    bool passreftrigger = triggerTools::passTriggersRef(event);
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
        std::cerr << "ERROR: filltrigger1d.cc requires " << nargs << " arguments: " << std::endl;
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
    variables.push_back(HistogramVariable("leptonpttrailing", "Trailing lepton p_{T}", 12, 0., 120.));
    variables.push_back(HistogramVariable("yield", "Total yield", 1, 0., 1.));
    // fill the histograms
    fillTriggerEfficiencyHistograms(input_file_path, output_file_path, 
	event_selection, pt_threshold_id, variables, nevents);
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
