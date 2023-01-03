// include header
#include "../interface/eventFlattening.h"
// include constants (particle masses)
#include "../../../constants/particleMasses.h"
#include <TLorentzVector.h>

using namespace eventFlattening;
// definition of the variables used for filling the ouput TTree
// (and evaluating an TMVA::Reader if requested)
// warning: these are not accessible outside this file; use the varmap instead!
// event id variables
ULong_t _runNb = 0;
ULong_t _lumiBlock = 0;
ULong_t _eventNb = 0;
// event weight for simulation
Float_t _weight = 0; // generator weight scaled by cross section and lumi
Float_t _normweight = 0; // total weight, including reweighting and fake rate
Float_t _leptonreweight = 1; // lepton reweighting factor
Float_t _nonleptonreweight = 1; // all other reweighting factors
Float_t _fakerateweight = 0; // fake rate reweighting factor
Float_t _chargeflipweight = 0; // charge flip reweighting factor
// event BDT variables
Float_t _abs_eta_recoil = 0;
Float_t _Mjj_max = 0;
Float_t _lW_asymmetry = 0;
Float_t _deepCSV_max = 0;
Float_t _deepCSV_leading = 0;
Float_t _deepCSV_subLeading = 0;
Float_t _deepFlavor_max = 0;
Float_t _deepFlavor_leading = 0;
Float_t _deepFlavor_subLeading = 0;
Float_t _lT = 0;
Float_t _MT = 0;
Float_t _pTjj_max = 0;
Float_t _dRlb_min = 99.;
Float_t _dRl1l2 =99.;
Float_t _dPhill_max = 0;
Float_t _HT = 0;
Float_t _nJets = 0;
Float_t _nBJets = 0;
Float_t _dRlWrecoil = 0;
Float_t _dRlWbtagged = 0;
Float_t _M3l = 0;
Float_t _abs_eta_max = 0;
Float_t _MET_pt = 0;
Float_t _MET_phi = 0;

// BDT output score
Float_t _eventBDT = 0.;
// other variables
Float_t _nMuons = 0;
Float_t _nElectrons = 0;
Float_t _leptonMVATOP_min = 1.;
Float_t _leptonMVAttH_min = 1.;
Float_t _yield = 0.5; // fixed value
Float_t _leptonPtLeading = 0.;
Float_t _leptonPtSubLeading = 0.;
Float_t _leptonPtTrailing = 0.;
Float_t _leptonEtaLeading = 0.;
Float_t _leptonEtaSubLeading = 0.;
Float_t _leptonEtaTrailing = 0.;
Float_t _leptonELeading = 0.;//new/done
Float_t _leptonESubLeading = 0.;//new/done
Float_t _jetPtLeading = 0.;
Float_t _jetPtSubLeading = 0.;
Float_t _jetEtaLeading = 0.;
Float_t _jetEtaSubLeading = 0.;
Float_t _jetMassLeading = 0.;//new/done
Float_t _jetMassSubLeading = 0.;//new/done
Float_t _numberOfVertices = 0.;
Int_t _fakeRateFlavour = -1;
Float_t _bestZMass = 0.;
Int_t _lW_charge = 0;
Float_t _leptonChargeLeading = 0;//new/done
Float_t _leptonChargeSubLeading = 0;//new/done
Float_t _l1dxy = 0.;//new
Float_t _l1dz = 0.;//new
Float_t _l1sip3d = 0.;//new            //all done
Float_t _l2dxy = 0.;//new
Float_t _l2dz = 0.;//new
Float_t _l2sip3d = 0.;//new
Float_t _lW_pt = 0.;
Float_t _Z_pt = 0.;
Float_t year = 1.;



void eventFlattening::setVariables(std::map<std::string,double> varmap){
    // copy (and parse) the values contained in varmap to the TTree variables

    _runNb = (unsigned long) varmap["_runNb"];
    _lumiBlock = (unsigned long) varmap["_lumiBlock"];
    _eventNb = (unsigned long) varmap["_eventNb"];

    _weight = varmap["_weight"];
    _normweight = varmap["_normweight"];
    _leptonreweight = varmap["_leptonreweight"];
    _nonleptonreweight = varmap["_nonleptonreweight"];
    _fakerateweight = varmap["_fakerateweight"];
    _chargeflipweight = varmap["_chargeflipweight"];

    _abs_eta_recoil = varmap["_abs_eta_recoil"];
    _Mjj_max = varmap["_Mjj_max"]; 
    _lW_asymmetry = varmap["_lW_asymmetry"];
    _deepCSV_max = varmap["_deepCSV_max"];
    _deepCSV_leading = varmap["_deepCSV_leading"];
    _deepCSV_subLeading = varmap["_deepCSV_subLeading"];
    _deepFlavor_max = varmap["_deepFlavor_max"];
    _deepFlavor_leading = varmap["_deepFlavor_leading"];
    _deepFlavor_subLeading = varmap["_deepFlavor_subLeading"];
    _lT = varmap["_lT"];
    _MT = varmap["_MT"];
    _pTjj_max = varmap["_pTjj_max"];
    _dRlb_min = varmap["_dRlb_min"];
    _dRl1l2 = varmap["_dRl1l2"];    
    _dPhill_max = varmap["_dPhill_max"];
    _HT = varmap["_HT"];
    _nJets = varmap["_nJets"];
    _nBJets = varmap["_nBJets"];
    _dRlWrecoil = varmap["_dRlWrecoil"];
    _dRlWbtagged = varmap["_dRlWbtagged"];
    _M3l = varmap["_M3l"];
    _abs_eta_max = varmap["_abs_eta_max"];
    _MET_pt = varmap["_MET_pt"];
    _MET_phi = varmap["_MET_phi"];

    _eventBDT = varmap["_eventBDT"];

    _nMuons = (int) varmap["_nMuons"];
    _nElectrons = (int) varmap["_nElectrons"];
    _leptonMVATOP_min = varmap["_leptonMVATOP_min"];
    _leptonMVAttH_min = varmap["_leptonMVAttH_min"];
    _yield = varmap["_yield"];
    _leptonChargeLeading = varmap["_leptonChargeLeading"];
    _leptonChargeSubLeading = varmap["_leptonChargeSubLeading"];
    _leptonPtLeading = varmap["_leptonPtLeading"];
    _leptonPtSubLeading = varmap["_leptonPtSubLeading"];
    _leptonPtTrailing = varmap["_leptonPtTrailing"];
    _leptonEtaLeading = varmap["_leptonEtaLeading"];
    _leptonEtaSubLeading = varmap["_leptonEtaSubLeading"];
    _leptonEtaTrailing = varmap["_leptonEtaTrailing"];
    _leptonELeading = varmap["_leptonELeading"];
    _leptonESubLeading = varmap["_leptonESubLeading"];
    _jetPtLeading = varmap["_jetPtLeading"];
    _jetPtSubLeading = varmap["_jetPtSubLeading"];
    _jetEtaLeading = varmap["_jetPtLeading"];
    _jetEtaSubLeading = varmap["_jetPtSubLeading"];
    _jetMassLeading = varmap["_jetMassLeading"];
    _jetMassSubLeading = varmap["_jetMassSubLeading"];
    _numberOfVertices = varmap["_numberOfVertices"];
    _fakeRateFlavour = varmap["_fakeRateFlavour"];
    _bestZMass = varmap["_bestZMass"];
    _lW_charge = varmap["_lW_charge"];
    _lW_pt = varmap["_lW_pt"];
    _Z_pt = varmap["_Z_pt"];
    _l1dxy = varmap["_l1dxy"];
    _l1dz = varmap["_l1dz"];
    _l1sip3d = varmap["_l1sip3d"];
    _l2dxy = varmap["_l2dxy"];
    _l2dz = varmap["_l2dz"];
    _l2sip3d = varmap["_l2sip3d"];
}

std::shared_ptr<TMVA::Reader> eventFlattening::initReader(const std::string& weightfileloc){

	std::shared_ptr<TMVA::Reader> reader = std::make_shared<TMVA::Reader>( "!Color:!Silent");

	reader->AddVariable( "f1", &_abs_eta_recoil );
	reader->AddVariable( "f2", &_Mjj_max );
	reader->AddVariable( "f3", &_deepFlavor_max );
	reader->AddVariable( "f4", &_deepFlavor_leading );
	reader->AddVariable( "f5", &_deepFlavor_subLeading );
	reader->AddVariable( "f6", &_lT );
	reader->AddVariable( "f7", &_pTjj_max );
	reader->AddVariable( "f8", &_dRlb_min );
	reader->AddVariable( "f9", &_dRl1l2 );
	reader->AddVariable( "f10",&_HT );
	reader->AddVariable( "f11", &_nJets );
	reader->AddVariable( "f12", &_nBJets );
	reader->AddVariable( "f13", &_dRlWrecoil );
	reader->AddVariable( "f14", &_dRlWbtagged );
	reader->AddVariable( "f15", &_M3l );
	reader->AddVariable( "f16", &_abs_eta_max );
	reader->AddVariable( "f17", &_MET_pt );
	reader->AddVariable( "f18", &_nMuons);
	reader->AddVariable( "f19", &_leptonMVATOP_min );
	reader->AddVariable( "f20", &_leptonChargeLeading);
	reader->AddVariable( "f21", &_leptonPtLeading );
	reader->AddVariable( "f22", &_leptonPtSubLeading );
	reader->AddVariable( "f23", &_leptonEtaLeading );
	reader->AddVariable( "f24", &_leptonEtaSubLeading );
	reader->AddVariable( "f25", &_leptonELeading );
	reader->AddVariable( "f26", &_leptonESubLeading );
	reader->AddVariable( "f27", &_jetPtLeading );
	reader->AddVariable( "f28", &_jetPtSubLeading );
	reader->AddVariable( "f29", &_jetMassLeading );
	reader->AddVariable( "f30", &_jetMassSubLeading );
	reader->AddVariable( "f31", &year );

        reader->BookMVA("BDT", weightfileloc);
        return reader;
}

std::map< std::string, double > eventFlattening::initVarMap(){
    // initialize a map of variables set to their default values
    std::map< std::string, double> varmap = {
	{"_runNb", 0},{"_lumiBlock",0},{"_eventNb",0},

	{"_weight",0},{"_normweight",0},
	{"_leptonreweight",1},{"_nonleptonreweight",1},
	{"_fakerateweight",0},{"_chargeflipweight",0},

	{"_abs_eta_recoil",0},{"_Mjj_max",0},{"_lW_asymmetry",0},
	{"_deepCSV_max",0},{"_deepCSV_leading",0},{"_deepCSV_subLeading",0},
        {"_deepFlavor_max",0},{"_deepFlavor_leading",0},{"_deepFlavor_subLeading",0},
        {"_lT",0},
	{"_MT",0},{"_pTjj_max",0},{"_dRlb_min",99.},{"_dRl1l2",99.},
	{"_dPhill_max",0},{"_HT",0},{"_nJets",0},
	{"_nBJets",0},{"_dRlWrecoil",0},{"_dRlWbtagged",0},
	{"_M3l",0},{"_abs_eta_max",0},{"_MET_phi",0},{"_MET_pt",0},

	{"_eventBDT",0},
	
	{"_nMuons",0},{"_nElectrons",0},
	
	{"_leptonMVATOP_min",1.},{"_leptonMVAttH_min",1.},
	
	{"_yield",0.5},

        {"_leptonChargeLeading",0}, {"_leptonChargeSubLeading",0},	
	{"_leptonPtLeading",0.}, {"_leptonPtSubLeading",0.}, {"_leptonPtTrailing",0.},
	{"_leptonEtaLeading",0.}, {"_leptonEtaSubLeading",0.}, {"_leptonEtaTrailing",0.},
        {"_leptonELeading",0.}, {"_leptonESubLeading",0.},
	{"_jetPtLeading",0.}, {"_jetPtSubLeading",0.},
        {"_jetEtaLeading",0.}, {"_jetEtaSubLeading",0.},
        {"_jetMassLeading",0.}, {"_jetMassSubLeading",0.},

	{"_numberOfVertices",0},
	
	{"_fakeRateFlavour",-1},
    
	{"_bestZMass",0.},
	
	{"_lW_charge",0}, {"_lW_pt",0.}, {"_Z_pt",0.},
        {"_l1dxy",0.},{"_l1dz",0.},{"_l1sip3d",0.},
        {"_l2dxy",0.},{"_l2dz",0.},{"_l2sip3d",0.}
    };
    return varmap;    
}

void eventFlattening::initOutputTree(TTree* outputTree){
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");

    // event weight for simulation (fill with ones for data)
    outputTree->Branch("_weight", &_weight, "_weight/F");
    outputTree->Branch("_normweight", &_normweight, "_normweight/F");
    outputTree->Branch("_leptonreweight", &_leptonreweight, "_leptonreweight/F");
    outputTree->Branch("_nonleptonreweight", &_nonleptonreweight, "_nonleptonreweight/F");
    outputTree->Branch("_fakerateweight", &_fakerateweight, "_fakerateweight/F");
    outputTree->Branch("_chargeflipweight", &_chargeflipweight, "_chargeflipweight/F"); 
   
    // event BDT variables
    outputTree->Branch("_abs_eta_recoil", &_abs_eta_recoil, "_abs_eta_recoil/F");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/F");
    outputTree->Branch("_lW_asymmetry", &_lW_asymmetry, "_lW_asymmetry/F");
    outputTree->Branch("_deepCSV_max", &_deepCSV_max, "_deepCSV_max/F");
    outputTree->Branch("_deepCSV_leading", &_deepCSV_leading, "_deepCSV_leading/F");
    outputTree->Branch("_deepCSV_subLeading", &_deepCSV_subLeading, "_deepCSV_subLeading/F");
    outputTree->Branch("_deepFlavor_max", &_deepFlavor_max, "_deepFlavor_max/F");
    outputTree->Branch("_deepFlavor_leading", &_deepFlavor_leading, "_deepFlavor_leading/F");
    outputTree->Branch("_deepFlavor_subLeading", &_deepFlavor_subLeading, "_deepFlavor_subLeading/F");
    outputTree->Branch("_lT", &_lT, "_lT/F");
    outputTree->Branch("_MT", &_MT, "_MT/F");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/F");
    outputTree->Branch("_dRlb_min", &_dRlb_min, "_dRlb_min/F");
    outputTree->Branch("_dRl1l2", &_dRl1l2, "_dRl1l2/F");
    outputTree->Branch("_dPhill_max", &_dPhill_max, "_dPhill_max/F");
    outputTree->Branch("_HT", &_HT, "_HT/F");
    outputTree->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/F");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/F");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/F");
    outputTree->Branch("_M3l", &_M3l, "_M3l/F");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/F");
    outputTree->Branch("_MET_pt", &_MET_pt, "_MET_pt/F");
    outputTree->Branch("_MET_phi", &_MET_phi, "_MET_phi/F");

    // BDT output score (initialized here but filled in calling function!)
    outputTree->Branch("_eventBDT", &_eventBDT, "_eventBDT/F");

    // other variables
    outputTree->Branch("_nMuons", &_nMuons, "_nMuons/F");
    outputTree->Branch("_nElectrons", &_nElectrons, "_nElectrons/F");
    outputTree->Branch("_leptonMVATOP_min", &_leptonMVATOP_min, "_leptonMVATOP_min/F");
    outputTree->Branch("_leptonMVAttH_min", &_leptonMVAttH_min, "_leptonMVAttH_min/F");
    outputTree->Branch("_yield", &_yield, "_yield/F");
    outputTree->Branch("_leptonChargeLeading", &_leptonChargeLeading, "_leptonChargeLeading/F");
    outputTree->Branch("_leptonChargeSubLeading", &_leptonChargeSubLeading, "_leptonChargeSubLeading/F");
    outputTree->Branch("_leptonPtLeading", &_leptonPtLeading, "_leptonPtLeading/F");
    outputTree->Branch("_leptonPtSubLeading", &_leptonPtSubLeading, "_leptonPtSubLeading/F");
    outputTree->Branch("_leptonPtTrailing", &_leptonPtTrailing, "_leptonPtTrailing/F");
    outputTree->Branch("_leptonEtaLeading", &_leptonEtaLeading, "_leptonEtaLeading/F");
    outputTree->Branch("_leptonEtaSubLeading", &_leptonEtaSubLeading, "_leptonEtaSubLeading/F");
    outputTree->Branch("_leptonEtaTrailing", &_leptonEtaTrailing, "_leptonEtaTrailing/F");
    outputTree->Branch("_leptonELeading", &_leptonELeading, "_leptonELeading/F");
    outputTree->Branch("_leptonESubLeading", &_leptonESubLeading, "_leptonESubLeading/F");
    outputTree->Branch("_jetPtLeading", &_jetPtLeading, "_jetPtLeading/F");
    outputTree->Branch("_jetPtSubLeading", &_jetPtSubLeading, "_jetPtSubLeading/F");
    outputTree->Branch("_jetEtaLeading", &_jetEtaLeading, "_jetEtaLeading/F");
    outputTree->Branch("_jetEtaSubLeading", &_jetEtaSubLeading, "_jetEtaSubLeading/F");
    outputTree->Branch("_jetMassLeading", &_jetMassLeading, "_jetMassLeading/F");
    outputTree->Branch("_jetMassSubLeading", &_jetMassSubLeading, "_jetMassSubLeading/F");
    outputTree->Branch("_numberOfVertices", &_numberOfVertices, "_numberOfVertices/F");
    outputTree->Branch("_fakeRateFlavour", &_fakeRateFlavour, "_fakeRateFlavour/I");
    outputTree->Branch("_bestZMass", &_bestZMass, "_bestZMass/F");
    outputTree->Branch("_lW_charge", &_lW_charge, "_lW_charge/I");
    outputTree->Branch("_lW_pt", &_lW_pt, "_lW_pt/F");
    outputTree->Branch("_Z_pt", &_Z_pt, "_Z_pt/F");
    outputTree->Branch("_l1dxy", &_l1dxy, "_l1dxy/F");
    outputTree->Branch("_l1dz", &_l1dz, "_l1dz/F");
    outputTree->Branch("_l1sip3d", &_l1sip3d, "_l1sip3d/F");
    outputTree->Branch("_l2dxy", &_l2dxy, "_l2dxy/F");
    outputTree->Branch("_l2dz", &_l2dz, "_l2dz/F");
    outputTree->Branch("_l2sip3d", &_l2sip3d, "_l2sip3d/F");
}

 
// main function //

std::map< std::string, double > eventFlattening::eventToEntry(Event& event,
				const CombinedReweighter& reweighter,
				const std::string& selection_type, 
				const std::shared_ptr< TH2D>& frMap_muon, 
				const std::shared_ptr< TH2D>& frMap_electron,
				const std::shared_ptr< TH2D>& cfMap_electron,
				const std::string& variation,
                                const std::shared_ptr<TMVA::Reader>& reader){
    // fill one entry in outputTree (initialized with initOutputTree), 
    // based on the info of one event.
    // note that the event must be cleaned and processed by an event selection function first!

    //std::cout<<"----- new event -----"<<std::endl;   

    // re-initialize all variables in the map
    std::map< std::string, double > varmap = initVarMap();
 
    // sort leptons and jets by pt
    event.sortJetsByPt();
    event.sortLeptonsByPt();

    // event id variables 
    varmap["_runNb"] = event.runNumber();
    varmap["_lumiBlock"] = event.luminosityBlock();
    varmap["_eventNb"] = event.eventNumber();

    // other global precomputed event variables
    varmap["_numberOfVertices"] = event.numberOfVertices();

    // MET
    varmap["_MET_pt"] = event.getMet("nominal").pt();
    varmap["_MET_phi"] = event.getMet("nominal").phi();

    // event weight
    varmap["_weight"] = event.weight();
    varmap["_normweight"] = event.weight();
    if(event.isMC()){ 
	varmap["_normweight"] *= reweighter.totalWeight(event);
	//varmap["_leptonreweight"] = reweighter["muonID"]->weight(event) 
	//			    * reweighter["electronID"]->weight(event);
	//varmap["_nonleptonreweight"] = reweighter.totalWeight(event)/varmap["_leptonreweight"];
    }

    // in case of running in mode "fakerate", take into account fake rate weight
    if(selection_type=="fakerate"){
	double frweight = readFakeRateTools::fakeRateWeight(event,frMap_muon,frMap_electron);
	if(event.isMC()) frweight *= -1;
	varmap["_normweight"] *= frweight;
	varmap["_fakerateweight"] = frweight;
	varmap["_fakeRateFlavour"] = readFakeRateTools::fakeRateFlavour(event);
    }

    // in case of running in mode "chargeflips", take into account charge flip weight
    if(selection_type=="chargeflips"){
	double cfweight = readChargeFlipTools::chargeFlipWeight(event, cfMap_electron, true);
	if(event.isMC()) cfweight = 0;
	varmap["_normweight"] *= cfweight;
	varmap["_chargeflipweight"] = cfweight;
    }

    // get correct jet collection and met (defined in eventSelections.cc!)
    JetCollection jetcollection = event.getJetCollection(variation);
    JetCollection bjetcollection = jetcollection.mediumBTagCollection();
    Met met = event.getMet(variation);
    // get lepton collection as well
    // (warning: a lot of event methods work on this collection implicitly,
    //  so changing the definition here is not enough to consistently use 
    //  another collection of leptons!)
    LeptonCollection lepcollection = event.leptonCollection();

    // number of muons and electrons
    varmap["_nMuons"] = lepcollection.numberOfMuons();
    varmap["_nElectrons"] = lepcollection.numberOfElectrons();

    // lepton pt and eta
    if(lepcollection.numberOfLightLeptons()>=1){
	varmap["_leptonPtLeading"] = lepcollection[0].pt();
	varmap["_leptonEtaLeading"] = lepcollection[0].eta();
        varmap["_leptonELeading"] = lepcollection[0].energy();
        varmap["_leptonChargeLeading"] = lepcollection[0].charge();
        varmap["_l1dxy"] = lepcollection[0].dxy();
        varmap["_l1dz"] = lepcollection[0].dz();
        varmap["_l1sip3d"] = lepcollection[0].sip3d();
    }
    if(lepcollection.numberOfLightLeptons()>=2){
	varmap["_leptonPtSubLeading"] = lepcollection[1].pt();
	varmap["_leptonEtaSubLeading"] = lepcollection[1].eta();
        varmap["_leptonESubLeading"] = lepcollection[1].energy();
        varmap["_leptonChargeSubLeading"] = lepcollection[1].charge();
        varmap["_dRl1l2"] = deltaR(lepcollection[0],lepcollection[1]);
        varmap["_l2dxy"] = lepcollection[1].dxy();
        varmap["_l2dz"] = lepcollection[1].dz();
        varmap["_l2sip3d"] = lepcollection[1].sip3d();
    }
    if(lepcollection.numberOfLightLeptons()>=3){
	varmap["_leptonPtTrailing"] = lepcollection[2].pt();
        varmap["_leptonEtaTrailing"] = lepcollection[2].eta();
       
    }
    // jet pt
    jetcollection.sortByPt();
    if(jetcollection.size()>=1) varmap["_jetPtLeading"] = jetcollection[0].pt();
    if(jetcollection.size()>=2) varmap["_jetPtSubLeading"] = jetcollection[1].pt();
    if(jetcollection.size()>=1) varmap["_jetEtaLeading"] = jetcollection[0].eta();
    if(jetcollection.size()>=2) varmap["_jetEtaSubLeading"] = jetcollection[1].eta();
    // jet CSVs and flavors
    if(jetcollection.size()>=1) varmap["_deepCSV_leading"] = jetcollection[0].deepCSV();
    if(jetcollection.size()>=2) varmap["_deepCSV_subLeading"] = jetcollection[1].deepCSV();
    if(jetcollection.size()>=1) varmap["_deepFlavor_leading"] = jetcollection[0].deepFlavor();
    if(jetcollection.size()>=2) varmap["_deepFlavor_subLeading"] = jetcollection[1].deepFlavor();    
    // jet masses
    TLorentzVector LV1;
    TLorentzVector LV2;
    if(jetcollection.size()>=1){
        LV1.SetPtEtaPhiE(jetcollection[0].pt(),jetcollection[0].eta(),jetcollection[0].phi(),jetcollection[0].energy());
        varmap["_jetMassLeading"] = LV1.M();
    }
    if(jetcollection.size()>=2){
        LV2.SetPtEtaPhiE(jetcollection[1].pt(),jetcollection[1].eta(),jetcollection[1].phi(),jetcollection[1].energy());
        varmap["_jetMassSubLeading"] = LV2.M();
    }
    // other more or less precomputed event variables
    varmap["_lT"] = lepcollection.scalarPtSum() + met.pt();
    varmap["_HT"] = jetcollection.scalarPtSum();
    varmap["_nJets"] = jetcollection.size();
    varmap["_nBJets"] = bjetcollection.size();
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
	    lIt != lepcollection.cend(); lIt++){
        std::shared_ptr<Lepton> lep = *lIt;
        if(lep->isElectron()){
	    std::shared_ptr<Electron> ele = std::static_pointer_cast<Electron>(lep);
            if(ele->leptonMVAttH() < varmap["_leptonMVAttH_min"]){
		varmap["_leptonMVAttH_min"] = ele->leptonMVAttH();
	    }
            if(ele->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
		varmap["_leptonMVATOP_min"] = ele->leptonMVATOP();
	    }
        }
        else if(lep->isMuon()){
	    std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
            if(mu->leptonMVAttH() < varmap["_leptonMVAttH_min"]){
		 varmap["_leptonMVAttH_min"] = mu->leptonMVAttH();
	    }
            if(mu->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
		 varmap["_leptonMVATOP_min"] = mu->leptonMVATOP();
	    }
        }
    }   

    int lWindex = 0;
    // set mT
    if(event.hasOSSFLightLeptonPair()){ 
	varmap["_MT"] = event.mtW(); 
    }

    // find lepton from W and set its properties
    // (note: code runs but is rather meaningless if no or multiple OSSF pairs are present, 
    // e.g. in some control regions)
    if(event.hasOSSFLightLeptonPair()){ 
	lWindex = event.WLeptonIndex();
    } 
    LeptonCollection::const_iterator lIt = lepcollection.cbegin();
    for(int i=0; i<lWindex; i++){++lIt;}
    Lepton& lW = **lIt;
    varmap["_lW_asymmetry"] = fabs(lW.energy())*lW.charge();
    varmap["_lW_charge"] = lW.charge();
    varmap["_lW_pt"] = lW.pt();

    // find reconstructed Z mass and pt
    if(event.hasOSSFLightLeptonPair()){
	std::pair< std::pair<int,int>, double> zbosonresults 
	    = event.leptonCollection().bestZBosonCandidateIndicesAndMass();
	varmap["_bestZMass"] = zbosonresults.second;
	varmap["_Z_pt"] = (event.leptonCollection()[zbosonresults.first.first]
			    + event.leptonCollection()[zbosonresults.first.second]).pt();
    }
    
    // top reconstruction
    std::pair< double, double > pmz = pmzcandidates(lW, met);
    std::pair< double, int > topresults = besttopcandidate(jetcollection,
					    lW, met, pmz.first, pmz.second);
    int taggedbindex = topresults.second;
    if(jetcollection.numberOfMediumBTaggedJets()==0) taggedbindex = 0;

    // find index of recoiling jet
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
	    jIt != jetcollection.cend(); jIt++){
	if(jIt-jetcollection.cbegin() != taggedbindex){
	    recoilindex = jIt-jetcollection.cbegin();
	    break;
	}
    }

    // loop over jets and find relevant quantities
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
        jIt != jetcollection.cend(); jIt++){
	Jet& jet = **jIt;
        // if abs(eta) is larger than max, modify max
        if(fabs(jet.eta())>varmap["_abs_eta_max"]) varmap["_abs_eta_max"] = fabs(jet.eta());
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>varmap["_deepCSV_max"] and jet.inBTagAcceptance()){
	     varmap["_deepCSV_max"] = jet.deepCSV();
	}
	if(jet.deepFlavor()>varmap["_deepFlavor_max"] and jet.inBTagAcceptance()){
	    varmap["_deepFlavor_max"] = jet.deepFlavor();
	}
        // find maximum dijet mass and pt
        for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != jetcollection.cend(); jIt2++){
            Jet& jet2 = **jIt2;
            if((jet+jet2).mass()>varmap["_Mjj_max"]) varmap["_Mjj_max"] = (jet+jet2).mass();
            if((jet+jet2).pt()>varmap["_pTjj_max"]) varmap["_pTjj_max"] = (jet+jet2).pt();
        } 
    }
    if(recoilindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator jIt = jetcollection.cbegin();
	for(int i=0; i<recoilindex; i++){jIt++;}
	Jet& recoiljet = **jIt;
	varmap["_dRlWrecoil"] = deltaR(lW,recoiljet);
	varmap["_abs_eta_recoil"] = fabs(recoiljet.eta());
    }
    if(taggedbindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator tbjIt = jetcollection.cbegin();
	for(int i=0; i<taggedbindex; i++){tbjIt++;}
	Jet& taggedbjet = **tbjIt;
	varmap["_dRlWbtagged"] = deltaR(lW,taggedbjet);
    }

    // loop over leptons and find some kinematic properties
    PhysicsObject l3vec;
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
        lIt != lepcollection.cend(); lIt++){
        Lepton& lep = **lIt;
        for(LeptonCollection::const_iterator lIt2 = lIt+1; lIt2 != lepcollection.cend(); lIt2++){
            Lepton& lep2 = **lIt2;
            if(deltaPhi(lep,lep2)>varmap["_dPhill_max"]) varmap["_dPhill_max"] = deltaPhi(lep,lep2);
        }
        for(JetCollection::const_iterator jIt = bjetcollection.cbegin(); 
	    jIt != bjetcollection.cend(); jIt++){
            Jet& bjet = **jIt;
            if(deltaR(lep,bjet)<varmap["_dRlb_min"]) varmap["_dRlb_min"] = deltaR(lep,bjet);
        }
    }
    varmap["_M3l"] = event.leptonSystem().mass();

    setVariables(varmap);
    varmap["_eventBDT"] = reader->EvaluateMVA("BDT");
    setVariables(varmap);
    // now return the varmap (e.g. to fill histograms)
    return varmap;
}

std::pair<double,double> eventFlattening::pmzcandidates(Lepton& lW, Met& met){
    // this method returns two candidates for longitudinal component of missing momentum,
    // by imposing the W mass constraint on the system (lW,pmiss).
    
    // first define all four-vector quantities
    double El = lW.energy(); double plx = lW.px(); double ply = lW.py(); double plz = lW.pz();
    double pmx = met.px(); double pmy = met.py(); 
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};
    
    //std::cout<<"lepton: "<<El<<" "<<plx<<" "<<ply<<" "<<plz<<std::endl;
    //std::cout<<"ptmiss: "<<pmx<<" "<<pmy<<std::endl;
    
    // then solve quadratic equation
    double A = El*El - plz*plz;
    double B = -plz*(2*(plx*pmx+ply*pmy)+mW*mW);
    double C = El*El*(pmx*pmx+pmy*pmy) - std::pow(mW*mW/2,2) - std::pow(plx*pmx+ply*pmy,2);
    C += -mW*mW*(plx*pmx+ply*pmy);
    double discr = B*B - 4*A*C;
    if(discr<0){
        //std::cout<<"### WARNING ###: negative discriminant found."<<std::endl;
        discr = 0;
    }
    pmz.first = (-B + std::sqrt(discr))/(2*A);
    pmz.second = (-B - std::sqrt(discr))/(2*A);
    return pmz;
}

std::pair<double,int> eventFlattening::besttopcandidate(JetCollection& alljets, Lepton& lW, 
					Met& met, double pmz1, double pmz2){
    // This method returns the reconstruced top mass closest to its nominal mass,
    // by combining the four-vectors of the lepton from W, the two missing momentum candidates,
    // and all medium b-jets in the event.
    // The index of the jet that gives the best top mass is returned as well.
    
    std::pair<double,int> res = {0.,0};
    double bestmass = 0.;
    double massdiff = fabs(particle::mTop-bestmass);
    int bindex = -1;
    // set lorentz vectors for lepton and neutrino
    double metpx = met.px();
    double metpy = met.py();  
    LorentzVector nu1 = lorentzVectorPxPyPzEnergy( metpx, metpy, pmz1, 
			    std::sqrt(metpx*metpx+metpy*metpy+pmz1*pmz1));
    LorentzVector nu2 = lorentzVectorPxPyPzEnergy( metpx, metpy, pmz2,
			    std::sqrt(metpx*metpx+metpy*metpy+pmz2*pmz2));
    LorentzVector lep = lorentzVectorPxPyPzEnergy( lW.px(), lW.py(), lW.pz(), lW.energy() );
    // loop over jets
    double mass = 0.;
    for(JetCollection::const_iterator jIt = alljets.cbegin(); jIt != alljets.cend(); jIt++){
        Jet& jetobject = **jIt;
	// consider only medium tagged b-jets
	if(!jetobject.isBTaggedMedium()) continue;
        LorentzVector jet = lorentzVectorPxPyPzEnergy( jetobject.px(), jetobject.py(),
							jetobject.pz(), jetobject.energy());
	// try neutrino hypothesis one
        mass = (nu1 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
	// try neutrino hypothesis two
        mass = (nu2 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
    }
    res.first = bestmass;
    res.second = bindex;
    return res;   
}
