/*
Tools for reading collections of histogram variables in txt format
*/

// include header
#include "../interface/variableTools.h"


// class definition of HistogramVariable //

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


// class definition of DoubleHistogramVariable

DoubleHistogramVariable::DoubleHistogramVariable(
    const std::string& name,
    const std::string& primaryVariable,
    const std::string& secondaryVariable,
    const std::vector<double> primaryBins,
    const std::vector<double> secondaryBins ):
    _name( name ),
    _primaryVariable( primaryVariable ),
    _secondaryVariable( secondaryVariable ),
    _primaryBins( primaryBins ),
    _secondaryBins( secondaryBins )
    {}

DoubleHistogramVariable::DoubleHistogramVariable(
    const std::string& name,
    const std::string& primaryVariable,
    const std::string& secondaryVariable,
    const std::vector<std::string> primaryBins,
    const std::vector<std::string> secondaryBins ){
    _name = name;
    _primaryVariable = primaryVariable;
    _secondaryVariable = secondaryVariable;
    for( std::string binedge: primaryBins ){ _primaryBins.push_back(std::stod(binedge)); }
    for( std::string binedge: secondaryBins ){ _secondaryBins.push_back(std::stod(binedge)); }
}

std::string DoubleHistogramVariable::toString() const{
    std::string res = "DoubleHistogramVariable(";
    res.append( " name: " + name() );
    res.append( "\n" );
    res.append( " primary variable: " + primaryVariable() + "," );
    res.append( " primary bin edges: " );
    for( double binedge: primaryBins() ){ res.append( std::to_string(binedge)+"," ); }
    res.append( "\n" );
    res.append( " secondary variable: " + secondaryVariable() + "," );
    res.append( " secondary bin edges: " );
    for( double binedge: secondaryBins() ){ res.append( std::to_string(binedge)+"," ); }
    res.append(" )" );
    return res;
}

unsigned int DoubleHistogramVariable::nPrimaryBins() const{
    return _primaryBins.size()-1;
}

unsigned int DoubleHistogramVariable::nSecondaryBins() const{
    return _secondaryBins.size()-1;
}

unsigned int DoubleHistogramVariable::nTotalBins() const{
    return nPrimaryBins()*nSecondaryBins();
}

int findBin( const std::vector<double>& binedges, double value ){
    // small helper function to retrieve correct bin number
    int bin = 0;
    for( double binedge: binedges ){
        if( binedge>value ) break;
        bin++;
    }
    // add underflow to first bin
    if( bin==0 ) bin = 1;
    // add overflow to last bin
    if( bin==(int)binedges.size() ) bin = (int)binedges.size()-1;
    return bin;
}

int DoubleHistogramVariable::findBinNumber( double primaryValue, double secondaryValue ) const{
    int primaryBinNb = findBin( _primaryBins, primaryValue );
    int secondaryBinNb = findBin( _secondaryBins, secondaryValue );
    return nPrimaryBins()*(secondaryBinNb-1) + primaryBinNb;
}

int DoubleHistogramVariable::findPrimaryBinNumber( double primaryValue ) const{
    int primaryBinNb = findBin( _primaryBins, primaryValue );
    return primaryBinNb;
}

int DoubleHistogramVariable::findSecondaryBinNumber( double secondaryValue ) const{
    int secondaryBinNb = findBin( _secondaryBins, secondaryValue );
    return secondaryBinNb;
}


// helper functions

std::vector<std::vector<std::string>> readTxtLines( const std::string& txtFile ){
    // helper function for reading txt files
    // returns a vector, where each element represents a line in the txt file,
    // and consists of a vector of strings (space-separated elements on that line).
    std::vector< std::vector<std::string> > res;
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
	res.push_back(elems);
    }
    return res;
}

std::vector<HistogramVariable> variableTools::readVariables( const std::string& txtFile ){
    // read a vector of HistogramVariable objects from a txt file.
    // the txt files is assumed to be formatted as follows:
    // name [spaces] variable [spaces] nbins [spaces] xlow [spaces] xhigh
    // or alternatively:
    // name [spaces] variable [spaces] 0 [spaces] bin edges [separated by spaces]
    std::vector<HistogramVariable> res;
    // read the file
    std::vector<std::vector<std::string>> fileContent;
    fileContent = readTxtLines(txtFile);
    for( std::vector<std::string> elems: fileContent ){
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

std::vector<DoubleHistogramVariable> variableTools::readDoubleVariables( 
    const std::string& txtFile ){
    // read a vector of DoubleHistogramVariable objects from a txt file.
    // the txt files is assumed to be formatted as follows:
    // name [spaces] variable [spaces] 0 [spaces] bin edges [separated by spaces]
    // and the name should be the same for each pair of two consecutive lines.
    std::vector<DoubleHistogramVariable> res;
    // read the file
    std::vector<std::vector<std::string>> fileContent;
    fileContent = readTxtLines(txtFile);
    unsigned int lineIndex = 0;
    while( lineIndex<fileContent.size() ){
	if( lineIndex+1 >= fileContent.size() ){
	    std::string msg = "ERROR in variableTools::readDoubleVariables:";
	    msg += " number of lines in txt files appears to be odd, while only even is allowed.";
	    throw std::runtime_error(msg);
	}
	std::vector<std::string> line1 = fileContent[lineIndex];
	std::vector<std::string> line2 = fileContent[lineIndex+1];
        // check the length
        if( line1.size()<5 || line2.size()<5 ){
            std::string msg = "ERROR in variableTools::readDoubleVariables:";
            msg += " lines representing a double histogram variable have unexpected length: \n";
            for( std::string elem: line1 ){ msg += elem + ","; }
	    msg += "\n";
	    for( std::string elem: line2 ){ msg += elem + ","; }
            throw std::runtime_error(msg);
        }
	// check the name
	if( line1[0]!=line2[0] ){
	    std::string msg = "ERROR in variableTools::readDoubleVariables:";
            msg += " lines representing a double histogram variable have different name: \n";
            for( std::string elem: line1 ){ msg += elem + ","; }
            msg += "\n";
            for( std::string elem: line2 ){ msg += elem + ","; }
            throw std::runtime_error(msg);
	}
        // make a DoubleHistogramVariable and add to the result
        std::vector<std::string> primaryBins( line1.begin()+3, line1.end() );
	std::vector<std::string> secondaryBins( line2.begin()+3, line2.end() );
        DoubleHistogramVariable var = DoubleHistogramVariable(
            line1[0], line1[1], line2[1],
	    primaryBins, secondaryBins );
        res.push_back( var );
	// update index
	lineIndex += 2;
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

std::map< std::string, std::shared_ptr<TH1D> > variableTools::initializeHistograms(
    const std::vector<DoubleHistogramVariable>& vars ){
    // initialize 1D histograms from a vector of DoubleHistogramVariable objects
    // note: the bin edges are ranging from 0.5 to nTotalBins+0.5 in steps of 1;
    //       always fill by bin number rather than by value!
    //       (use e.g. DoubleHistogramVariable::findBinNumber)
    std::map< std::string, std::shared_ptr<TH1D> > res;
    for( DoubleHistogramVariable var: vars ){
        std::shared_ptr<TH1D> hist;
	hist = std::make_shared<TH1D>(
                var.name().c_str(), var.name().c_str(),
                var.nTotalBins(), 0.5, var.nTotalBins()+0.5 );
        hist->SetDirectory(0);
        hist->Sumw2();
        res[var.name()] = hist;
    }
    return res;
}
