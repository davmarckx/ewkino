

//include c++ library classes

//include ROOT classes 
#include "TTree.h"

//include general parts of framework
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/HistInfo.h"
#include "../weights/interface/ConcreteReweighterFactory.h"
#include "../Tools/interface/SusyScan.h"
#include "../Tools/interface/histogramTools.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "../Tools/interface/KerasModelReader.h"

//include ewkino specific code
#include "interface/ewkinoSelection.h"
#include "interface/ewkinoCategorization.h"
#include "interface/EwkinoXSections.h"
#include "interface/ewkinoVariables.h"
#include "interface/ewkinoSearchRegions.h"


//compare floating points
bool floatEqual( const double lhs, const double rhs ){
    return ( fabs( ( lhs - rhs ) / lhs ) < 1e-6 );
}


//build histograms 
std::vector< HistInfo > makeDistributionInfo( const std::string& deltaM, const std::string& controlRegion ){

    //make general plots
    std::vector< HistInfo > histInfoVec;
    if( deltaM == "None" ){
        histInfoVec = {

            HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 18, 25, 205 ),
            HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 14, 15, 155 ),
            HistInfo( "leptonPtTrailing", "P_{T}^{trailing lepton} (GeV)", 14, 15, 155 ),

            HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
            HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),
            HistInfo( "leptonEtaTrailing", "|#eta|^{trailing lepton}", 10, 0, 2.5 ),

            ( controlRegion == "WZ" ? HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 30, 100 ) : HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 0, 200 ) ),
            ( controlRegion == "WZ" ? HistInfo( "mt", "M_{T}^{W} (GeV)", 10, 50, 100 ) : HistInfo( "mt", "M_{T}^{W} (GeV)", 10, 0, 200 ) ),
            ( ( controlRegion == "TTZ" || controlRegion == "WZ" ) ? HistInfo( "mll", "M_{ll} (GeV)", 10, 75, 105 ) :  HistInfo( "mll", "M_{ll} (GeV)", 10, 0, 200 ) ),
            HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 800 ),
            HistInfo( "ht", "H_{T} (GeV)", 10, 0, 800 ),
            HistInfo( "m3l", "M_{3l} (GeV)", 10, 0, 400 ),
            HistInfo( "mt3l", "M_{T}^{3l} (GeV)", 10, 0, 600 ),

            HistInfo( "nJets", "number of jets", 8, 0, 8 ),
            HistInfo( "nBJets", "number of b-jets (medium deep CSV)", 5, 0, 5 ),
            HistInfo( "nVertex", "number of vertices", 30, 0, 70 )
        };
    } else {
        histInfoVec = {
            HistInfo( "neuralNet", "neural network output", 10, 0, 1 )
        };
    }
    return histInfoVec;
}


std::vector< double > buildFillingVector( Event& event, const std::string& uncertainty, const double massSplitting, const KerasModelReader* nnReader ){
    
    auto varMap = ewkino::computeVariables( event, uncertainty );
    std::vector< double > fillValues;
    //nullptr indicates general plots
    if( nnReader == nullptr ){
        fillValues = {
            event.lepton( 0 ).pt(),
            event.lepton( 1 ).pt(),
            event.lepton( 2 ).pt(),
            event.lepton( 0 ).absEta(),
            event.lepton( 1 ).absEta(),
            event.lepton( 2 ).absEta(),
            varMap.at("met"),
            varMap.at("mtW"),
            varMap.at("mll"),
            varMap.at("ltmet"),
            varMap.at("ht"),
            varMap.at("m3l"),
            varMap.at("mt3l"),
            varMap.at("numberOfJets"),
            varMap.at("numberOfBJets"),
            static_cast< double >( event.numberOfVertices() )
        };
    
    //otherwise plot neural network
    } else {
        std::vector< double > nnInput = { varMap.at("met"), varMap.at("mll"), varMap.at("mtW"), varMap.at("ltmet"), varMap.at("ht"), varMap.at("m3l"), varMap.at("mt3l") };
        std::vector< double > parameters = { massSplitting };
        double nnOutput = nnReader->predict( nnInput, parameters );
        fillValues = { nnOutput };
    
    }
    return fillValues;
}






void analyze( const std::string& modelName, const std::string& deltaM, const std::string& year, const std::string& controlRegion, const std::string& sampleDirectoryPath ){

	analysisTools::checkYearString( year );

    //build neural network reader
    KerasModelReader* nnReader = nullptr;
    if( deltaM != "None" ){
        if( modelName == "TChiWZ" ){
            nnReader = new KerasModelReader( "kerasModels/TChiWZ/prelu_batchSize=2048_batchnormFirst_batchnormHidden_dropoutAll_dropoutRate=0p5_learningRate=1_learningRateDecay=1_numberOfEpochs=500_numberOfHiddenLayers=3_Nadam_unitsPerLayer=256.h5", 7, true, 1 );
        } else if( modelName == "TChiSlepSnu_x0p95" ){
            nnReader = new KerasModelReader( "kerasModels/TChiSlepSnu_x0p95/prelu_batchSize=2048_batchnormFirst_batchnormHidden_dropoutAll_dropoutRate=0p5_learningRate=1_learningRateDecay=1_numberOfEpochs=2000_numberOfHiddenLayers=5_Nadam_unitsPerLayer=256.h5", 7, true, 1 );
        } else if( modelName == "TChiSlepSnu_x0p5" ){
            nnReader = new KerasModelReader( "kerasModels/TChiSlepSnu_x0p5/prelu_batchSize=2048_batchnormFirst_batchnormHidden_dropoutAll_dropoutRate=0p5_learningRate=1_learningRateDecay=1_numberOfEpochs=2000_numberOfHiddenLayers=3_Nadam_unitsPerLayer=256.h5", 7, true, 1 );
        } else if( modelName == "TChiSlepSnu_x0p05" ){
            nnReader = new KerasModelReader( "kerasModels/TChiSlepSnu_x0p05/prelu_batchSize=2048_batchnormFirst_batchnormHidden_dropoutAll_dropoutRate=0p5_learningRate=1_learningRateDecay=1_numberOfEpochs=2000_numberOfHiddenLayers=5_Nadam_unitsPerLayer=128.h5", 7, true , 1 );
        } else {
            throw std::invalid_argument( "Model " + modelName + " is unknown." );
        }
    }

    //selection that defines the control region
    const std::map< std::string, std::function< bool (Event&, const std::string&) > > crSelectionFunctionMap{
        { "WZ", ewkino::passVariedSelectionWZCR },
        { "XGamma", ewkino::passVariedSelectionXGammaCR },
        { "TTZ", ewkino::passVariedSelectionTTZCR },
        { "NP", ewkino::passVariedSelectionNPCR }
    };
    auto passSelection = crSelectionFunctionMap.at( controlRegion );



    //initialize mass splitting
    double massSplitting;
    if( deltaM != "None" ){
        massSplitting = std::stod( deltaM );
    } else {
        massSplitting = 0;
    }

    //build TreeReader and loop over samples
    std::cout << "building treeReader" << std::endl;
    TreeReader treeReader( "sampleLists/samples_" + modelName + "_" + year + ".txt", sampleDirectoryPath );
    treeReader.removeBSMSignalSamples();

    //build ewkino reweighter
    std::cout << "building reweighter" << std::endl;
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/", year, treeReader.sampleVector() );

    //read FR maps
    std::cout << "building FR maps" << std::endl;
    TFile* frFileMuons = TFile::Open( ( "frMaps/fakeRateMap_data_muon_" + year + "_mT.root" ).c_str() );
    std::shared_ptr< TH2 > frMapMuons = std::shared_ptr< TH2 >( dynamic_cast< TH2* >( frFileMuons->Get( ( "fakeRate_muon_" + year ).c_str() ) ) );
    frMapMuons->SetDirectory( gROOT );
    frFileMuons->Close();

    TFile* frFileElectrons = TFile::Open( ( "frMaps/fakeRateMap_data_electron_" + year + "_mT.root" ).c_str() );
    std::shared_ptr< TH2 > frMapElectrons = std::shared_ptr< TH2 >( dynamic_cast< TH2* >( frFileElectrons->Get( ( "fakeRate_electron_" + year ).c_str() ) ) );
    frMapElectrons->SetDirectory( gROOT );
    frFileElectrons->Close();

    //histogram collection
    std::cout << "building histograms" << std::endl;
    std::vector< HistInfo > histInfoVector = makeDistributionInfo( deltaM, controlRegion  );

    //make histograms for each process, and integral signal to check shapes
    //add an additional histogram for the nonprompt prediction
    std::vector< Sample > sampleVec = treeReader.sampleVector();
    std::vector< std::vector< std::shared_ptr< TH1D > > > histograms( histInfoVector.size(), std::vector< std::shared_ptr< TH1D > >( sampleVec.size() + 1 )  );
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
        for( size_t p = 0; p < sampleVec.size() + 1; ++p ){
            if( p < sampleVec.size() ){
                histograms[ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_" + sampleVec[p].uniqueName() );
            } else {
                histograms[ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_nonprompt" );
            }
        }
    }

    const std::vector< std::string > shapeUncNames = { "JEC_" + year, "JER_" + year, "uncl", "scale", "pileup", "bTag_" + year, "prefire", "lepton_reco", "lepton_id"}; //, "pdf" }; //"scaleXsec", "pdfXsec" }
    std::map< std::string, std::vector< std::vector< std::shared_ptr< TH1D > > > > histogramsUncDown;
    std::map< std::string, std::vector< std::vector< std::shared_ptr< TH1D > > > > histogramsUncUp;
    for( const auto& unc : shapeUncNames ){
        histogramsUncDown[ unc ] = std::vector< std::vector< std::shared_ptr< TH1D > > >( histInfoVector.size(), std::vector< std::shared_ptr< TH1D > >( sampleVec.size() + 1 )  );
        histogramsUncUp[ unc ] = std::vector< std::vector< std::shared_ptr< TH1D > > >( histInfoVector.size(), std::vector< std::shared_ptr< TH1D > >( sampleVec.size() + 1 )  );
        
        for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
            for( size_t p = 0; p < sampleVec.size() + 1; ++p ){
                if( p < sampleVec.size() ){
                    histogramsUncDown[ unc ][ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_" + sampleVec[p].uniqueName() + unc + "Down" );
                    histogramsUncUp[ unc ][ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_" + sampleVec[p].uniqueName() + unc + "Up" );
                } else {
                    histogramsUncDown[ unc ][ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_nonprompt"  + unc + "Down" );
                    histogramsUncUp[ unc ][ dist ][ p ] = histInfoVector[ dist ].makeHist( histInfoVector[ dist ].name() + "_nonprompt" + unc + "Up" );
                }
            }
        }
    }

    std::cout << "event loop" << std::endl;

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){
        treeReader.initSample();

        if( treeReader.isSusy() ) continue;

        std::cout << treeReader.currentSample().fileName() << std::endl;

        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );

            //apply baseline selection
            if( !ewkino::passBaselineSelection( event, true, false, true ) ) continue;


            //apply lepton pT cuts
            if( !ewkino::passPtCuts( event ) ) continue;

            //require triggers
            if( !treeReader.isSusy() && !ewkino::passTriggerSelection( event ) ) continue;

            //remove photon overlap
            if( !ewkino::passPhotonOverlapRemoval( event ) ) continue;

            //require MC events to only contain prompt leptons
            if( event.isMC() && !treeReader.isSusy() && !ewkino::leptonsArePrompt( event ) ) continue;

            ewkino::EwkinoCategory category = ewkino::ewkinoCategory( event );
            if( !( category == ewkino::trilepLightOSSF || category == ewkino::trilepLightNoOSSF ) ) continue;
            
            //apply scale-factors and reweighting
            double weight = event.weight();
            if( event.isMC() ){
                weight *= reweighter.totalWeight( event );
            }

            //apply fake-rate weight
            size_t fillIndex = sampleIndex;
            if( !ewkino::leptonsAreTight( event ) && !treeReader.isSusy() ){
                fillIndex = treeReader.numberOfSamples();
                weight *= ewkino::fakeRateWeight( event, frMapMuons, frMapElectrons );
                if( event.isMC() ) weight *= -1.;
            }

            //fill nominal histograms
            //if( assVariedSelectionWZCR( event, "nominal" ) ){
            if( passSelection( event, "nominal" ) ){
                auto fillValues = buildFillingVector( event, "nominal", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histograms[ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }

                //in case of data fakes fill all uncertainties for nonprompt with nominal values
                if( event.isData() && ( fillIndex == treeReader.numberOfSamples() ) ){
                    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                        for( const auto& key : shapeUncNames ){
                            histogram::fillValue( histogramsUncDown[ key ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                            histogram::fillValue( histogramsUncUp[ key ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );       
                        }
                    }
                }

            }

            //no uncertainties for data
            if( event.isData() ) continue;
            
            //fill JEC down histograms
            if( passSelection( event, "JECDown" ) ){
                auto fillValues = buildFillingVector( event, "JECDown", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncDown[ "JEC_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }

            //fill JEC up histograms
            if( passSelection( event, "JECUp" ) ){
                auto fillValues = buildFillingVector( event, "JECUp", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncUp[ "JEC_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }

            //fill JER down histograms
            if( passSelection( event, "JERDown" ) ){
                auto fillValues = buildFillingVector( event, "JERDown", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncDown[ "JER_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }

            //fill JER up histograms
            if( passSelection( event, "JERUp" ) ){
                auto fillValues = buildFillingVector( event, "JERUp", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncUp[ "JER_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }

            //fill unclustered down histograms
            if( passSelection( event, "UnclDown" ) ){
                auto fillValues = buildFillingVector( event, "UnclDown", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncDown[ "uncl" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }

            //fill unclustered up histograms 
            if( passSelection( event, "UnclUp" ) ){
                auto fillValues = buildFillingVector( event, "UnclUp", massSplitting, nnReader );
                for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                    histogram::fillValue( histogramsUncUp[ "uncl" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight );
                }
            }
            
            //apply nominal selection and compute nominal variables
            if( !passSelection( event, "nominal" ) ) continue;
            auto fillValues = buildFillingVector( event, "nominal", massSplitting, nnReader );

            //fill scale down histograms
            double weightScaleDown;
            try{
                weightScaleDown =  event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5();
            } catch( std::out_of_range& ){
                weightScaleDown = 1.;
            }
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "scale" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightScaleDown );
            }
        
            //fill scale up histograms
            double weightScaleUp;
            try{
                weightScaleUp = event.generatorInfo().relativeWeight_MuR_2_MuF_2();
            } catch( std::out_of_range& ){
                weightScaleUp = 1.;
            }
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "scale" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightScaleUp );
            }

            //fill pileup down histograms
            double weightPileupDown = reweighter[ "pileup" ]->weightDown( event ) / reweighter[ "pileup" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "pileup" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightPileupDown );
            }

            //fill pileup up histograms
            double weightPileupUp = reweighter[ "pileup" ]->weightUp( event ) / reweighter[ "pileup" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "pileup" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightPileupUp );
            }

            //fill b-tag down histograms
            //WARNING : THESE SHOULD ACTUALLY BE SPLIT BETWEEN HEAVY AND LIGHT FLAVORS
            double weightBTagDown = reweighter[ "bTag" ]->weightDown( event ) / reweighter[ "bTag" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "bTag_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightBTagDown );
            }

            //fill b-tag up histograms
            //WARNING : THESE SHOULD ACTUALLY BE SPLIT BETWEEN HEAVY AND LIGHT FLAVORS
            double weightBTagUp = reweighter[ "bTag" ]->weightUp( event ) / reweighter[ "bTag" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "bTag_" + year ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightBTagUp );
            }

            //fill prefiring down histograms
            double weightPrefireDown = reweighter[ "prefire" ]->weightDown( event ) / reweighter[ "prefire" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "prefire" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightPrefireDown );
            }
        
            //fill prefiring up histograms
            double weightPrefireUp = reweighter[ "prefire" ]->weightUp( event ) / reweighter[ "prefire" ]->weight( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "prefire" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * weightPrefireUp );
            }

            double recoWeightDown;
            double recoWeightUp;
            if( !event.is2018() ){
                recoWeightDown = reweighter[ "electronReco_pTBelow20" ]->weightDown( event ) * reweighter[ "electronReco_pTAbove20" ]->weightDown( event ) / ( reweighter[ "electronReco_pTBelow20" ]->weight( event ) * reweighter[ "electronReco_pTAbove20" ]->weight( event ) );
                recoWeightUp = reweighter[ "electronReco_pTBelow20" ]->weightUp( event ) * reweighter[ "electronReco_pTAbove20" ]->weightUp( event ) / ( reweighter[ "electronReco_pTBelow20" ]->weight( event ) * reweighter[ "electronReco_pTAbove20" ]->weight( event ) );
            } else {
                recoWeightDown = reweighter[ "electronReco" ]->weightDown( event ) / ( reweighter[ "electronReco" ]->weight( event ) );
                recoWeightUp = reweighter[ "electronReco" ]->weightUp( event ) / ( reweighter[ "electronReco" ]->weight( event ) );
            }

            //fill lepton reco down histograms 
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "lepton_reco" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * recoWeightDown );
            }

            //fill lepton reco up histograms 
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "lepton_reco" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * recoWeightUp );
            }

            double leptonIDWeightDown = reweighter[ "muonID" ]->weightDown( event ) * reweighter[ "electronID" ]->weightDown( event ) / ( reweighter[ "muonID" ]->weight( event ) * reweighter[ "electronID" ]->weight( event ) );
            double leptonIDWeightUp = reweighter[ "muonID" ]->weightUp( event ) * reweighter[ "electronID" ]->weightUp( event ) / ( reweighter[ "muonID" ]->weight( event ) * reweighter[ "electronID" ]->weight( event ) );

            //fill lepton id down histograms
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncDown[ "lepton_id" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * leptonIDWeightDown );
            }

            //fill lepton id up histograms
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramsUncUp[ "lepton_id" ][ dist ][ fillIndex ].get(), fillValues[ dist ], weight * leptonIDWeightUp );
            }
        }
    }

    //set negative contributions to zero
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
        
        //backgrounds, data and merged signal
        for( size_t p = 0; p < sampleVec.size() + 1; ++p ){
            analysisTools::setNegativeBinsToZero( histograms[ dist ][ p ] );

            for( const auto& unc : shapeUncNames ){
                analysisTools::setNegativeBinsToZero( histogramsUncDown[ unc ][ dist ][ p ] );
                analysisTools::setNegativeBinsToZero( histogramsUncUp[ unc ][ dist ][ p ] );
            }
        }
    }
        

    //merge process histograms
    std::vector< std::string > proc = {"Data", "WZ", "X + #gamma", "ZZ/H", "t#bar{t}/t + X", "Multiboson", "Nonprompt", };
    std::vector< std::vector< TH1D* > > mergedHistograms( histInfoVector.size(), std::vector< TH1D* >( proc.size() ) );
    //size_t numberOfBackgrounds = 0;
    //for( const auto& s : sampleVec ){
    //    if( !s.isNewPhysicsSignal() ) ++numberOfBackgrounds; 
    //}
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
        for( size_t m = 0, sample = 0; m < proc.size() - 1; ++m ){
            mergedHistograms[ dist ][ m ] = dynamic_cast< TH1D* >( histograms[ dist ][ sample ]->Clone() );
            //while( sample < numberOfBackgrounds - 1 && sampleVec[ sample ].processName() == sampleVec[ sample + 1 ].processName() ){
            while( sample < sampleVec.size() - 1 && sampleVec[ sample ].processName() == sampleVec[ sample + 1 ].processName() ){
                mergedHistograms[ dist ][ m ]->Add( histograms[ dist ][ sample + 1].get() );
                ++sample;
            }
			++sample;
        }
        
        //add nonprompt histogram
        mergedHistograms[ dist ][ proc.size() -1 ] = dynamic_cast< TH1D* >( histograms[ dist ].back()->Clone() );
    }

    //merge process histograms for uncertainties 
	std::map< std::string, std::vector< std::vector< TH1D* > > > mergedHistogramsUncDown;
	std::map< std::string, std::vector< std::vector< TH1D* > > > mergedHistogramsUncUp;

	for( const auto& unc : shapeUncNames ){
		mergedHistogramsUncDown[ unc ] = std::vector< std::vector< TH1D* > >( histInfoVector.size(), std::vector< TH1D* >( proc.size() ) );
		mergedHistogramsUncUp[ unc ] = std::vector< std::vector< TH1D* > >( histInfoVector.size(), std::vector< TH1D* >( proc.size() ) );
		for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
			for( size_t m = 0, sample = 0; m < proc.size() - 1; ++m ){
				mergedHistogramsUncDown[ unc ][ dist ][ m ] = dynamic_cast< TH1D* >( histogramsUncDown[ unc ][ dist ][ sample ]->Clone() );
				mergedHistogramsUncUp[ unc ][ dist ][ m ] = dynamic_cast< TH1D* >( histogramsUncUp[ unc ][ dist ][ sample ]->Clone() );
				//while( sample < numberOfBackgrounds - 1 && sampleVec[ sample ].processName() == sampleVec[ sample + 1 ].processName() ){
				while( sample < sampleVec.size() - 1 && sampleVec[ sample ].processName() == sampleVec[ sample + 1 ].processName() ){
					mergedHistogramsUncDown[ unc ][ dist ][ m ]->Add( histogramsUncDown[ unc ][ dist ][ sample + 1].get() );
					mergedHistogramsUncUp[ unc ][ dist ][ m ]->Add( histogramsUncUp[ unc ][ dist ][ sample + 1].get() );
					++sample;
				}
				++sample;
			}
			mergedHistogramsUncDown[ unc ][ dist ][ proc.size() - 1 ] = dynamic_cast< TH1D * >( histogramsUncDown[ unc ][ dist ].back()->Clone() );
			mergedHistogramsUncUp[ unc ][ dist ][ proc.size() - 1 ] = dynamic_cast< TH1D * >( histogramsUncUp[ unc ][ dist ].back()->Clone() );
		}
	}

    //make total uncertainty histograms for plotting
	const std::vector< std::string > uncorrelatedBetweenProcesses = {"scale", "pdf", "scaleXsec", "pdfXsec"};
	double lumiUncertainty = 1.025;
    std::vector<double> flatUnc = { lumiUncertainty, 1.02 }; //lumi, trigger
    std::map< std::string, double > backgroundSpecificUnc;
    if( controlRegion == "TTZ" || controlRegion == "NP" ){
        backgroundSpecificUnc = {
	    	{"Nonprompt", 1.3},
	    	{"WZ", 1.1},
	    	{"X + #gamma", 1.1},
	    	{"ZZ/H", 1.1},
	    	{"t#bar{t}/t + X", 1.15 },
	    	{"Multiboson", 1.5}
	    };
    } else {
        backgroundSpecificUnc = {
	    	{"Nonprompt", 1.3},
	    	{"WZ", 1.1},
	    	{"X + #gamma", 1.1},
	    	{"ZZ/H", 1.1},
	    	{"t#bar{t}/t + X", 1.5 },
	    	{"Multiboson", 1.5}
	    };
    }

    //const std::set< std::string > acceptedShapes = { "JEC_" + year, "JER_" + year, "uncl", "scale", "pileup", "bTag_" + year, "prefire", "lepton_reco", "lepton_id"}
    //const std::set< std::string > acceptedShapes = { "JEC_" + year, "scale", "bTag_" + year, "prefire", "lepton_reco", "lepton_id"};
    //const std::set< std::string > acceptedShapes = { "JEC_" + year };

	std::vector< TH1D* > totalSystUncertainties( histInfoVector.size() );
	for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
		totalSystUncertainties[ dist ] = dynamic_cast< TH1D* >( mergedHistograms[ dist ][ 0 ]->Clone() );
		for( int bin = 1; bin < totalSystUncertainties[ dist ]->GetNbinsX() + 1; ++bin ){
			double binUnc = 0;

			//add shape uncertainties
			for( auto& shape : shapeUncNames ){
                //if( acceptedShapes.find( shape ) == acceptedShapes.cend() ) continue;
				bool nuisanceIsUncorrelated = ( std::find( uncorrelatedBetweenProcesses.cbegin(), uncorrelatedBetweenProcesses.cend(), shape ) != uncorrelatedBetweenProcesses.cend() );

				//correlated case : linearly add up and down variations
				double varDown = 0.;
				double varUp = 0.;

				//uncorrelated case : quadratically add the maximum of the up and down variations
				double var = 0.;

				for( size_t p = 1; p < proc.size(); ++p ){
					double nominalContent = mergedHistograms[ dist ][ p ]->GetBinContent( bin );
					double downVariedContent = mergedHistogramsUncDown[ shape ][ dist ][ p ]->GetBinContent( bin );
					double upVariedContent = mergedHistogramsUncUp[ shape ][ dist ][ p ]->GetBinContent( bin );
					double down = fabs( downVariedContent - nominalContent );
					double up = fabs( upVariedContent - nominalContent );
                    
					//uncorrelated case : 
					if( nuisanceIsUncorrelated ){
					    double variation = std::max( down, up );
					    var += variation*variation;
					
					//correlated case :     
					} else {
					    varDown += down;
					    varUp += up;
					}

				}
				//correlated case : 
				if( !nuisanceIsUncorrelated ){
				    var = std::max( varDown, varUp );
				    var = var*var;
				}
				
				//add (already quadratic) uncertainties 
				binUnc += var;
			}

			//add general flat uncertainties (considered correlated among all processes)
            for( double unc : flatUnc ){
                double var = 0;
                for( size_t p = 1; p < proc.size(); ++p ){
                    if( proc[p] == "Nonprompt" ){
                        continue;
                    }
                    double binContent = mergedHistograms[ dist ][ p ]->GetBinContent( bin );
                    double variation = binContent*(unc - 1.);
                    var += variation;
                }
                binUnc += var*var;
            }

            //add background specific uncertainties (uncorrelated between processes)
            for(auto& uncPair : backgroundSpecificUnc){
                for( size_t p = 1; p < proc.size(); ++p ){
                    if(proc[p] == uncPair.first){
                        double var = mergedHistograms[ dist ][ p ]->GetBinContent( bin )*( uncPair.second - 1. );
                        binUnc += var*var;
                    }
                }
            }

            //square root of quadratic sum is total uncertainty
        	totalSystUncertainties[ dist ]->SetBinContent( bin, sqrt( binUnc ) );
		}
	}
	
    //make plots
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
        std::string header;
        if( year == "2016" ){
            header = "35.9 fb^{-1} (13 TeV)";
        } else if( year == "2017" ){
            header = "41.5 fb^{-1} (13 TeV)";
        } else {
            header = "59.7 fb^{-1} (13 TeV)";
        }
        std::string directoryName;
        std::string plotNameAddition;
        if( deltaM == "None" ){
            directoryName = stringTools::formatDirectoryName( "plots/ewkino/" + year + "/" + controlRegion );
            plotNameAddition = "_" + controlRegion + "_" + year;
        } else {
            directoryName = stringTools::formatDirectoryName( "plots/ewkino/" + year + "/" + controlRegion + "/" + modelName + "/" + deltaM );
            plotNameAddition = "_" + controlRegion + "_" + modelName + "_" + deltaM + "_" + year;
        }
        systemTools::makeDirectory( directoryName );
        plotDataVSMC( mergedHistograms[dist][0], &mergedHistograms[dist][1], &proc[0], proc.size() - 1, directoryName + histInfoVector[ dist ].name() + plotNameAddition + ".pdf" , "ewkino", false, false, header, totalSystUncertainties[ dist ], nullptr );
        plotDataVSMC( mergedHistograms[dist][0], &mergedHistograms[dist][1], &proc[0], proc.size() - 1, directoryName + histInfoVector[ dist ].name() + plotNameAddition + "_log.pdf" , "ewkino", true, false, header, totalSystUncertainties[ dist ], nullptr );
    }
}


void analyzeAllMasses( const std::string& modelName, const std::string& year, const std::string& controlRegion, const std::string& sampleDirectoryPath ){
    TreeReader treeReader( "sampleLists/samples_" + modelName + "_" + year + ".txt", sampleDirectoryPath );
    SusyScan susyScan;
    for( const auto& sample : treeReader.sampleVector() ){
        if( sample.isNewPhysicsSignal() ){
            susyScan.addScan( sample );
        }
    }

    for( auto splitting : susyScan.massSplittings() ){
        std::string deltaM = std::to_string( splitting );
        std::string commandString = "./controlRegions " + modelName + " " + deltaM + " " + year + " " + controlRegion;
        std::string scriptName = controlRegion + "_" + modelName + "_" + deltaM + "_" + year + ".sh";
        systemTools::submitCommandAsJob( commandString, scriptName, "100:00:00" );
    }
}


void analyzeNominal( const std::string& year, const std::string& controlRegion ){
    std::string commandString = "./controlRegions TChiWZ None " + year + " " + controlRegion;
    std::string scriptName = controlRegion + "_nominal_" + year + ".sh";
    systemTools::submitCommandAsJob( commandString, scriptName, "10:00:00" );
}


int main( int argc, char* argv[] ){
    setTDRStyle();
    const std::string sampleDirectoryPath = "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino/";
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    
    //run specific model and mass splitting and year
    if( argc > 4 ){
        std::string model = argvStr[1];
        std::string deltaM = argvStr[2];
        std::string year = argvStr[3];
        std::string controlRegion = argvStr[4];
        analyze( model, deltaM, year, controlRegion, sampleDirectoryPath );

    } else if( argc == 4 ){
        std::string model = argvStr[1];
        std::string year = argvStr[2];
        std::string controlRegion = argvStr[3];
        analyzeAllMasses( model, year, controlRegion, sampleDirectoryPath );
        analyzeNominal( year, controlRegion );

    //run all mass splittings all years for a specific model and control region
    } else if( argc == 3 ){
        std::string model = argvStr[1];
        std::string controlRegion = argvStr[2];
        for( const auto& year : { "2016", "2017", "2018" } ){
            analyzeAllMasses( model, year, controlRegion, sampleDirectoryPath );
            analyzeNominal( year, controlRegion );
        }

    //run all mass splittings for all years for all models and a specific control region
    } else if( argc == 2 ){
        if( argvStr[1] == "nominal" ){
            for( const auto& controlRegion : {"WZ", "XGamma", "TTZ", "NP" } ){
                for( const auto& year : { "2016", "2017", "2018" } ){
                    analyzeNominal( year, controlRegion );
                }
            }

        } else {
            std::string controlRegion = argvStr[1];
            for( const auto& year : { "2016", "2017", "2018" } ){
                for( const auto& model : {"TChiWZ", "TChiSlepSnu_x0p95", "TChiSlepSnu_x0p5", "TChiSlepSnu_x0p05" } ){
                    analyzeAllMasses( model, year, controlRegion, sampleDirectoryPath );
                }
                analyzeNominal( year, controlRegion );
            }
        }

    //run all mass splittings for all models for all years and all control regions
    } else {
        for( const auto& controlRegion : {"WZ", "XGamma", "TTZ", "NP" } ){
            for( const auto& year : { "2016", "2017", "2018" } ){
                for( const auto& model : {"TChiWZ", "TChiSlepSnu_x0p95", "TChiSlepSnu_x0p5", "TChiSlepSnu_x0p05" } ){
                    analyzeAllMasses( model, year, controlRegion, sampleDirectoryPath );
                }
                analyzeNominal( year, controlRegion );
            }
        }
    }
    return 0;
}
