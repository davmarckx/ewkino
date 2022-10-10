/*
Testing script for Run2ULReweighter
Specifically, the distribution of all reweighting factors is obtained.
*/

#include "../../interface/CombinedReweighter.h"
#include "../../interface/ConcreteReweighterFactory.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

//include other parts of framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Event/interface/Event.h"
#include "../../../Tools/interface/HistInfo.h"
#include "../../../test/copyMoveTest.h"


int main( int argc, char* argv[] ){

    int nargs = 4;
    if( argc != nargs+1 ){
        std::cerr << "### ERROR ###: run2ulReweighter_test.cc requires " << nargs;
        std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- directory of input file(s)" << std::endl;
        std::cerr << "- name of input file (.root) OR samplelist (.txt)" << std::endl;
	std::cerr << "- name of output file" << std::endl;
        std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputDirectory = argvStr[1];
    std::string& sampleList = argvStr[2];
    std::string& outputFileName = argvStr[3];
    long unsigned nEvents = std::stoul(argvStr[4]);

    // read the input file
    TreeReader treeReader;
    std::vector<Sample> samples;
    bool modeSampleList = false;
    // case 1: single input file
    if( stringTools::stringEndsWith(sampleList, ".root") ){
        std::string inputFilePath = stringTools::formatDirectoryName(inputDirectory)+sampleList;
        treeReader.initSampleFromFile( inputFilePath );
        samples.push_back( treeReader.currentSample() );
    }
    // case 2: samplelist
    else if( stringTools::stringEndsWith(sampleList, ".txt") ){
        treeReader.readSamples( sampleList, inputDirectory );
        samples = treeReader.sampleVector();
        modeSampleList = true;
    }
    std::cout << "will use the following samples:" << std::endl;
    for( Sample sample: samples ) std::cout << "- " << sample.fileName() << std::endl;

    // initialize year from first sample
    // note: in this simple test no check is done to assure all samples in the list are of same year!
    if( modeSampleList ) treeReader.initSample();
    std::string year = treeReader.getYearString();

    // make the reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new Run2ULReweighterFactory() );
    // for testing the testing script:
    //std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
	"../../", year, treeReader.sampleVector() );

    // initialize some histograms
    HistInfo histInfo = HistInfo( "", "reweighting factor", 50, 0.5, 1.5 );
    std::shared_ptr<TH1D> totWeightNom = histInfo.makeHist( "totWeightNom" );
    // electron reco
    std::shared_ptr<TH1D> electronRecoWeightNom = histInfo.makeHist( "electronRecoWeightNom" );
    std::shared_ptr<TH1D> electronRecoWeightUp = histInfo.makeHist( "electronRecoWeightUp" );
    std::shared_ptr<TH1D> electronRecoWeightDown = histInfo.makeHist( "electronRecoWeightDown" );
    std::shared_ptr<TH1D> electronRecoPtLWeightNom = histInfo.makeHist( "electronRecoPtLWeightNom" );
    std::shared_ptr<TH1D> electronRecoPtLWeightUp = histInfo.makeHist( "electronRecoPtLWeightUp" );
    std::shared_ptr<TH1D> electronRecoPtLWeightDown = histInfo.makeHist( "electronRecoPtLWeightDown" );
    std::shared_ptr<TH1D> electronRecoPtSWeightNom = histInfo.makeHist( "electronRecoPtSWeightNom" );
    std::shared_ptr<TH1D> electronRecoPtSWeightUp = histInfo.makeHist( "electronRecoPtSWeightUp" );
    std::shared_ptr<TH1D> electronRecoPtSWeightDown = histInfo.makeHist( "electronRecoPtSWeightDown" );
    std::shared_ptr<TH1D> totWeightElectronRecoUp = histInfo.makeHist( "totWeightElectronRecoUp" );
    std::shared_ptr<TH1D> totWeightElectronRecoDown = histInfo.makeHist( "totWeightElectronRecoDown" );
    // lepton ID
    std::shared_ptr<TH1D> electronIDWeightNom = histInfo.makeHist( "electronIDWeightNom" );
    std::shared_ptr<TH1D> electronIDWeightStatUp = histInfo.makeHist( "electronIDWeightStatUp" );
    std::shared_ptr<TH1D> electronIDWeightStatDown = histInfo.makeHist( "electronIDWeightStatDown" );
    std::shared_ptr<TH1D> electronIDWeightSystUp = histInfo.makeHist( "electronIDWeightSystUp" );
    std::shared_ptr<TH1D> electronIDWeightSystDown = histInfo.makeHist( "electronIDWeightSystDown" );
    std::shared_ptr<TH1D> totWeightElectronIDStatUp = histInfo.makeHist( "totWeightElectronIDStatUp" );
    std::shared_ptr<TH1D> totWeightElectronIDStatDown = histInfo.makeHist( "totWeightElectronIDStatDown" );
    std::shared_ptr<TH1D> totWeightElectronIDSystUp = histInfo.makeHist( "totWeightElectronIDSystUp" );
    std::shared_ptr<TH1D> totWeightElectronIDSystDown = histInfo.makeHist( "totWeightElectronIDSystDown" );
    std::shared_ptr<TH1D> muonIDWeightNom = histInfo.makeHist( "muonIDWeightNom" );
    std::shared_ptr<TH1D> muonIDWeightStatUp = histInfo.makeHist( "muonIDWeightStatUp" );
    std::shared_ptr<TH1D> muonIDWeightStatDown = histInfo.makeHist( "muonIDWeightStatDown" );
    std::shared_ptr<TH1D> muonIDWeightSystUp = histInfo.makeHist( "muonIDWeightSystUp" );
    std::shared_ptr<TH1D> muonIDWeightSystDown = histInfo.makeHist( "muonIDWeightSystDown" );
    std::shared_ptr<TH1D> totWeightMuonIDStatUp = histInfo.makeHist( "totWeightMuonIDStatUp" );
    std::shared_ptr<TH1D> totWeightMuonIDStatDown = histInfo.makeHist( "totWeightMuonIDStatDown" );
    std::shared_ptr<TH1D> totWeightMuonIDSystUp = histInfo.makeHist( "totWeightMuonIDSystUp" );
    std::shared_ptr<TH1D> totWeightMuonIDSystDown = histInfo.makeHist( "totWeightMuonIDSystDown" );
    // pileup
    std::shared_ptr<TH1D> pileupWeightNom = histInfo.makeHist( "pileupWeightNom" );
    std::shared_ptr<TH1D> pileupWeightUp = histInfo.makeHist( "pileupWeightUp" );
    std::shared_ptr<TH1D> pileupWeightDown = histInfo.makeHist( "pileupWeightDown" );
    std::shared_ptr<TH1D> totWeightPileupUp = histInfo.makeHist( "totWeightPileupUp" );
    std::shared_ptr<TH1D> totWeightPileupDown = histInfo.makeHist( "totWeightPileupDown" );
    // prefire
    std::shared_ptr<TH1D> prefireWeightNom = histInfo.makeHist( "prefireWeightNom" );
    std::shared_ptr<TH1D> prefireWeightUp = histInfo.makeHist( "prefireWeightUp" );
    std::shared_ptr<TH1D> prefireWeightDown = histInfo.makeHist( "prefireWeightDown" );
    std::shared_ptr<TH1D> totWeightPrefireUp = histInfo.makeHist( "totWeightPrefireUp" );
    std::shared_ptr<TH1D> totWeightPrefireDown = histInfo.makeHist( "totWeightPrefireDown" );

    // loop over samples
    unsigned numberOfSamples = samples.size();
    for( unsigned i = 0; i < numberOfSamples; ++i ){
        std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        if( modeSampleList ) treeReader.initSample( samples[i] );
	// find number of enries
	long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEvents==0 ) nEvents = numberOfEntries;
        else nEvents = std::min(nEvents, numberOfEntries);
	// loop over entries
        std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
        for( long unsigned entry = 0; entry < nEvents; ++entry ){
            if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	    
	    // build the event
            Event event = treeReader.buildEvent( entry );
	    
	    // do some selection (optional)

	    // determine the weight
	    double weight = reweighter.totalWeight( event );

	    // fill the histograms
	    totWeightNom->Fill( weight );
	    // electron reco
	    electronRecoWeightNom->Fill( reweighter.singleWeight(event,"electronReco_pTBelow20")
					    *reweighter.singleWeight(event,"electronReco_pTAbove20") );
	    electronRecoWeightUp->Fill( reweighter.singleWeightUp(event,"electronReco_pTBelow20")
                                            *reweighter.singleWeightUp(event,"electronReco_pTAbove20") );
	    electronRecoWeightDown->Fill( reweighter.singleWeightDown(event,"electronReco_pTBelow20")
                                            *reweighter.singleWeightDown(event,"electronReco_pTAbove20") );
	    electronRecoPtLWeightNom->Fill( reweighter.singleWeight(event,"electronReco_pTAbove20") );
            electronRecoPtLWeightUp->Fill( reweighter.singleWeightUp(event,"electronReco_pTAbove20") );
            electronRecoPtLWeightDown->Fill( reweighter.singleWeightDown(event,"electronReco_pTAbove20") );
	    electronRecoPtSWeightNom->Fill( reweighter.singleWeight(event,"electronReco_pTBelow20") );
            electronRecoPtSWeightUp->Fill( reweighter.singleWeightUp(event,"electronReco_pTBelow20") );
            electronRecoPtSWeightDown->Fill( reweighter.singleWeightDown(event,"electronReco_pTBelow20") );
	    totWeightElectronRecoUp->Fill( weight / reweighter.singleWeight(event,"electronReco_pTBelow20")
						  * reweighter.singleWeightUp(event,"electronReco_pTBelow20")
                                                  / reweighter.singleWeight(event,"electronReco_pTAbove20")
                                                  * reweighter.singleWeightUp(event,"electronReco_pTAbove20") );
            totWeightElectronRecoDown->Fill( weight / reweighter.singleWeight(event,"electronReco_pTBelow20")
                                                    * reweighter.singleWeightDown(event,"electronReco_pTBelow20")
                                                    / reweighter.singleWeight(event,"electronReco_pTAbove20")
                                                    * reweighter.singleWeightDown(event,"electronReco_pTAbove20") );
	    // printouts for testing
	    /*double testweight = reweighter.singleWeight(event,"electronReco_pTBelow20")
                                *reweighter.singleWeight(event,"electronReco_pTAbove20");
	    double testweightup = reweighter.singleWeightUp(event,"electronReco_pTBelow20")
                                  *reweighter.singleWeightUp(event,"electronReco_pTAbove20");
	    double testweightdown = reweighter.singleWeightDown(event,"electronReco_pTBelow20")
                                    *reweighter.singleWeightDown(event,"electronReco_pTAbove20");
	    if( testweightup>1.1 && testweightup<1.2 ){
		std::cout << "--- event ---" << std::endl;
		std::cout << "nominal: " << testweight << std::endl;
		std::cout << "nominal pt<20: " << reweighter.singleWeight(event,"electronReco_pTBelow20") << std::endl;
		std::cout << "nominal pt>20: " << reweighter.singleWeight(event,"electronReco_pTAbove20") << std::endl;
		std::cout << testweightup << std::endl;
		std::cout << testweightdown << std::endl;
		for( auto lep: event.looseLeptonCollection().electronCollection() ){
		    std::cout << lep->uncorrectedPt() << "  " << lep->etaSuperCluster() << std::endl;
		}
	    }*/
	    // lepton ID
	    electronIDWeightNom->Fill( reweighter.singleWeight(event,"electronID") );
	    electronIDWeightStatUp->Fill( reweighter.singleWeight(event,"electronID")
					*reweighter.singleWeightUp(event,"electronIDStat") );
	    electronIDWeightStatDown->Fill( reweighter.singleWeight(event,"electronID")
                                        *reweighter.singleWeightDown(event,"electronIDStat") );
	    electronIDWeightSystUp->Fill( reweighter.singleWeight(event,"electronID")
					*reweighter.singleWeightUp(event,"electronIDSyst") );
	    electronIDWeightSystDown->Fill( reweighter.singleWeight(event,"electronID")
                                        *reweighter.singleWeightDown(event,"electronIDSyst") );
	    totWeightElectronIDStatUp->Fill( reweighter.weightUp(event,"electronIDStat") );
	    totWeightElectronIDStatDown->Fill( reweighter.weightDown(event,"electronIDStat") );
	    totWeightElectronIDSystUp->Fill( reweighter.weightUp(event,"electronIDSyst") );
	    totWeightElectronIDSystDown->Fill( reweighter.weightDown(event,"electronIDSyst") );
	    muonIDWeightNom->Fill( reweighter.singleWeight(event,"muonID") );
            muonIDWeightStatUp->Fill( reweighter.singleWeight(event,"muonID")
                                        *reweighter.singleWeightUp(event,"muonIDStat") );
	    muonIDWeightStatDown->Fill( reweighter.singleWeight(event,"muonID")
                                        *reweighter.singleWeightDown(event,"muonIDStat") );
            muonIDWeightSystUp->Fill( reweighter.singleWeight(event,"muonID")
                                        *reweighter.singleWeightUp(event,"muonIDSyst") );
	    muonIDWeightSystDown->Fill( reweighter.singleWeight(event,"muonID")
                                        *reweighter.singleWeightDown(event,"muonIDSyst") );
	    totWeightMuonIDStatUp->Fill( reweighter.weightUp(event,"muonIDStat") );
            totWeightMuonIDStatDown->Fill( reweighter.weightDown(event,"muonIDStat") );
            totWeightMuonIDSystUp->Fill( reweighter.weightUp(event,"muonIDSyst") );
            totWeightMuonIDSystDown->Fill( reweighter.weightDown(event,"muonIDSyst") );
	    // pileup
	    pileupWeightNom->Fill( reweighter.singleWeight(event,"pileup") );
	    pileupWeightUp->Fill( reweighter.singleWeightUp(event,"pileup") );
	    pileupWeightDown->Fill( reweighter.singleWeightDown(event,"pileup") );
	    totWeightPileupUp->Fill( reweighter.weightUp(event,"pileup") );
            totWeightPileupDown->Fill( reweighter.weightDown(event,"pileup") );
	    // prefire
            prefireWeightNom->Fill( reweighter.singleWeight(event,"prefire") );
            prefireWeightUp->Fill( reweighter.singleWeightUp(event,"prefire") );
            prefireWeightDown->Fill( reweighter.singleWeightDown(event,"prefire") );
	    totWeightPrefireUp->Fill( reweighter.weightUp(event,"prefire") );
            totWeightPrefireDown->Fill( reweighter.weightDown(event,"prefire") );
        }
    }

    // write histograms to output file
    TFile* filePtr = TFile::Open( outputFileName.c_str(), "recreate" );
    totWeightNom->Write();
    electronRecoWeightNom->Write();
    electronRecoWeightUp->Write();
    electronRecoWeightDown->Write();
    electronRecoPtLWeightNom->Write();
    electronRecoPtLWeightUp->Write();
    electronRecoPtLWeightDown->Write();
    electronRecoPtSWeightNom->Write();
    electronRecoPtSWeightUp->Write();
    electronRecoPtSWeightDown->Write();
    totWeightElectronRecoUp->Write();
    totWeightElectronRecoDown->Write();
    electronIDWeightNom->Write();
    electronIDWeightStatUp->Write();
    electronIDWeightStatDown->Write();
    electronIDWeightSystUp->Write();
    electronIDWeightSystDown->Write();
    totWeightElectronIDStatUp->Write();
    totWeightElectronIDStatDown->Write();
    totWeightElectronIDSystUp->Write();
    totWeightElectronIDSystDown->Write();
    muonIDWeightNom->Write();
    muonIDWeightStatUp->Write();
    muonIDWeightStatDown->Write();
    muonIDWeightSystUp->Write();
    muonIDWeightSystDown->Write();
    totWeightMuonIDStatUp->Write();
    totWeightMuonIDStatDown->Write();
    totWeightMuonIDSystUp->Write();
    totWeightMuonIDSystDown->Write();
    pileupWeightNom->Write();
    pileupWeightUp->Write();
    pileupWeightDown->Write();
    totWeightPileupUp->Write();
    totWeightPileupDown->Write();
    prefireWeightNom->Write();
    prefireWeightUp->Write();
    prefireWeightDown->Write();
    totWeightPrefireUp->Write();
    totWeightPrefireDown->Write();
    filePtr->Close();
    return 0;
}
