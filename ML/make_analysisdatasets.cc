// This is used for skimming the analysis events from the last used training events, more inefficient than splitting first but allows for differing you trainingset during training to lower potential bias

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "../Event/interface/Event.h"
#include "../TreeReader/interface/TreeReader.h"

//R__LOAD_LIBRARY($ROOTSYS/test/libEvent.so)

void filterFunction(const std::string& sampleList,
                    const std::string& inputDirectory,
                    const std::string& outputDirectory,
                    const std::string& name,
                    int sampleIndex ,
                    const std::string& yearx,
                    std::map< int, std::vector<int>>& indexMap
                    ){

    //Get old file, old tree and set top branch address
    std::string location = inputDirectory + "/" + name;
    std::cout << "get old file at " << location;
    TFile *oldfile = new TFile(location.c_str());
    TTree *oldtree = (TTree*)oldfile->Get("blackJackAndHookers/blackJackAndHookersTree");
    Int_t nentries = (Int_t)oldtree->GetEntries();

    //Event *event = nullptr;
    //oldtree->SetBranchAddress("event", &event);
    

    std::cout << "initializing TreeReader for sample at index " << sampleIndex << "..." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){ treeReader.initSample(); }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();


    //Create a new file + a clone of old tree in new file
    std::string outlocation = outputDirectory + "/" + name;
    std::cout << "make new file at " << outlocation;
    TFile *newfile = new TFile(outlocation.c_str(),"recreate");
    newfile->mkdir( "blackJackAndHookers" );
    newfile->cd( "blackJackAndHookers" );

    TTree *newtree = oldtree->CloneTree(0);
    oldtree->CopyAddresses(newtree);
    
   
    // copy histograms from input file to output file
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
      histPtr->Write();
    }

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if(numberOfEntries != nentries){throw std::runtime_error( "it seems like treereader is reading a different TTree, check your samples" );}


    int lumiblock;
    int eventnr;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry);
        oldtree->GetEntry(entry);
        lumiblock = event.luminosityBlock();
        eventnr = event.eventNumber();
        if (indexMap.find(lumiblock) == indexMap.end()) {
          //lumiblock not found so we can write
          //std::cerr<<"lumiblock not in"<<std::endl;
          newtree->Fill();
        } 
        else {
          if (std::count(indexMap[lumiblock].begin(), indexMap[lumiblock].end(), eventnr)) {
            //this sample was used in training
            std::cerr<<"!!!!!!!!!!!!!!!!!!!!!!!!in, dont fill"<<std::endl;
            continue;
          }
          else {
            // runnumber not found so we can write
            newtree->Fill();
          }
    }}

    //newfile->cd( "blackJackAndHookers" );
    newfile->Write("", BIT(2) );
    newfile->Close();
}


std::map< int, std::vector<int> > loadIndices(const std::string& type,
                    int sampleIndex ,
                    const std::string& yearx
                    ){

   std::fstream newfile;
   std::string location = "trainindices/";
   location += type + "_" + std::to_string(sampleIndex) + "_" + yearx + ".txt";

   std::cout<<"opening the indices file:"<<location;
   newfile.open(location);

   std::map< int, std::vector<int> > indexMap;
   
   if (newfile.is_open()){ //checking whether the file is open
      std::cout<<" opened the indices file "<<std::endl; 
      std::string tp;
      while(getline(newfile, tp)){ //read data from file object and put it into string.
        if( tp == "" ) continue;
        std::stringstream ss(tp);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> vstrings(begin, end);

        int val = std::stoi(vstrings[0]);
        vstrings.erase(vstrings.begin()); 

        std::vector<int> ints;
        std::transform(vstrings.begin(), vstrings.end(), std::back_inserter(ints),
        [&](std::string s) {
            return stoi(s);
        });
        
        vstrings.erase(vstrings.begin());
        indexMap.insert(make_pair(val, ints));
   }
      newfile.close(); //close the file object.
   }

   return indexMap;
}


int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 8  ){
        std::cerr << "ERROR: need following command line arguments:" << std::endl;
        std::cerr << " - sample name" << std::endl;
        std::cerr << " - sample type" << std::endl;
        std::cerr << " - sample index" << std::endl;
        std::cerr << " - sample year" << std::endl;
        std::cerr << " - sample list" << std::endl;
        std::cerr << " - sample inputdir" << std::endl;
        std::cerr << " - sample outputdir" << std::endl;

        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    // necessary arguments:
    std::string& sample_name = argvStr[1];
    std::string& sample_type = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& sample_year = argvStr[4];

    std::string& samplelist = argvStr[5];
    std::string& inputdir = argvStr[6];
    std::string& outputdir = argvStr[7];

    // call functions
    std::cerr<<"start loading indices"<<std::endl;
    std::map< int, std::vector<int> > indexMap = loadIndices(sample_type, sample_index, sample_year);

    std::cerr<<"start filtering"<<std::endl;
    filterFunction( samplelist, inputdir,outputdir, sample_name, sample_index, sample_year, indexMap);

    std::cerr<<"###done###"<<std::endl;
    return 0;
}
