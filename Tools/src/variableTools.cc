/*
Tools for reading collections of histogram variables in txt format
*/

// include header
#include "../interface/variableTools.h"

HistogramVariable::HistogramVariable(	const std::string& name,
					const std::string& variable,
					int nbins,
					double xlow,
					double xhigh,
					const std::vector<double> bins ):
    _name( name ),
    _variable( variable ),
    _nbins( nbins ),
    _xlow( xlow ),
    _xhigh( xhigh ),
    _bins( bins )
    {}

HistogramVariable::HistogramVariable(	const std::string& name,
                                        const std::string& variable,
                                        const std::string& nbins,
                                        const std::string& xlow,
                                        const std::string& xhigh,
                                        const std::vector<std::string> bins ){
    _name = name;
    _variable = variable;
    _nbins = std::stoi(nbins);
    _xlow = std::stod(xlow);
    _xhigh = std::stod(xhigh);
    for( std::string binedge: bins ){ _bins.push_back(std::stod(binedge)); }
}

std::string HistogramVariable::toString() const{
    std::string res = "HistogramVariable(";
    res.append( " name: " + name() + "," );
    res.append( " variable: " + variable() + "," );
    res.append( " nbins: " + std::to_string(nbins()) + "," );
    res.append( " xlow: " + std::to_string(xlow()) + "," );
    res.append( " xhigh: " + std::to_string(xhigh()) + "," );
    res.append( " bin edges: " );
    for( double binedge: bins() ){ res.append( std::to_string(binedge)+"," ); }
    res.append(" )" );
    return res;
}

std::vector<HistogramVariable> variableTools::readVariables( const std::string& txtFile ){
    // read a vector of HistogramVariable objects from a txt file.
    // the txt files is assumed to be formatted as follows:
    // name [spaces] variable [spaces] nbins [spaces] xlow [spaces] xhigh
    // or alternatively:
    // name [spaces] variable [spaces] 0 [spaces] bin edges [separated by spaces]
    std::vector<HistogramVariable> res;
    // read the file line by line
    std::ifstream infile(txtFile);
    std::string line;
    while( std::getline(infile, line) ){
	std::stringstream ss;
	ss.str(line);
	// skip empty or commented lines
	if( line.size()<=1 ){ continue; }
	if( line[0]=='#' || line[0]=='/' ){ continue; }
	// split the line by space characters and make a vector
	std::vector<std::string> elems;
	std::string elem;
	while( std::getline(ss, elem, ' ') ){
	    if( elem.size()==0 ){ continue; }
	    elems.push_back(elem);
	}
	// check the length
        if( elems.size()<5 ){
            std::string msg = "ERROR in variableTools::readVariables:";
            msg += " line representing a histogram variable has unexpected length: ";
            for( std::string elem: elems ){ msg += elem + ","; }
            throw std::runtime_error(msg);
        }
        // case of fixed bin width
        if( elems[2]!="0" ){
	    // check the length
	    if( elems.size()!=5 ){
		std::string msg = "ERROR in variableTools::readVariables:";
		msg += " line representing a histogram variable has unexpected length: ";
		for( std::string elem: elems ){ msg += elem + ","; }
		throw std::runtime_error(msg);
	    }
	    // make a HistogramVariable and add to the result
	    HistogramVariable var = HistogramVariable( 
		elems[0], elems[1], elems[2], elems[3], elems[4]);
	    res.push_back( var );
	}
	// case of variable bin width
	else{
            // make a HistogramVariable and add to the result
            std::vector<std::string> binedges( elems.begin()+3, elems.end() );
            HistogramVariable var = HistogramVariable(
                elems[0], elems[1], std::to_string(binedges.size()-1), 
		binedges[0], binedges[binedges.size()-1], binedges);
	    res.push_back( var );
	}
    }
    return res;
}

std::vector<HistInfo> variableTools::makeHistInfoVec( 
    const std::vector<HistogramVariable>& vars ){
    // make a vector of HistInfo objects from a vector of HistogramVariable objects
    std::vector<HistInfo> histInfoVec;
    for( HistogramVariable var: vars ){
	histInfoVec.push_back( HistInfo(var.name(), var.variable(), var.nbins(), 
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

std::map< std::string, std::shared_ptr<TH2D> > variableTools::initializeHistograms2D(
    const std::vector<HistogramVariable>& vars ){
    // initialize 2D histograms (with same x and y bins)
    std::map< std::string, std::shared_ptr<TH2D> > res;
    for( HistogramVariable var: vars ){
	std::shared_ptr<TH2D> hist;
	// case of fixed bin width
	if( var.bins().size()==0 ){
	    hist = std::make_shared<TH2D>( 
		var.name().c_str(), var.name().c_str(),
		var.nbins(), var.xlow(), var.xhigh(),
		var.nbins(), var.xlow(), var.xhigh() );
	}
	// case of variable bin width
	else{
	    hist = std::make_shared<TH2D>(
		var.name().c_str(), var.name().c_str(),
		var.nbins(), &(var.bins()[0]),
                var.nbins(), &(var.bins()[0]) );
	}
	hist->SetDirectory(0);
        hist->Sumw2();
	res[var.name()] = hist;
    }
    return res;
}
