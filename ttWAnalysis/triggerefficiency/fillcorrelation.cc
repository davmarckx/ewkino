// Test the (absense of) correlation between reference and lepton triggers

// include header with help functions
#include "interface/triggerTools.h"

std::map< std::string, std::shared_ptr<TH2D> > initializeHistograms(
	const std::string& prefix ){
    // make output collection of histograms
    // (only one histogram for now, but keep general for potential extensions)
    std::map< std::string, std::shared_ptr<TH2D> > histMap;
    std::vector<std::string> triggers = {"singlelepton", "dilepton", "trilepton", "anylepton"};
    for( std::string trigger: triggers ){
	std::string fullName = prefix+"_"+trigger+"_correlation";
	std::string mapName = trigger+"_correlation";
	std::shared_ptr< TH2D > hist(
	new TH2D( fullName.c_str(), 
	      (fullName+ "; Passes reference triggers; Passes lepton triggers").c_str(),
	      2, -0.5, 1.5, 
	      2, -0.5, 1.5 )
	);
	hist->Sumw2();
	histMap[mapName] = hist;
    }
    return histMap;
}

void fillTriggerCorrelationHistograms(
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

    // make output histograms
    std::map< std::string, std::shared_ptr<TH2D> > histMap = initializeHistograms(prefix);

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents!=0 && nEvents<numberOfEntries ){ numberOfEntries = nEvents; }
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event
        Event event = treeReader.buildEvent(entry,true,true,false,false);
	event.applyLeptonConeCorrection();
	double weight = event.weight();

	// split the event selection string into a list
        std::vector<std::string> selectionTags = stringTools::split(eventSelection,"_");

	// select three tight light leptons
        if(std::find(selectionTags.begin(),selectionTags.end(),"3tight")!=selectionTags.end()){
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();
            event.removeTaus();
            event.selectTightLeptons();
            if(event.leptonCollection().size()!=3) continue;
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
	
	// check if event passes reference triggers
	// builtin reference trigger boolean (not present in current samples)
        //bool passreftrigger = event.passTriggers_ref();
	// equivalent alternative: custom function checking individual triggers
	bool passreftrigger = triggerTools::passTriggersRef(event);

	// check if event passes lepton triggers
	bool passsingleleptrigger = event.passTriggers_e() || event.passTriggers_m();
	bool passdileptrigger = event.passTriggers_ee() || event.passTriggers_mm()
				|| event.passTriggers_em();
        bool passtrileptrigger = event.passTriggers_eee() || event.passTriggers_mmm()
                                || event.passTriggers_eem() || event.passTriggers_emm();
	bool passanyleptrigger = passsingleleptrigger || passdileptrigger || passtrileptrigger;
        
	// fill the histograms
	histogram::fillValues( histMap["singlelepton_correlation"].get(), 
                               int(passreftrigger), int(passsingleleptrigger), weight );
	histogram::fillValues( histMap["dilepton_correlation"].get(), 
                               int(passreftrigger), int(passdileptrigger), weight );
	histogram::fillValues( histMap["trilepton_correlation"].get(), 
                               int(passreftrigger), int(passtrileptrigger), weight );
	histogram::fillValues( histMap["anylepton_correlation"].get(), 
                               int(passreftrigger), int(passanyleptrigger), weight );
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
    fillTriggerCorrelationHistograms(input_file_path, output_file_path, 
	event_selection, pt_threshold_id, nevents);
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
