#include "../interface/treeReader.h"

/*
* order objects by their pT
*/

void treeReader::orderByPt(std::vector<unsigned>& ind, const double* pt, const unsigned count) const{
    std::vector<std::pair<double, unsigned>> ptMap;
    for(unsigned p = 0; p < count; ++p){
        ptMap.push_back({pt[ind[p]], ind[p]});
    }
    std::sort(ptMap.begin(), ptMap.end(), [](std::pair<double, unsigned>& p1, std::pair<double, unsigned>& p2){ return p1.first > p2.first; } );
    for(unsigned p = 0; p < count; ++p){
        ind[p] = ptMap[p].second;
    }
}

unsigned treeReader::dilFlavorComb(const std::vector<unsigned>& ind) const{
    unsigned flavCount[3] = {0,0,0};
    for(unsigned l = 0; l < 2; ++l) ++flavCount[_lFlavor[ind[l]]];
    if(flavCount[2] == 0){
        if(flavCount[1] == 0) return 0; //ee
        else if(flavCount[1] == 1) return 1; //em
        else return 2; //mm
    } else if(flavCount[2] == 1){
        if(flavCount[1] == 0) return 3; //et
        else return 4; //mt
    }
    return 5; //tt
}


/*
* lepton event selection
*/

//apply cone-correction to leptons
double treeReader::coneCorr(const unsigned ind) const{
    double corr = 1.;
    if( lepIsGood(ind) && !lepIsTight(ind) ){
        corr *=( 1. + std::max(_relIso[ind] - 0.1, 0.) );
    }
    return corr;
}

void treeReader::applyConeCorrection(){
    for(unsigned l = 0; l < _nLight; ++l){
        double coneC = coneCorr(l);
        _lPt[l] *= coneC;
        _lE[l] *= coneC;
    }
}

//select good leptons and order them by pT 
unsigned treeReader::selectLep(std::vector<unsigned>& ind) const{
    ind.clear();
    unsigned lCount = 0;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l)){
            ++lCount;
            ind.push_back(l);
        }
    }
    if(lCount < 2) return 0;
    orderByPt(ind, _lPt, lCount);
    return lCount;	
}

//lepton selection with cone-correction application
unsigned treeReader::selectLepConeCorr(std::vector<unsigned>& ind){

    //apply cone correction
    applyConeCorrection();

    //select and order cone-corrected leptons
    return selectLep(ind);
}

//amount of consecutive tight leptons
unsigned treeReader::tightLepCount(const std::vector<unsigned>& ind, const unsigned lCount) const{
    unsigned tightC = 0; 
    for(unsigned l = 0; l < lCount; ++l){
        if(lepIsTight(ind[l])) ++tightC;
        else return tightC;
    }
    return tightC;
}

//lepton pT thresholds
bool treeReader::passPtCuts(const std::vector<unsigned>& ind) const{
    if(_lPt[ind[0]] <= 25) return false;
    if(_lPt[ind[1]] <= 15) return false;
    return true;
}

//check if leptons are prompt
bool treeReader::promptLeptons() const{

    //don't apply this check to data
    if( isData() ){
        return true;
    }
    
    //check whether every MC lepton is prompt
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l) && !_lIsPrompt[l]) return false;
    }
    return true;
}


/*
* number of jets 
*/

//select and count jets
unsigned treeReader::nJets(const unsigned unc, const bool clean, const bool allowForward ) const{
    unsigned nJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 30, unc, clean, allowForward)) ++nJets;
    }
    return nJets;
}

//select, count, and order jets by pT
unsigned treeReader::nJets(std::vector<unsigned>& jetInd, const unsigned unc, const bool clean, const bool allowForward) const{
    unsigned nJets = 0;
    jetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 30, unc, clean, allowForward)){
            ++nJets;
            jetInd.push_back(j);
        }
    }
    orderByPt(jetInd, _jetPt, nJets);
    return nJets;
}


/*
* number of b-tagged jets
*/

//select and count b-tagged jets
unsigned treeReader::nBJets(const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp) const{
    unsigned nbJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)){
            if(bTagged(j, wp, deepCSV) ) ++nbJets;
        }
    }
    return nbJets;
}

//select, count, and order b-jets by pT
unsigned treeReader::nBJets(std::vector<unsigned>& bJetInd, const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp) const{
    unsigned nbJets = 0;
    bJetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)){ 
            if(bTagged(j, wp, deepCSV) ){
                ++nbJets;
                bJetInd.push_back(j);
            }
        }
    }
    orderByPt(bJetInd, _jetPt, nbJets);
    return nbJets;
}


/*
* remove overlapping phase-space between different MC samples
*/

//check if lepton comes from matrix-element conversion
bool treeReader::lepFromMEExtConversion(const unsigned leptonIndex) const{
    bool fromConversion = (_lMatchPdgId[leptonIndex] == 22);
    bool promptConversion = (_lIsPrompt[leptonIndex] && _lProvenanceConversion[leptonIndex] == 0);
    return (fromConversion && promptConversion);
}

//reject events with overlapping photon-production phase-space
bool treeReader::photonOverlap(const Sample& samp, const bool mcNonprompt) const{

    //in case of data-driven nonprompt estimation the only sample that is to be cleaned is Wgamma (for overlap with WZ)
    bool isInclusiveSample;
    bool isPhotonSample;
    if( mcNonprompt ){
        isInclusiveSample = (samp.getFileName().find("DYJetsToLL") != std::string::npos) ||
            (samp.getFileName().find("TTTo2L") != std::string::npos) ||
            (samp.getFileName().find("TTJets") != std::string::npos );

        isPhotonSample = (samp.getFileName().find("ZGTo2LG") != std::string::npos) ||
            (samp.getFileName().find("TTGJets") != std::string::npos) ||
            (samp.getFileName().find("WGToLNuG") != std::string::npos);
    } else {
        isInclusiveSample = false;
        isPhotonSample = (samp.getFileName().find("WGToLNuG") != std::string::npos);
    }

    //require inclusive sample to contain no external conversions
    if(isInclusiveSample){
       for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && lepFromMEExtConversion(l) ){
                return true;
            }
        } 
    //require photon samples to have atlease one external conversion
    } if(isPhotonSample){
        bool hasConversion = false;
        for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && lepFromMEExtConversion(l) ){
                hasConversion = true;
            }
        }
        return !(hasConversion);
    }
    return false;
}

bool treeReader::photonOverlap(const bool mcNonprompt) const{
    return photonOverlap(currentSample, mcNonprompt);
}

//overlap removal between inclusive and HT-binned samples
bool treeReader::htOverlap(const Sample& samp) const{
    if(samp.getFileName().find("DYJetsToLL_M-50_Tune") != std::string::npos){
        return _gen_HT > 70.;
    } else if(samp.getFileName().find("DYJetsToLL_M-10_50_Tune") != std::string::npos){
        return _gen_HT > 100.;
    }
    return false;
}

bool treeReader::htOverlap() const{
    return htOverlap(currentSample);
}

/*
* trigger selection
*/

bool treeReader::passSingleLeptonTriggers() const{
    return (_passTrigger_e || _passTrigger_m);
}

bool treeReader::passDileptonTriggers() const{
    return (_passTrigger_ee || _passTrigger_em || _passTrigger_mm);
}

bool treeReader::passTrileptonTriggers() const{
    return (_passTrigger_eee || _passTrigger_eem || _passTrigger_emm || _passTrigger_mmm); 
}

bool treeReader::passTriggerCocktail() const{
    bool pass = passSingleLeptonTriggers() ||
        passDileptonTriggers() ||
        passTrileptonTriggers();
    return pass;
}

bool treeReader::passMETFilters() const{
    return _passMETFilters;
}
