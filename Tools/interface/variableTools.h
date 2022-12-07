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
			    double xhigh );
	HistogramVariable(  const std::string& name,
			    const std::string& variable,
                            const std::string& nbins,
                            const std::string& xlow,
			    const std::string& xhigh );
	std::string name() const{ return _name; }
	std::string variable() const{ return _variable; }
	int nbins() const{ return _nbins; }
	double xlow() const{ return _xlow; }
	double xhigh() const{ return _xhigh; }
	std::string toString() const;

    private:
	std::string _name;
	std::string _variable;
	int _nbins;
	double _xlow;
	double _xhigh;
};

namespace variableTools{

    std::vector<HistogramVariable> readVariables( const std::string& txtFile );
    std::vector<HistInfo> makeHistInfoVec( const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms( 
	const std::vector<HistInfo>& histInfoVec );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(
	const std::vector<HistogramVariable>& vars );

}

#endif
