#ifndef bTaggingTools_H
#define bTaggingTools_H

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

namespace bTaggingTools{
    std::vector<std::string> mapToText(
	const std::map< std::string, std::map< std::string, std::map< int, double >>>& weightMap );
    std::map< std::string, std::map< std::string, std::map< int, double >>> textToMap(
	const std::vector<std::string>& lines,
	std::vector<std::string> eventSelections,
	std::vector<std::string> variations );
    std::map< std::string, std::map< std::string, std::map< int, double >>> textToMap(
        const std::string& txtFilePath,
	std::vector<std::string> eventSelections, 
        std::vector<std::string> variations );
}

#endif
