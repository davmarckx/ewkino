#ifndef ReweighterBTagShape_H
#define ReweighterBTagShape_H

#include "Reweighter.h"

// include C++ classes
#include <vector>
#include "stdexcept"

// include ROOT classes
#include "TH2.h"

// include official b-tag weight reader
#include "../bTagSFCode/BTagCalibrationStandaloneUL.h"

class ReweighterBTagShape: public Reweighter{

    public:
	
	ReweighterBTagShape(	const std::string& weightDirectory,
				const std::string& sfFilePath,
                                const std::string& flavor,
				const std::string& bTagAlgo,
                                const std::vector<std::string>& variations,
                                const std::vector<Sample>& samples);
	void initialize( const std::vector<Sample>& samples, long unsigned numberOfEntries=0 );

	bool hasVariation( const std::string& variation ) const;
	bool hasSystematic( const std::string systematic ) const;
	bool considerVariation( const Jet& jet, const std::string& variation ) const;

	void setNormFactors( const Sample& sample, std::map<std::string, std::map<int,double>> normFactors );
	double getNormFactor( const Event&, const std::string& variation) const;
	double getNormFactor( const std::string& sampleName, int njets, 
			      const std::string& variation ) const;
	std::map< std::string, std::map< std::string, std::map<int,double >>> getNormFactors() const;
	void printNormFactors() const;
	
	double weight( const Jet& jet ) const;
	double weightUp( const Jet& jet, const std::string& systematic ) const;
	double weightDown( const Jet& jet, const std::string& systematic ) const;
	
	double weight( const Event& event ) const;
	double weightUp( const Event& event, const std::string& systematic ) const;
        double weightDown( const Event&, const std::string& systematic ) const;
	double weightNoNorm( const Event& event) const;
	std::vector<std::string> availableVariations() const{ return _variations; }
	std::vector<std::string> availableSystematics() const{ return _systematics; }
	// following functions are needed for correct inheritance, but meaningless
	double weightUp( const Event& ) const{ return 0.; } 
        double weightDown( const Event& ) const{ return 0.; }
	std::map< std::string, std::map< int, double >> calcAverageOfWeights(
                                    const Sample& sample,
                                    long unsigned numberOfEntries=0 ) const;
	int getNJets( const Event& event, const std::string& variation ) const;
        int getNJets( const Event& event ) const;

    private:

	//std::string _weightDirectory;
	std::shared_ptr<BTagCalibration> bTagSFCalibration;
	std::shared_ptr<BTagCalibrationReader> bTagSFReader;
	std::string _flavor;
	std::string _bTagAlgo;
	std::vector<std::string> _variations;
	std::vector<std::string> _systematics;
	std::map< std::string, std::map< std::string, std::map<int,double >>> _normFactors;

	double weight( const Jet& jet, const std::string& variation ) const;
	double weight( const Event& event, const std::string& variation ) const;
};

#endif
