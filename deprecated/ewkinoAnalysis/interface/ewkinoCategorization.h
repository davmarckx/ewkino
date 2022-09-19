#ifndef ewkinoCategorization_H
#define ewkinoCategorization_H

//include other parts of framework
#include "../../Event/interface/Event.h"


namespace ewkino{
    enum EwkinoCategory{ 
        dilepSS, 
        trilepLightOSSF, 
        trilepLightNoOSSF, 
        trilepSingleTauOSSF, 
        trilepSingleTauOS, 
        trilepSingleTauSS, 
        trilepDiTau,
        fourlepLightTwoOSSF,
        fourlepLightOneZeroOSSF,
        fourlepSingleTau,
        fourlepDiTauTwoOSSF,
        fourlepDiTauOneZeroOSSF,
        unknown
    };

    //assume objects have already passed the baseline selection and leptons are sorted by pT!
    EwkinoCategory ewkinoCategory( const Event& );
}
    

#endif
