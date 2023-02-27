#include "../interface/bTaggingTools.h"

std::vector<std::string> bTaggingTools::mapToText(
    const std::map< std::string, std::map< std::string, std::map< int, double >>>& weightMap ){
    // input: map of structure [event selection][variation][number of jets] -> weight
    // output: lines to write to txt file
    // - first line: event selections, separated by spaces
    // - second line: variations, separated by spaces
    // - next lines: event selection, space, variation, space,
    //               (njets, space, weight) for all njets.
    std::vector<std::string> res;
    std::vector<std::string> bulkLines;
    std::string firstLine = "";
    std::string secondLine = "";
    for( const auto &iter: weightMap ){
        firstLine.append( iter.first + " " );
        secondLine = "";
        for( const auto &iter2: weightMap.at(iter.first) ){
            secondLine.append( iter2.first + " " );
            std::string bulkLine = "";
            bulkLine.append( iter.first + " " + iter2.first + " " );
            for( const auto &iter3: weightMap.at(iter.first).at(iter2.first) ){
                bulkLine.append( std::to_string(iter3.first) + " " );
                bulkLine.append( std::to_string(iter3.second) + " " );
            }
            bulkLines.push_back(bulkLine);
        }
    }
    res.push_back(firstLine);
    res.push_back(secondLine);
    for( std::string s: bulkLines ){ res.push_back(s); }
    return res;
}

std::string getLine(
    const std::vector<std::string>& lines,
    const std::string& eventSelection,
    const std::string& variation ){
    // help function for textToMap
    for(std::string line: lines){
	if( line.rfind(eventSelection+" "+variation, 0) == 0 ){
	    return line;
	}
    }
    std::string msg = "ERROR in getLine: line for event selection " + eventSelection;
    msg += " and variation " + variation + " not found in input file";
    throw std::runtime_error(msg);
}

std::map< std::string, std::map< std::string, std::map< int, double >>> bTaggingTools::textToMap(
    const std::vector<std::string>& lines,
    std::vector<std::string> eventSelections,
    std::vector<std::string> variations ){
    // reverse operation of mapToText
    bool autoEventSelections = (eventSelections[0]=="auto");
    bool autoVariations = (variations[0]=="auto");
    if( autoEventSelections ){
	eventSelections.clear();
	std::stringstream firstLineStream(lines[0]);
	std::string temp;
	while( std::getline(firstLineStream, temp, ' ') ){
	    if( temp.size()==0 ){ continue; }
	    eventSelections.push_back(temp);
	}
    }
    if( autoVariations ){
	std::vector<std::string> variations;
	std::stringstream secondLineStream(lines[1]);
	std::string temp;
	while( std::getline(secondLineStream, temp, ' ') ){
	    if( temp.size()==0 ){ continue; }
	    variations.push_back(temp);
	}
    }
    std::map< std::string, std::map< std::string, std::map< int, double >>> weightMap;
    for( std::string es: eventSelections ){
        for( std::string var: variations ){
            std::string line = getLine(lines, es, var);
            std::vector<std::string> elems;
            std::stringstream lineStream(line);
	    std::string temp;
            while( std::getline(lineStream, temp, ' ') ){
                if( temp.size()==0 ){ continue; }
                elems.push_back(temp);
            }
            unsigned int inlineCounter = 2;
            std::map<int,double> lineMap;
            while( inlineCounter < elems.size() ){
                lineMap[ std::stoi(elems[inlineCounter]) ] = std::stod(elems[inlineCounter+1]);
                inlineCounter = inlineCounter + 2;
            }
            weightMap[es][var] = lineMap;
        }
    }
    return weightMap;
}

std::map< std::string, std::map< std::string, std::map< int, double >>> bTaggingTools::textToMap(
    const std::string& txtFilePath,
    std::vector<std::string> eventSelections,
    std::vector<std::string> variations ){
    // same as textToMap but starting from a txt file path instead of its contents
    std::vector<std::string> lines;
    std::ifstream infile(txtFilePath);
    std::string line;
    while( std::getline(infile, line) ){ lines.push_back(line); }
    return textToMap(lines, eventSelections, variations);
}
