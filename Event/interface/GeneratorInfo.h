#ifndef GeneratorInfo_H
#define GeneratorInfo_H

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../TreeReader/interface/NanoGenTreeReader.h"
#include "../../objects/interface/GenMet.h"

class GeneratorInfo{
    
    public:
	// constructor from TreeReader
        GeneratorInfo( const TreeReader& );
	// constructor from NanoGenTreeReader
	GeneratorInfo( const NanoGenTreeReader& );

        unsigned numberOfLheWeights() const{ return _numberOfLheWeights; }
	unsigned numberOfScaleVariations() const{ return _numberOfScaleWeights; }
	unsigned numberOfPdfVariations() const{ return _numberOfPdfWeights; }
	unsigned numberOfPsWeights() const{ return _numberOfPsWeights; }
	// note: inconsistent naming convention above to have backward compatibility...
	        
	double relativeWeight_MuR_1_MuF_1() const{ return _relativeWeight_MuR_1_MuF_1; }
        double relativeWeight_MuR_1_MuF_2() const{ return _relativeWeight_MuR_1_MuF_2; }
        double relativeWeight_MuR_1_MuF_0p5() const{ return _relativeWeight_MuR_1_MuF_0p5; }
        double relativeWeight_MuR_2_MuF_1() const{ return _relativeWeight_MuR_2_MuF_1; }
        double relativeWeight_MuR_2_MuF_2() const{ return _relativeWeight_MuR_2_MuF_2; }
        double relativeWeight_MuR_2_MuF_0p5() const{ return _relativeWeight_MuR_2_MuF_0p5; }
        double relativeWeight_MuR_0p5_MuF_1() const{ return _relativeWeight_MuR_0p5_MuF_1; }
        double relativeWeight_MuR_0p5_MuF_2() const{ return _relativeWeight_MuR_0p5_MuF_2; }
        double relativeWeight_MuR_0p5_MuF_0p5() const{ return _relativeWeight_MuR_0p5_MuF_0p5; }

        double relativeWeightPdfVar( const unsigned pdfIndex ) const;

	double relativeWeight_ISR_InverseSqrt2() const{ return _relativeWeight_ISR_InverseSqrt2; }
        double relativeWeight_FSR_InverseSqrt2() const{ return _relativeWeight_FSR_InverseSqrt2; }
        double relativeWeight_ISR_Sqrt2() const{ return _relativeWeight_ISR_Sqrt2; }
        double relativeWeight_FSR_Sqrt2() const{ return _relativeWeight_FSR_Sqrt2; }
        double relativeWeight_ISR_0p5() const{ return _relativeWeight_ISR_0p5; }
        double relativeWeight_FSR_0p5() const{ return _relativeWeight_FSR_0p5; }
        double relativeWeight_ISR_2() const{ return _relativeWeight_ISR_2; }
        double relativeWeight_FSR_2() const{ return _relativeWeight_FSR_2; }
        double relativeWeight_ISR_0p25() const{ return _relativeWeight_ISR_0p25; }
        double relativeWeight_FSR_0p25() const{ return _relativeWeight_FSR_0p25; }
        double relativeWeight_ISR_4() const{ return _relativeWeight_ISR_4; }
        double relativeWeight_FSR_4() const{ return _relativeWeight_FSR_4; }

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

    private:

	// number of lhe weights
        static constexpr unsigned maxNumberOfLheWeights = 148;
	unsigned _numberOfLheWeights;
	// note: above is for custom ntuples, where scale and PDF weights are put together in one array.
	// note: keep in sync with TreeReader!

	// scale weights
	static constexpr unsigned maxNumberOfScaleWeights = 9;
        unsigned _numberOfScaleWeights;
	double _relativeWeight_MuR_1_MuF_1 = 0;
        double _relativeWeight_MuR_1_MuF_2 = 0;
        double _relativeWeight_MuR_1_MuF_0p5 = 0;
        double _relativeWeight_MuR_2_MuF_1 = 0;
        double _relativeWeight_MuR_2_MuF_2 = 0;
        double _relativeWeight_MuR_2_MuF_0p5 = 0;
        double _relativeWeight_MuR_0p5_MuF_1 = 0;
        double _relativeWeight_MuR_0p5_MuF_2 = 0;
        double _relativeWeight_MuR_0p5_MuF_0p5 = 0;

	// pdf weights
        static constexpr unsigned maxNumberOfPdfWeights = 139;
        unsigned _numberOfPdfWeights;
        double _pdfWeights[maxNumberOfPdfWeights];

	// parton shower weights
        static constexpr unsigned maxNumberOfPsWeights = 46;
        unsigned _numberOfPsWeights;
        double _relativeWeight_ISR_InverseSqrt2 = 0;
        double _relativeWeight_FSR_InverseSqrt2 = 0;
        double _relativeWeight_ISR_Sqrt2 = 0;
        double _relativeWeight_FSR_Sqrt2 = 0;
        double _relativeWeight_ISR_0p5 = 0;
        double _relativeWeight_FSR_0p5 = 0;
        double _relativeWeight_ISR_2 = 0;
        double _relativeWeight_FSR_2 = 0;
        double _relativeWeight_ISR_0p25 = 0;
        double _relativeWeight_FSR_0p25 = 0;
        double _relativeWeight_ISR_4 = 0;
        double _relativeWeight_FSR_4 = 0;

	// prefire weights
        double _prefireWeight = 0;
        double _prefireWeightDown = 0;
        double _prefireWeightUp = 0;
	double _prefireWeightMuon = 0;
        double _prefireWeightMuonDown = 0;
        double _prefireWeightMuonUp = 0;
	double _prefireWeightECAL = 0;
        double _prefireWeightECALDown = 0;
        double _prefireWeightECALUp = 0;

	// other generator info
        unsigned _ttgEventType;
        unsigned _zgEventType;
        double _partonLevelHT;
        float _numberOfTrueInteractions;
};

#endif 
