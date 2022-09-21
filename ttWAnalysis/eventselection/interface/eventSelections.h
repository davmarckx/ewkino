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
bool passLeptonPtSelections(Event&);
bool hasnFOLeptons(Event&, int, bool select);
bool hasnTightLeptons(Event&, int, bool select);
bool allLeptonsArePrompt(const Event&);
// pass functions
bool pass_signalregion_trilepton(Event& event, const std::string& selectiontype,
                                const std::string& variation, const bool selectbjets);
bool pass_wzcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_zzcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_zgcontrolregion(Event&, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets);
bool pass_nonprompt_trilepton_noossf(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_nonprompt_trilepton_noz(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_nonprompt_trilepton(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);
bool pass_nonprompt_dilepton(Event&, const std::string& selectiontype,
                        const std::string& variation, const bool selectbjets);

#endif
