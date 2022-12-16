/*
Tools for reading collections of histogram variables in txt format
*/

#ifndef variableTools_h
#define variableTools_h

// include c++ library classes 
#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>

// include ROOT classes
#include "TH1D.h"

// include other parts of the framework
#include "../../Tools/interface/HistInfo.h"

class HistogramVariable{

    public:
	HistogramVariable(  const std::string& name, 
			    const std::string& variable,
			    int nbins,
			    double xlow,
			    double xhigh,
                            const std::vector<double> bins = {} );
	HistogramVariable(  const std::string& name,
			    const std::string& variable,
                            const std::string& nbins,
                            const std::string& xlow,
			    const std::string& xhigh,
			    const std::vector<std::string> bins = {} );
	std::string name() const{ return _name; }
	std::string variable() const{ return _variable; }
	int nbins() const{ return _nbins; }
	double xlow() const{ return _xlow; }
	double xhigh() const{ return _xhigh; }
        std::vector<double> bins() const{ return _bins; }
	std::string toString() const;

    private:
	std::string _name;
	std::string _variable;
	int _nbins;
	double _xlow;
	double _xhigh;
	std::vector<double> _bins;
};


class DoubleHistogramVariable{

    public:
        DoubleHistogramVariable(  
	    const std::string& name,
	    const std::string& primaryVariable,
            const std::string& secondaryVariable,
            const std::vector<double> primaryBins,
	    const std::vector<double> secondaryBins );
        DoubleHistogramVariable(  
	    const std::string& name,
            const std::string& primaryVariable,
	    const std::string& secondaryVariable,
            const std::vector<std::string> primaryBins,
	    const std::vector<std::string> secondaryBins );
        std::string name() const{ return _name; }
        std::string primaryVariable() const{ return _primaryVariable; }
	std::string secondaryVariable() const{ return _secondaryVariable; }
        std::vector<double> primaryBins() const{ return _primaryBins; }
	std::vector<double> secondaryBins() const{ return _secondaryBins; }
	std::string toString() const;
	unsigned int nPrimaryBins() const;
	unsigned int nSecondaryBins() const;
	unsigned int nTotalBins() const;
	int findBinNumber( double primaryValue, double secondaryValue ) const;

    private:
        std::string _name;
        std::string _primaryVariable;
	std::string _secondaryVariable;
        std::vector<double> _primaryBins;
	std::vector<double> _secondaryBins;
};

namespace variableTools{

    std::vector<HistogramVariable> readVariables( const std::string& txtFile );
    std::vector<DoubleHistogramVariable> readDoubleVariables( const std::string& txtFile );
    std::vector<HistInfo> makeHistInfoVec( const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms( 
	const std::vector<HistInfo>& histInfoVec );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(
	const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH2D> > initializeHistograms2D(
	const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(
	const std::vector<DoubleHistogramVariable>& vars );
}

#endif
