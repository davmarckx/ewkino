

//include c++ library classes
#include <fstream>
#include <thread>
#include <tuple>

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/analysisTools.h"
#include "../Event/interface/Event.h"

bool passSyncSelection( Event& event ){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    if( event.numberOfLightLeptons() < 3 ) return false;
    return true;
}


void makeSyncDump( const std::string& inputFilePath, const std::string& outputFilePath, const std::string& eventListPath ){

    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    //output file for all event tags
    std::ofstream eventTag_dump( eventListPath );

    //output file for information on events passing selection
    std::ofstream sync_dump( outputFilePath );

    for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
        Event event = treeReader.buildEvent( entry );

        eventTag_dump << event.eventTags() << "\n";

        if( !passSyncSelection( event ) ) continue;
        
        sync_dump << event.eventTags() << "\n";
        for( auto& muonPtr : event.muonCollection() ){
            Muon& muon = *muonPtr;
            sync_dump << std::setprecision(5) << "muon : pt = " << muon.pt() << "\teta = " << muon.eta() << "\tptratio = " << muon.ptRatio() << "\tptrel = " << muon.ptRel() << "\tclosestjetdeepflavor = " << muon.closestJetDeepFlavor() << "\tsip3d = " << muon.sip3d() <<"\tdxy = " << muon.dxy() << "\tdz = " << muon.dz() << "\tminiiso = " << muon.miniIso() << "\tsegmentcompatibility = " << muon.segmentCompatibility() << "\tleptonmva = " << muon.leptonMVAttH() << "\n";
        }
        for( auto& electronPtr : event.electronCollection() ){
            Electron& electron = *electronPtr;
            sync_dump << "electron : pt = " << electron.pt() << "\teta = " << electron.eta() << "\tptratio = " << electron.ptRatio() << "\tptrel = " << electron.ptRel() << "\tclosestjetdeepflavor = " << electron.closestJetDeepFlavor() << "\tsip3d = " << electron.sip3d() <<"\tdxy = " << electron.dxy() << "\tdz = " << electron.dz() << "\tminiiso = " << electron.miniIso() << "\telectronmva = " << electron.electronMVAFall17NoIso() << "\tleptonmva = " << electron.leptonMVAttH() << "\n";
        }
    }
    eventTag_dump.close();
}


int main(){

    //make sure ROOT behaves itself when running multithreaded
    ROOT::EnableThreadSafety();

    //list of arguments with which to call makeSyncDump
    std::vector< std::tuple< std::string, std::string, std::string > > argumentVector = {
        {"WZ_2016.root", "sync_2016.txt", "event_list_2016.txt"},
        {"WZ_2017.root", "sync_2017.txt", "event_list_2017.txt"},
        {"WZ_2018.root", "sync_2018.txt", "event_list_2018.txt"}
    };
    
    std::vector< std::thread > threadVector;
    threadVector.reserve( argumentVector.size() );
    for( auto& argumentTuple : argumentVector ){
        threadVector.emplace_back( makeSyncDump, std::get<0>( argumentTuple ), std::get<1>( argumentTuple ), std::get<2>( argumentTuple ) );
    }

    for( auto& t : threadVector ){
        t.join();
    }

    return 0;
}
