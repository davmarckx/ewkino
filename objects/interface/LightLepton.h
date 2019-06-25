#ifndef LightLepton_H
#define LightLepton_H

//include c++ library classes 

//include ROOT classes 

//include other parts of code 
#include "Lepton.h"
#include "../../TreeReader/interface/TreeReader.h"

class LightLepton : public Lepton {
    
    public: 
        LightLepton( const TreeReader&, const unsigned, LeptonSelector* ); 
        LightLepton( const LightLepton& ) = default;
        LightLepton( LightLepton&& ) = default;

        LightLepton& operator=( const LightLepton& ) = default;
        LightLepton& operator=( LightLepton&& ) = default;

        //isolation variables 
        double relIso0p3() const{ return _relIso0p3; }
        double relIso0p4() const{ return _relIso0p4; }
        double miniIso() const{ return _miniIso; }
        double miniIsoCharged() const{ return _miniIsoCharged; }
        double miniIsoNeutral() const{ return _miniIso - _miniIsoCharged; }

        //properties of the jet closest to the lepton
        double ptRatio() const{ return _ptRatio; }
        double ptRel() const{ return _ptRel; }
        double closestJetDeepCSV() const{ return _closestJetDeepCSV; }
        unsigned closestJetTrackMultiplicity() const{ return _closestJetTrackMultiplicity; }

        //lepton MVA discriminant
        double leptonMVA() const{ return _leptonMVA; }

        //check lepton type
        virtual bool isLightLepton() const override{ return true; }
        virtual bool isTau() const override{ return false; }

        //destructor
        virtual ~LightLepton(){};

    private:

        //isolation variables 
        double _relIso0p3 = 0;
        double _relIso0p4 = 0;
        double _miniIso = 0;
        double _miniIsoCharged = 0;

        //properties of the jet closest to the lepton
        double _ptRatio = 0;
        double _ptRel = 0;
        double _closestJetDeepCSV = 0;
        unsigned _closestJetTrackMultiplicity = 0;

        //lepton MVA output 
        double _leptonMVA = 0;
        
};
#endif
