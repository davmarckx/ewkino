/*
Perform cutflow study
*/

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"

// include other parts of the analysis code
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventSelectionsParticleLevel.h"

std::shared_ptr<TH1D> makeCutFlowHistogram( const std::string& pathToFile,
			const std::string& eventSelection, 
			const std::string& selectionType, 
			const std::string& variation,
			long nEvents,
			int maxCutFlowValue,
			bool doParticleLevel ){
    // make a TH1D containing the output of cutflow event selection.
    // the output of the cutflow eventselection function is an integer number
    // (usually the number of cuts passed, but see eventselection/src/eventSelections for details)

    // Note: the reference with respect to which efficiencies are calculated,
    // is the hCounter, i.e. the total number of originally generated events (before skimming).
    // The hCounter is filled in the first bin of the cutflow histogram.
    // The second bin is filled with the total number of events in the sample
    // (which might differ from hCounter because of skimming).

    // Note: when doParticleLevel is true, the first bin is still filled with hCounter,
    // but the second bin is filled with the total number of events passing particle level selection
    // (not the total number of events in the sample) and this selection is also propagated to all next bins.
    // Hence, the efficiency of the particle level selection with respect to the total sample,
    // and the efficiency of the event selection with respect to the particle level selection can be calculated.
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    
    // initialize output histogram
    int nOffset = 2;
    std::shared_ptr<TH1D> cutFlowHist = std::make_shared<TH1D>( 
	"cutFlowHist", "cutFlowHist;cutflow value;number of events", 
	maxCutFlowValue+nOffset, 0.5, maxCutFlowValue+nOffset+0.5);
    // set the bin labels to empty strings
    for( int i=1; i<cutFlowHist->GetNbinsX()+1; ++i){
	cutFlowHist->GetXaxis()->SetBinLabel(i, "");
    }

    // get hCounter of the sample and fill first histogram bin with it
    TH1D* hCounter = new TH1D( "hCounter", "Events counter", 1, 0, 1 );
    std::shared_ptr<TFile> filePtr = std::make_shared<TFile>( pathToFile.c_str() , "read");
    filePtr->cd( "blackJackAndHookers" );
    hCounter->Read( "hCounter" );
    int nSimulatedEvents = hCounter->GetEntries();
    //double sumSimulatedEventWeights = hCounter->GetBinContent(1);
    delete hCounter;
    filePtr->Close();
    cutFlowHist->SetBinContent(1, nSimulatedEvents);
    cutFlowHist->GetXaxis()->SetBinLabel(1, "hCounter");

    // get number of events in sample and fill second histogram bin with it
    if( !doParticleLevel ){
	cutFlowHist->SetBinContent(2, treeReader.numberOfEntries());
	cutFlowHist->GetXaxis()->SetBinLabel(2, "nEntries");
    }
    else cutFlowHist->GetXaxis()->SetBinLabel(2, "Particle level selection");

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	Event event = treeReader.buildEvent(entry, false, false, false, false, doParticleLevel);
	// do particle level selection
	// warning: the event selection is hard-coded to be signal region here
	if( doParticleLevel ){
	    bool passPL = eventSelectionsParticleLevel::pass_signalregion_dilepton_inclusive( event );
	    if( !passPL ) continue;
	    cutFlowHist->Fill(2);
	}
	// do detector level selection
	// warning: the event selection is hard-coded to be signal region here
	//          eventSelection argument is ignored.
	std::tuple<int,std::string> cutFlowTuple = eventSelections::pass_signalregion_dilepton_inclusive_cutflow( event, 
			    selectionType, variation, true );
	int cutFlowValue = std::get<0>(cutFlowTuple);
	std::string cutFlowDescription = std::get<1>(cutFlowTuple);
	if( cutFlowValue>maxCutFlowValue ){
	    std::string msg = "WARNING: cutFlowValue is "+std::to_string(cutFlowValue);
	    msg += " while maximum was set to "+std::to_string(maxCutFlowValue);
	    msg += "; please run again with larger maxCutFlowValue.";
	    throw std::runtime_error(msg);
	}
	// determine corresponding bin number
	int binnb = cutFlowValue+nOffset;
	// set the bin label if not done so before
	if( std::string(cutFlowHist->GetXaxis()->GetBinLabel(binnb))==std::string("") ){
	    cutFlowHist->GetXaxis()->SetBinLabel(binnb, cutFlowDescription.c_str());
	}
	// fill the histogram
	cutFlowHist->Fill(cutFlowValue+nOffset);
    }
    return cutFlowHist;
}


int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 9 ){
        std::cerr << "ERROR: cutFlow requires the following arguments: " << std::endl;
	std::cerr << "- input_file_path" << std::endl;
	std::cerr << "- output_file_path" << std::endl;
	std::cerr << "- event_selection" << std::endl;
	std::cerr << "- selection_type" <<std::endl;
	std::cerr << "- variation" << std::endl;
	std::cerr << "- nevents" << std::endl;
	std::cerr << "- max_cutflow_value" << std::endl;
	std::cerr << "- do particle level selection" << std::endl;
	return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& event_selection = argvStr[3];
    std::string& selection_type = argvStr[4];
    std::string& variation = argvStr[5];
    long nevents = std::stol(argvStr[6]);
    int max_cutflow_value = std::stoi(argvStr[7]);
    bool do_particle_level = (argvStr[8]=="true" || argvStr[8]=="True");
   
    // make the cutflow histogram
    std::shared_ptr<TH1D> cutFlowHist = makeCutFlowHistogram( 
			input_file_path, event_selection, 
			selection_type, variation,
			nevents, max_cutflow_value,
			do_particle_level );

    // write to output file
    TFile* outputFilePtr = TFile::Open( output_file_path.c_str() , "RECREATE" );
    cutFlowHist->Write();
    outputFilePtr->Close();

    std::cerr<<"###done###"<<std::endl;
    return 0;
}
