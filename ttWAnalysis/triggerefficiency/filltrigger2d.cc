// This is the main C++ class used to determine trigger efficiencies.
// It is supposed to run on the output file of a skimming procedure
// and produce a root file containing a histogram of trigger efficiencies.

// Variation: only run on dilepton final states and make 2D distributions,
// as a function of the leading and subleading lepton pt,
// split per flavour state (mm, me, em, ee).

// include header with help functions
#include "interface/triggerTools.h"

std::map< std::string, std::shared_ptr<TH2D> > initializeHistograms( 
	const std::string& prefix, const std::vector<std::string>& flavours ){
    // make output collection of histograms
    std::map< std::string, std::shared_ptr<TH2D> > histMap;
    const std::vector< std::string > numdenoms = {"tot", "trig"};
    // loop over flavour combinations
    for( std::string flavour: flavours ){
	// loop over numerator and denominator
	for( std::string numdenom: numdenoms ){
	    std::string fullName = prefix+"_"+flavour+"_"+numdenom;
	    std::string mapName = flavour+"_"+numdenom;
	    // define binning for 2D histograms
	    std::vector< double > leadingPtBins = {30., 35., 45., 55., 75., 100., 150., 200.};
	    if(flavour=="mm" || flavour=="me"){
		leadingPtBins = {25., 30., 35., 45., 55., 75., 100., 150., 200.}; }
	    const std::vector< double > trailingPtBins = {20., 25., 30., 35., 45., 55., 75., 100., 150., 200.};
	    // initialize histogram
	    std::shared_ptr< TH2D > hist(
		new TH2D( fullName.c_str(), 
			  (fullName+ "; Leading lepton p_{T} (GeV); Subleading lepton p_{T} (GeV)").c_str(),
			  leadingPtBins.size() - 1, &leadingPtBins[0], 
			  trailingPtBins.size() - 1, &trailingPtBins[0] )
	    );
	    hist->Sumw2();
	    histMap[mapName] = hist;
	}
    }
    return histMap;
}

void fillEvent(const Event& event, double weight, 
		std::map<std::string,std::shared_ptr<TH2D>> histMap){
    // do all calculations and set variables locally.
    event.sortLeptonsByPt();
    if( event.leptonCollection().size()!=2 ){
	std::string msg = "ERROR in fillEvent: the size of the lepton collection is supposed to be 2,";
	msg.append( " but found " + std::to_string(event.leptonCollection().size()) );
	throw std::runtime_error(msg);
    }
    // determine lepton pt
    double leadpt = event.leptonCollection()[0].pt();
    double trailpt = event.leptonCollection()[1].pt();
    // determine flavour composition
    std::string flavourstr = triggerTools::getFlavourString(event);
    // check corresponding histogram
    if( histMap.find(flavourstr+"_tot")==histMap.end() ){
	std::string msg = "ERROR in fillEvent: histograms for flavour string "+flavourstr;
        msg.append( " not found in the histogram map." );
        throw std::runtime_error(msg);
    }
    // fill denominator
    histogram::fillValues( histMap[flavourstr+"_tot"].get(), leadpt, trailpt, weight );
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
    histogram::fillValues( histMap[flavourstr+"_trig"].get(), leadpt, trailpt, weight );
}

void fillTriggerEfficiencyHistograms(
	const std::string& pathToFile, 
	const std::string& outputFilePath,
	const std::string& eventSelection,
	const std::string& ptThresholdId,
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

    // initialize flavours
    const std::vector< std::string > flavours = {"mm", "me", "em", "ee"};

    // make output histograms
    std::map< std::string, std::shared_ptr<TH2D> > histMap = initializeHistograms(prefix,flavours);

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

	// select two tight light leptons + additional selections
        if(std::find(selectionTags.begin(),selectionTags.end(),"legacy")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
	    event.selectTightLeptons();
            if(event.leptonCollection().size()!=2) continue;
	    if(event.met().pt() < 40) continue;
	    if(event.leptonSystem().mass() < 20) continue;
        }

	// additional selection: reco pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"recoptcuts")!=selectionTags.end()){
	    event.sortLeptonsByPt(false);
	    std::string flavourStr = triggerTools::getFlavourString(event);
	    std::vector<double> recopt;
	    for( auto lepton: event.leptonCollection() ){
		recopt.push_back(lepton->uncorrectedPt());
	    }
	    if( !triggerTools::passPtThresholds( recopt, 
		triggerTools::ptThresholds(ptThresholdId, flavourStr) )) continue;
	}

	// additional selection: cone pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"coneptcuts")!=selectionTags.end()){
	    event.sortLeptonsByPt();
	    std::string flavourStr = triggerTools::getFlavourString(event);
	    std::vector<double> conept;
	    for( auto lepton: event.leptonCollection() ){
                conept.push_back(lepton->pt());
	    }
	    if( !triggerTools::passPtThresholds( conept, 
		triggerTools::ptThresholds(ptThresholdId, flavourStr) )) continue;
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
        fillEvent(event, weight, histMap);
    }

    std::cout << debugcounter << " events passed selection." << std::endl;

    // for MC it can happen that #pass > #tot (due to negative weights).
    // this gives errors when doing correct division, so need to manually fix here.
    // it could also happen that #tot < 0 or #trig < 0, so put minimum to zero first.
    if(!isData){
	for( std::string flavour: flavours ){	
	    std::shared_ptr<TH2D> tothist = histMap[flavour+"_tot"];
            std::shared_ptr<TH2D> trighist = histMap[flavour+"_trig"];
	    for(int j=0; j<tothist->GetNbinsX()+2; ++j){
		for(int k=0; k<tothist->GetNbinsY()+2; ++k){
		    if(tothist->GetBinContent(j,k)<0){
			tothist->SetBinContent(j,k,0);
			tothist->SetBinError(j,k,0);
		    }
		    if(trighist->GetBinContent(j,k)<0){
			trighist->SetBinContent(j,k,0);
			trighist->SetBinError(j,k,0);
		    }
		    if(trighist->GetBinContent(j,k)>tothist->GetBinContent(j,k)){
			trighist->SetBinContent(j,k,tothist->GetBinContent(j,k));
			trighist->SetBinError(j,k,tothist->GetBinError(j,k));
		    }
		}
	    }
        }
    }

    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->cd();
    // write histograms
    for(auto mapelement : histMap) mapelement.second->Write();
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    int nargs = 5;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: filltrigger2d.cc requires " << nargs << " arguments: " << std::endl;
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
    // fill the histograms
    fillTriggerEfficiencyHistograms(input_file_path, output_file_path, 
	event_selection, pt_threshold_id, nevents);
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
