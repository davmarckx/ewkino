// include header
#include "../interface/fakeRateMeasurementTools.h"

// help function for creating a 2D histogram map
RangedMap< RangedMap< std::shared_ptr< TH1D > > > build2DHistogramMap( 
    const std::vector< double >& ptBinBorders, const std::vector< double >& etaBinBorders, 
    const HistInfo& mtHistInfo, const std::string& name ){

    std::map< double, RangedMap< std::shared_ptr< TH1D > > >  histMap2DTemp;
    for( auto ptBinBorder : ptBinBorders ){
        std::map< double, std::shared_ptr< TH1D > > histMapTemp;

        for( auto etaBinBorder : etaBinBorders ){ 
            histMapTemp[etaBinBorder] = mtHistInfo.makeHist( name + "_pT_" 
		+ std::to_string( int( ptBinBorder ) ) + "_eta_" 
		+ stringTools::replace( stringTools::doubleToString( etaBinBorder, 2 ), ".", "p" ) );
        }
        histMap2DTemp.insert( { ptBinBorder, RangedMap< std::shared_ptr< TH1D > >( histMapTemp ) } );
    }
    return RangedMap< RangedMap< std::shared_ptr< TH1D > > >( histMap2DTemp );
}

// help function for writing a 2D histogram map
void write2DHistogramMap( const RangedMap< RangedMap< std::shared_ptr< TH1D > > >& histMap ){
    for( auto& map_pair : histMap ){
        for( auto& hist_pair : map_pair.second ){
            hist_pair.second->Write();
        }
    }
}

// help function for creating a lepton reweigther
// NOTE: REQUESTED WEIGHT FILES ARE NOT PRESENT,
// THIS FUNCTION IS EWKINO SPECIFIC, DO NOT USE!
/*std::shared_ptr< Reweighter > makeLeptonReweighter( const std::string& year, const bool isMuon, 
    const bool isFO){

    std::string flavorString = ( isMuon ? "m" : "e" );
    std::string wpString = ( isFO ? "FO" : "3lTight" );
    const std::string weightDirectory = "../../weights/";
    std::string file_name = stringTools::formatDirectoryName(weightDirectory);
    file_name.append("weightFiles/leptonSF/leptonSF_" + flavorString);
    file_name.append("_" + year + "_" + wpString + ".root");
    TFile* leptonSFFile = TFile::Open( file_name.c_str() );
    std::shared_ptr< TH2 > leptonSFHist( dynamic_cast< TH2* >( leptonSFFile->Get( "EGamma_SF2D" ) ) );
    leptonSFHist->SetDirectory( gROOT );
    leptonSFFile->Close();

    if( isMuon ){
        if( isFO ){
            MuonReweighter muonReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        } else {
            MuonReweighter muonReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        }
    } else {
        if( isFO ){
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        } else {
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        }
        return nullptr;
    }
}*/

// function for filling fake rate histograms for a single sample
void fillFakeRateMeasurementHistograms(const std::string& leptonFlavor, const std::string& year, 
    const std::string& sampleDirectory, const std::string& sampleList, const unsigned sampleIndex,
    const std::vector< std::string >& triggerVector, 
    const std::map< std::string, Prescale >& prescaleMap, double maxMT, double maxMet,
    const bool isTestRun, const unsigned long nEvents ){ 
 
    std::cout<<"start function fillFakeRateMeasurementHistograms"<<std::endl;

    progressTracker progress = progressTracker("fillFakeRateMeasurement_progress_"+year+"_"
                                +leptonFlavor+"_sample_"+std::to_string(sampleIndex)+".txt");

    // initialize flavor and year
    fakeRate::checkFlavorString( leptonFlavor );
    bool isMuonMeasurement = ( leptonFlavor == "muon" );
    analysisTools::checkYearString( year );

    // check if all triggers have a prescale defined
    for( const auto& trigger : triggerVector ){
        if( prescaleMap.find( trigger ) == prescaleMap.end() ){
	    std::string errorm("Given vector of triggers contains triggers");
	    errorm.append("that are not present in the given prescale map.");
            throw std::invalid_argument(errorm);
        }
    }

    // define binning
    std::vector< double > ptBinBorders;
    std::vector< double > etaBinBorders;
    if( isMuonMeasurement ){
        etaBinBorders = {0., 1.2, 2.1};
        //ptBinBorders = { 10, 15, 20, 30, 45 };
	ptBinBorders = { 10, 20, 30, 45 };
    } else{
        etaBinBorders = {0., 0.8, 1.442};
        //ptBinBorders = { 10, 15, 20, 30, 45 };
	ptBinBorders = {10, 20, 30, 45 };
    }

    // initialize TreeReader and set to correct sample
    std::cout<<"initializing TreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleList , sampleDirectory );
    treeReader.initSample();
    for(unsigned idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    const bool isData = treeReader.isData();

    // make histogram maps
    std::cout<<"start building histogram maps"<<std::endl;
    unsigned numberOfMTBins = 16;
    HistInfo mtHistInfo( "mT", "m_{T}( GeV )", numberOfMTBins, 0., 160. );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, 
	treeReader.currentSamplePtr()->processName() + "_prompt_numerator_mT_" 
	+ year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, 
	treeReader.currentSamplePtr()->processName() + "_prompt_denominator_mT_" 
	+ year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, 
	treeReader.currentSamplePtr()->processName() + "_nonprompt_numerator_mT_" 
	+ year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, 
	treeReader.currentSamplePtr()->processName() + "_nonprompt_denominator_mT_" 
	+ year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "data_numerator_mT_" + year + "_" + leptonFlavor);
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "data_denominator_mT_" + year + "_" + leptonFlavor );

    
    // do some pt-to-trigger matching
    RangedMap< std::string > leptonPtToTriggerMap = fakeRate::mapLeptonPtToTriggerName( 
						    triggerVector, isMuonMeasurement );
    std::map< double, std::string > conePtLowerBoundMap;
    for( auto it = leptonPtToTriggerMap.cbegin(); it != leptonPtToTriggerMap.cend(); ++it ){
        double conePtBound;
        if( it == leptonPtToTriggerMap.cbegin() ){conePtBound = it->first;} 
	else {
            if( isMuonMeasurement ){conePtBound = 2*it->first;} 
	    else {conePtBound = 1.5*it->first;}
        }
        conePtLowerBoundMap[ conePtBound ] = it->second;
    }
    RangedMap< std::string > conePtToTriggerMap( conePtLowerBoundMap );
    std::map<std::string,double> triggerToJetPtMap = fakeRate::mapTriggerToJetPtThreshold(triggerVector);
    
    // initialize a reweighter
    std::cout<<"building reweighter"<<std::endl;
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/", year, 
					thissample );

    // set number of entries
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    double nEventsReweight = 1.;
    if( isTestRun ){
        // loop over a smaller number of entries for testing and debugging
        unsigned long nLimit = 10000;
        std::cout << "limiting number of entries because of test run setting" << std::endl;
        numberOfEntries = std::min(nLimit, numberOfEntries);
    }
    if( nEvents!=0 && nEvents<numberOfEntries && !isData ){
        // loop over a smaller number of entries if samples are impractically large
        std::cout << "limiting number of entries to " << nEvents << std::endl;
        nEventsReweight = (double)numberOfEntries/nEvents;
        std::cout << "(with corresponding reweighting factor " << nEventsReweight << ")" << std::endl;
        numberOfEntries = nEvents;
    }

    // do event loop
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    for(long unsigned entry=0; entry<numberOfEntries; ++entry){
	if( entry%50000 == 0 ) progress.writeProgress( static_cast<double>(entry)/numberOfEntries );

	Event event = treeReader.buildEvent( entry, true, false );

	// apply MET filters
	if( !event.passMetFilters() ) continue; 

	// apply event selection
	if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement, 
		!isMuonMeasurement, false, true, 0.7, 30 ) ) continue;
        LightLepton& lepton = event.lightLepton( 0 );
	if( lepton.pt() < 10 ) continue;

	const double pTFix = 35.;
        PhysicsObject leptonFix( pTFix, lepton.eta(), lepton.phi(), lepton.energy(),
				    lepton.is2016(), lepton.is2016PreVFP(), lepton.is2016PostVFP(),
				    lepton.is2017(), lepton.is2018() );
        double mT = mt( leptonFix, event.met() );

	if( mT >= maxMT ) continue;
        if( event.metPt() >= maxMet ) continue;

	// check if event passes correct trigger and jet selection
	std::string triggerToUse = conePtToTriggerMap[ lepton.pt() ];
        if( !event.passTrigger( triggerToUse ) ) continue;
	if( !fakeRate::passTriggerJetSelection( event, triggerToUse, triggerToJetPtMap ) ) continue;

	// determine correct event weight
	double weight = event.weight();
	    
	// temp: remove events with unphysical large weights
        // issue with unknown cause observed in a small number of generator weights 
        // in UL WJets samples
        if( weight>1e4 ){
            std::string msg = "WARNING: vetooing event with large weight.";
            msg += " (weight is " + std::to_string(weight) + ")";
            std::cerr << msg << std::endl;
            continue;
        }

	weight *= nEventsReweight;
        
	if( event.isMC() ){
            const Prescale& prescale = prescaleMap.find( triggerToUse )->second;
            weight *= prescale.value();
            weight *= reweighter.totalWeight( event );
        }
	else weight = 1;

	// determine whether lepton is prompt or nonprompt (for MC)
	bool isPrompt = false;
	if( event.isMC() ){ 
	    if( lepton.isPrompt() && treeReader.currentSamplePtr()->processName()!="QCD" ){
		// manually set all leptons in QCD samples to nonprompt!
		isPrompt = true;
	    }
	}

	// fill numerator histograms
	if( lepton.isTight() ){
	    if( treeReader.isData() ){
                data_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            } else if( isPrompt ){
                prompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            } else {
                nonprompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            }
        }

	// fill denominator histograms
	if( treeReader.isData() ){
            data_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		std::min( mT, mtHistInfo.maxBinCenter() ), weight );
        } else if( isPrompt ){
            prompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( 
		mT, mtHistInfo.maxBinCenter() ), weight );
        } else {
            nonprompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		std::min( mT, mtHistInfo.maxBinCenter() ), weight );
        }
    }

    // write output file
    progress.close();
    std::cout<<"finished event loop"<<std::endl;
    std::string file_name = "fakeRateMeasurement_data_" + leptonFlavor + "_" + year;
    file_name.append("_mT_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );
    
    if( treeReader.isMC() ){
	write2DHistogramMap( prompt_numerator_map );
	write2DHistogramMap( prompt_denominator_map );
	write2DHistogramMap( nonprompt_numerator_map );
	write2DHistogramMap( nonprompt_denominator_map );
    }
    if( treeReader.isData() ){
	write2DHistogramMap( data_numerator_map );
	write2DHistogramMap( data_denominator_map );
    }

    histogram_file->Close();
    std::cout<<"finished function fillFakeRateMeasurementHistograms"<<std::endl;
}  


void fillMCFakeRateMeasurementHistograms( const std::string& flavor, const std::string& year, 
					    const std::string& sampleDirectory, 
					    const std::string& sampleList,
					    const unsigned sampleIndex,
					    const bool isTestRun ){

    std::cout<<"starting function fillMCFakeRateMeasurementHistograms"<<std::endl;
    fakeRate::checkFlavorString( flavor );
    analysisTools::checkYearString( year );
    const bool isMuon =  ( flavor == "muon" );
    
    // define binning for 2D histograms
    // old binning:
    const std::vector< double > ptBins = {10., 20., 30., 45., 65., 100.};
    // new binning:
    //const std::vector< double > ptBins = {10., 15., 20., 30., 45., 100.};
    std::vector< double > etaBins;
    if( isMuon ){
        etaBins = { 0., 1.2, 2.1, 2.4 }; 
    } else {
	etaBins = { 0., 0.8, 1.442, 2.5 };
    }

    // initialize 2D histograms for numerator and denominator
    std::string numerator_name = "fakeRate_numerator_" + flavor + "_" + year;
    std::shared_ptr< TH2D > numeratorMap( 
	new TH2D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV); |#eta|").c_str(), 
		    ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) 
    );
    numeratorMap->Sumw2();
    std::string denominator_name = "fakeRate_denominator_" + flavor + "_" + year;
    std::shared_ptr< TH2D > denominatorMap( 
	new TH2D(   denominator_name.c_str(), denominator_name.c_str(), 
		    ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) 
    );
    denominatorMap->Sumw2();

    // initialize 1D histograms for light and heavy flavour fake rate separately
    // (can be a useful tool to check if they are approximately equal, but not used further)
    // extension: split heavy flavour in b- and c-flavoured partons for more detailed diagnostics
    unsigned nbins = 10;
    double ptlow = 10.;
    double pthigh = 70.;
    numerator_name = "fakeRate_numerator_bflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > bflavorNumerator(
        new TH1D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV)").c_str(),
                    nbins, ptlow, pthigh)
    );
    bflavorNumerator->Sumw2();
    denominator_name = "fakeRate_denominator_bflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > bflavorDenominator(
        new TH1D(   denominator_name.c_str(), denominator_name.c_str(),
                    nbins, ptlow, pthigh)
    );
    bflavorDenominator->Sumw2();

    numerator_name = "fakeRate_numerator_cflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > cflavorNumerator(
        new TH1D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV)").c_str(),
                    nbins, ptlow, pthigh)
    );
    cflavorNumerator->Sumw2();
    denominator_name = "fakeRate_denominator_cflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > cflavorDenominator(
        new TH1D(   denominator_name.c_str(), denominator_name.c_str(),
                    nbins, ptlow, pthigh)
    );
    cflavorDenominator->Sumw2();

    numerator_name = "fakeRate_numerator_lightflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > lightNumerator(
        new TH1D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV)").c_str(),
                    nbins, ptlow, pthigh)
    );
    lightNumerator->Sumw2();
    denominator_name = "fakeRate_denominator_lightflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > lightDenominator(
        new TH1D(   denominator_name.c_str(), denominator_name.c_str(),
                    nbins, ptlow, pthigh)
    );
    lightDenominator->Sumw2();

    // make TreeReader and set to correct sample
    std::cout<<"making TreeReader and setting to sample no. "<<sampleIndex<<"."<<std::endl;
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i = 1; i <= sampleIndex; ++i ){
        treeReader.initSample();
    }

    // loop over events to fill histograms
    unsigned numberOfEntries = treeReader.numberOfEntries();
    if( isTestRun){ numberOfEntries = 5000; }
    std::cout<<"start event loop for "<<numberOfEntries<<" events."<<std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry );
	if( isTestRun ){
	    /*std::cout << "-------------------------" << std::endl;
	    std::cout << "event ID: " << event.runNumber() << "/" << event.luminosityBlock();
	    std::cout << "/" << event.eventNumber() << std::endl;
	    for( auto lepton : event.leptonCollection() ){
		lepton->print();
		std::cout << std::endl;
	    }*/
	}

	// apply MET filters (not included in passFakeRateEventSelection!)
	if( !event.passMetFilters() ) continue;

        // apply fake-rate selection
	// arguments: event, onlyMuons, onlyElectrons, onlyTight, requireJet, deltaR, jetPt (def 25) 
        if( !fakeRate::passFakeRateEventSelection( event, isMuon, !isMuon, false, true, 0.7) ){
	    continue;
	}

        LightLepton& lepton = event.lightLeptonCollection()[ 0 ];

        // lepton should be nonprompt
        if( lepton.isPrompt() ) continue;
	// note: in the line below leptons from photons were excluded, 
	// but this doesn't seem to be correct
        //if( lepton.matchPdgId() == 22 ) continue;

	if( isTestRun ){
	    //std::cout << "event passed FO selection" << std::endl;
	    std::cout << "event ID: " << event.runNumber() << "/" << event.luminosityBlock();
            std::cout << "/" << event.eventNumber() << std::endl;
	    std::cout << "(scaled) weight: " << event.weight() << std::endl;
	    //if( event.eventNumber()==3088713 ){
		//std::cout << lepton << std::endl;
	    //}
	}

	double weight = event.weight();

        // fill 2D denominator histogram 
        histogram::fillValues( denominatorMap.get(), lepton.pt(), lepton.absEta(), weight );
    
        // fill 2D numerator histogram
        if( lepton.isTight() ){
            histogram::fillValues(numeratorMap.get(), lepton.pt(), lepton.absEta(), weight );
        }
    
	// fill b-flavour 1D histograms
	if(lepton.provenanceCompressed()==1){
	    bflavorDenominator->Fill(lepton.pt(), weight);
	    if(lepton.isTight()) bflavorNumerator->Fill(lepton.pt(), weight);
	}

	// fill c-flavour 1D histograms
	else if(lepton.provenanceCompressed()==2){
            cflavorDenominator->Fill(lepton.pt(), weight);
            if(lepton.isTight()) cflavorNumerator->Fill(lepton.pt(), weight);
        }

	// fill light flavour 1D histograms
	else{
	    lightDenominator->Fill(lepton.pt(), weight);
            if(lepton.isTight()) lightNumerator->Fill(lepton.pt(), weight);
	}
    }

    std::cout<<"finished event loop"<<std::endl;

    std::string file_name = "fakeRateMeasurement_MC_" + flavor + "_" + year;
    file_name.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );

    numeratorMap->Write();
    denominatorMap->Write();

    bflavorNumerator->Write();
    bflavorDenominator->Write();
    cflavorNumerator->Write();
    cflavorDenominator->Write();
    lightNumerator->Write();
    lightDenominator->Write();

    histogram_file->Close();
    std::cout<<"finished function fillMCFakeRateMeasurementHistograms"<<std::endl;
}
