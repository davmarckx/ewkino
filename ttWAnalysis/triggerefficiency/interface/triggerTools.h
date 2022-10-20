#ifndef triggerTools_H
#define triggerTools_H

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraphAsymmErrors.h"

// include other parts of framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/HistInfo.h"
#include "../../../Tools/interface/rootFileTools.h"
#include "../../../Tools/interface/variableTools.h"
#include "../../../Tools/interface/histogramTools.h"
#include "../../../Event/interface/Event.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"
#include "../../eventselection/interface/eventSelections.h"
#include "../../eventselection/interface/eventFlattening.h"

namespace triggerTools{
    std::vector<double> ptThresholds( const std::string& id, 
				      const std::string& channel );
    bool passPtThresholds( const std::vector<double>& pts, 
			   const std::vector<double>& thresholds );
    std::string getFlavourString( const Event& event );
    bool passTriggersRef( const Event& event );
}

#endif
