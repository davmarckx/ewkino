#ifndef GeneratorInfo_H
#define GeneratorInfo_H

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/GenMet.h"

class GeneratorInfo{
    
    public:
        GeneratorInfo( const TreeReader& );

        unsigned numberOfLheWeights() const{ return _numberOfLheWeights; }
        double relativeWeightScaleVar( const unsigned scaleIndex ) const;
        double relativeWeight_MuR_1_MuF_1() const{ return relativeWeightScaleVar( 0 ); }
        double relativeWeight_MuR_1_MuF_2() const{ return relativeWeightScaleVar( 1 ); }
        double relativeWeight_MuR_1_MuF_0p5() const{ return relativeWeightScaleVar( 2 ); }
        double relativeWeight_MuR_2_MuF_1() const{ return relativeWeightScaleVar( 3 ); }
        double relativeWeight_MuR_2_MuF_2() const{ return relativeWeightScaleVar( 4 ); }
        double relativeWeight_MuR_2_MuF_0p5() const{ return relativeWeightScaleVar( 5 ); }
        double relativeWeight_MuR_0p5_MuF_1() const{ return relativeWeightScaleVar( 6 ); }
        double relativeWeight_MuR_0p5_MuF_2() const{ return relativeWeightScaleVar( 7 ); }
        double relativeWeight_MuR_0p5_MuF_0p5() const{ return relativeWeightScaleVar( 8 ); }

        double relativeWeightPdfVar( const unsigned pdfIndex ) const;

        unsigned numberOfPsWeights() const{ return _numberOfPsWeights; }
        double relativeWeightPsVar( const unsigned psIndex ) const;
        double relativeWeight_ISR_InverseSqrt2() const{ return relativeWeightPsVar( 2 ); }
        double relativeWeight_FSR_InverseSqrt2() const{ return relativeWeightPsVar( 3 ); }
        double relativeWeight_ISR_Sqrt2() const{ return relativeWeightPsVar( 4 ); }
        double relativeWeight_FSR_Sqrt2() const{ return relativeWeightPsVar( 5 ); }
        double relativeWeight_ISR_0p5() const{ return relativeWeightPsVar( 6 ); }
        double relativeWeight_FSR_0p5() const{ return relativeWeightPsVar( 7 ); }
        double relativeWeight_ISR_2() const{ return relativeWeightPsVar( 8 ); }
        double relativeWeight_FSR_2() const{ return relativeWeightPsVar( 9 ); }
        double relativeWeight_ISR_0p25() const{ return relativeWeightPsVar( 10 ); }
        double relativeWeight_FSR_0p25() const{ return relativeWeightPsVar( 11 ); }
        double relativeWeight_ISR_4() const{ return relativeWeightPsVar( 12 ); }
        double relativeWeight_FSR_4() const{ return relativeWeightPsVar( 13 ); }

        unsigned ttgEventType() const{ return _ttgEventType; }
        unsigned zgEventType() const{ return _zgEventType; }
        double partonLevelHT() const{ return _partonLevelHT; }
        float numberOfTrueInteractions() const{ return _numberOfTrueInteractions; }

        double prefireWeight() const{ return _prefireWeight; }
        double prefireWeightDown() const{ return _prefireWeightDown; }
        double prefireWeightUp() const{ return _prefireWeightUp; }
	double prefireWeightMuon() const{ return _prefireWeightMuon; }
        double prefireWeightMuonDown() const{ return _prefireWeightMuonDown; }
        double prefireWeightMuonUp() const{ return _prefireWeightMuonUp; }
	double prefireWeightECAL() const{ return _prefireWeightECAL; }
        double prefireWeightECALDown() const{ return _prefireWeightECALDown; }
        double prefireWeightECALUp() const{ return _prefireWeightECALUp; }

        const GenMet& genMet() const{ return *_genMetPtr; }

    private:
        static constexpr unsigned maxNumberOfLheWeights = 148;
        unsigned _numberOfLheWeights;
        double _lheWeights[maxNumberOfLheWeights];
        static constexpr unsigned maxNumberOfPsWeights = 46;
        unsigned _numberOfPsWeights;
        double _psWeights[maxNumberOfPsWeights];
        double _prefireWeight = 0;
        double _prefireWeightDown = 0;
        double _prefireWeightUp = 0;
	double _prefireWeightMuon = 0;
        double _prefireWeightMuonDown = 0;
        double _prefireWeightMuonUp = 0;
	double _prefireWeightECAL = 0;
        double _prefireWeightECALDown = 0;
        double _prefireWeightECALUp = 0;

        unsigned _ttgEventType;
        unsigned _zgEventType;
        double _partonLevelHT;
        float _numberOfTrueInteractions;

        std::shared_ptr< GenMet > _genMetPtr;
};

#endif 
