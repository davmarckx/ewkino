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
#include "systemTools.h"

// forward declarations
class HistogramVariable;
class DoubleHistogramVariable;


class Variable{
    // abstract base class for HistogramVariable and DoubleHistogramVariable
    
    public:
	Variable() {}; // default constructor
	Variable(   const std::string& name,
		    const std::string& variable,
		    const std::string& type ):
	    _name(name), _variable(variable), _type(type)
	    {}
	std::string name() const{ return _name; }
        std::string variable() const{ return _variable; }
        std::string type() const{ return _type; }
        virtual std::string toString() const = 0;
        virtual int findBinNumber( double value ) const = 0;
	virtual int findBinNumber( double primaryValue, double secondaryValue ) const = 0;
        virtual std::shared_ptr<TH1D> initializeHistogram( const std::string& histName ) const = 0;
        virtual std::string primaryVariable() const = 0;
        virtual std::string secondaryVariable() const = 0;
        virtual HistogramVariable primary() const = 0;
        virtual HistogramVariable secondary() const = 0;
        virtual HistogramVariable histogramVariable() const = 0;

    protected:
	std::string _name;
        std::string _variable;
        std::string _type;

};
	
class HistogramVariable: public Variable{

    public:
	HistogramVariable() {}; // default constructor
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
	int nbins() const{ return _nbins; }
	double xlow() const{ return _xlow; }
	double xhigh() const{ return _xhigh; }
        std::vector<double> bins() const{ return _bins; }
	std::string toString() const override;
	int findBinNumber( double value ) const override;
	std::shared_ptr<TH1D> initializeHistogram( const std::string& histName ) const override;
	std::shared_ptr<TH2D> initializeHistogram2D( const std::string& histName ) const;
	HistogramVariable histogramVariable() const override;
	// abstract functions needed by inheritance
	// note: nonsensical for single variables, so use dummy implementations for now.
	int findBinNumber( double primaryValue, double secondaryValue ) const override{
	    if(primaryValue<0) return -2;
	    if(secondaryValue<0) return -2;
	    return -2;
	}
	std::string primaryVariable() const override { return ""; }
	std::string secondaryVariable() const override { return ""; }
	HistogramVariable primary() const override { return HistogramVariable( "", "", 0, 0., 0. ); }
	HistogramVariable secondary() const override { return HistogramVariable( "", "", 0, 0., 0.); }

    private:
	int _nbins;
	double _xlow;
	double _xhigh;
	std::vector<double> _bins;
};


class DoubleHistogramVariable: public Variable{

    public:
	DoubleHistogramVariable() {}; // default constructor
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
        std::string primaryVariable() const override { return _primaryVariable; }
	std::string secondaryVariable() const override { return _secondaryVariable; }
        std::vector<double> primaryBins() const{ return _primaryBins; }
	std::vector<double> secondaryBins() const{ return _secondaryBins; }
	HistogramVariable primary() const override;
	HistogramVariable secondary() const override;
	std::string toString() const override;
	unsigned int nPrimaryBins() const;
	unsigned int nSecondaryBins() const;
	unsigned int nTotalBins() const;
	int findBinNumber( double primaryValue, double secondaryValue ) const override;
	int findPrimaryBinNumber( double primaryValue ) const;
	int findSecondaryBinNumber( double secondaryValue ) const;
	std::shared_ptr<TH1D> initializeHistogram( const std::string& histName ) const override;
	// abstract functions needed by inheritance
        // note: nonsensical for double variables, so use dummy implementations for now.
        int findBinNumber( double value ) const override{
            if(value<0) return -1;
            return -1;
        }
        HistogramVariable histogramVariable() const override {
	    return HistogramVariable( "", "", 0, 0., 0. );
	}

    private:
        std::string _primaryVariable;
	std::string _secondaryVariable;
        std::vector<double> _primaryBins;
	std::vector<double> _secondaryBins;
};


namespace variableTools{

    std::vector<HistogramVariable> readVariables( const std::string& txtFile );
    std::vector<DoubleHistogramVariable> readDoubleVariables( const std::string& txtFile );
    std::vector<std::shared_ptr<Variable>> readHistogramVariables( const std::string& txtFile );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(
	const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH2D> > initializeHistograms2D(
	const std::vector<HistogramVariable>& vars );
    std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(
	const std::vector<DoubleHistogramVariable>& vars );
}

#endif
