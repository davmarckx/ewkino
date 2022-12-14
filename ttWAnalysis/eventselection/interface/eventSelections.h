#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

bool passES(Event&, const std::string& eventselection, const std::string& selectiontype,
	     const std::string& variation, const bool selectbjets=true);
// help functions
void cleanLeptonsAndJets(Event&);
JetCollection getjetcollection(const Event&, const std::string& variation);
Met getmet(const Event&, const std::string& variation);
bool hasLeptonFromMEExternalConversion( const Event& );
bool leptonFromMEExternalConversion( const Lepton& );
bool passAnyTrigger(Event&);
bool hasnFOLeptons(Event&, int, bool select);
bool hasnTightLeptons(Event&, int, bool select);
bool allLeptonsArePrompt(const Event&);
// pass functions
// signal regions
bool pass_signalregion_dilepton_inclusive(Event& event, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_signalregion_dilepton_ee(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_signalregion_dilepton_em(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_signalregion_dilepton_me(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_signalregion_dilepton_mm(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_signalregion_trilepton(Event& event, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
// prompt control regions
bool pass_wzcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_zzcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_zgcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_trileptoncontrolregion(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_fourleptoncontrolregion(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
// nonprompt control regions
bool pass_npcontrolregion_dilepton_inclusive(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_npcontrolregion_dilepton_ee(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_npcontrolregion_dilepton_em(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_npcontrolregion_dilepton_me(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_npcontrolregion_dilepton_mm(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
// charge flip control region
bool pass_cfcontrolregion(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
// cutflow functions
// remark: put in a namespace for clearer calling in other scripts;
// the other ones have not been put in a namespace "for historical reasons",
// maybe change later.
namespace eventSelections{
    std::tuple<int,std::string> pass_signalregion_dilepton_inclusive_cutflow(
	Event& event, const std::string& selectiontype,
	const std::string& variation, const bool selectbjets);
    std::tuple<int,std::string> pass_signalregion_trilepton_cutflow(
        Event& event, const std::string& selectiontype,
        const std::string& variation, const bool selectbjets);
}

#endif
