/*
Tools for reading collections of histogram variables in txt format
*/

// include header
#include "../interface/variableTools.h"

HistogramVariable::HistogramVariable(	const std::string& name,
					const std::string& title,
					int nbins,
					double xlow,
					double xhigh ):
    _name( name ),
    _title( title ),
    _nbins( nbins ),
    _xlow( xlow ),
    _xhigh( xhigh )
    {}

HistogramVariable::HistogramVariable(	const std::string& name,
                                        const std::string& title,
                                        const std::string& nbins,
                                        const std::string& xlow,
                                        const std::string& xhigh ){
    _name = name;
    _title = title;
    _nbins = std::stoi(nbins);
    _xlow = std::stod(xlow);
    _xhigh = std::stod(xhigh);
}

std::vector<HistogramVariable> variableTools::readVariables( const std::string& txtFile ){
    // read a vector of HistogramVariable objects from a txt file.
    // the txt files is assumed to be formatted as follows:
    // name [tab] title [tab] nbins [tab] xlow [tab] xhigh [newline]
    std::vector<HistogramVariable> res;
    // read the file line by line
    std::ifstream infile(txtFile);
    std::string line;
    while( std::getline(infile, line) ){
	// split the line by tab characters and make a vector
	std::vector<std::string> elems;
	std::stringstream ss;
	ss.str(line);
	std::string elem;
	while( std::getline(ss, elem, '\t') ){
	    elems.push_back(elem);
	}
	// check the length
	if( elems.size()!=5 ){
	    std::string msg = "ERROR in variableTools::readVariables:";
	    msg += " line representing a histogram variable has unexpected length: ";
	    for( std::string elem: elems ){ msg += elem + ","; }
	    throw std::runtime_error(msg);
	}
	// make a HistogramVariable and add to the result
	HistogramVariable var = HistogramVariable( elems[0], elems[1], elems[2], 
						   elems[3], elems[4]);
	res.push_back( var );	
    }
    return res;
}

std::vector<HistInfo> variableTools::makeHistInfoVec( 
    const std::vector<HistogramVariable>& vars ){
    // make a vector of HistInfo objects from a vector of HistogramVariable objects
    std::vector<HistInfo> histInfoVec;
    for( HistogramVariable var: vars ){
	histInfoVec.push_back( HistInfo(var.name(), var.title(), var.nbins(), 
					var.xlow(), var.xhigh()) );
    }
    return histInfoVec;
}

std::map< std::string, std::shared_ptr<TH1D> > variableTools::initializeHistograms(
    const std::vector<HistInfo>& histInfoVec ){
    // initialize histograms from a vector of HistInfo objects
    std::map< std::string, std::shared_ptr<TH1D> > histograms;
    for( HistInfo h: histInfoVec ){
        histograms[h.name()] = h.makeHist( (std::string(h.name())).c_str() );
        histograms[h.name()]->SetDirectory(0);
    }
    return histograms;
}

std::map< std::string, std::shared_ptr<TH1D> > variableTools::initializeHistograms(
    const std::vector<HistogramVariable>& vars ){
    // initialize histograms from a vector of HistogramVariable objects
    std::vector<HistInfo> histInfoVec = makeHistInfoVec( vars );
    return initializeHistograms( histInfoVec );
}
