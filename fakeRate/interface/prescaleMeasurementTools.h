#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <thread>

#include "TH1D.h"
#include "TFile.h"

// include parts of the framework
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/RangedMap.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/Prescale.h"
#include "fakeRateSelection.h"
#include "fakeRateTools.h"
#include "CutsFitInfo.h"
#include "progressTracker.h"
#include "../../plotting/interface/tdrStyle.h"
#include "../../plotting/interface/plotCode.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ConcreteLeptonReweighter.h"
#include "../../weights/interface/ConcreteReweighterLeptons.h"
#include "../../weights/interface/ConcreteSelection.h"

HistInfo makeVarHistInfo( const unsigned numberOfBins, const double cut,
                            const double max, const bool useMT = true);

void fillPrescaleMeasurementHistograms( const std::string& year,
    const std::string& sampleDirectoryPath,
    const std::string& sampleListPath, const unsigned sampleIndex,
    const bool isTestRun, const unsigned long nEvents,
    const std::vector< std::string >& triggerVector, const bool useMT = true,
    const double metCut = 0, double mtCut = 0);
