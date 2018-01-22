#ifndef Category_H
#define Category_H

//include c++ library classes
#include <string>
#include <vector>
#include <iostream>

class Category{
    using pos = size_t;
    public:
        Category(const std::vector < std::vector <std::string> >&);
        size_t getIndex(const std::vector<pos>&) const;
        size_t getRange(size_t ind) const { return ranges[ind]; }
        size_t size() const { return cat.size(); }
        std::string name(const std::vector<pos>&) const;
        std::string name(size_t ind) const { return cat[ind]; }
        const std::vector<std::string>& getCat() const { return cat; }
        std::vector < std::string > ::const_iterator cbegin() const { return cat.cbegin(); }
        std::vector < std::string > ::const_iterator cend() const { return cat.cend(); }
        const std::string& operator[](size_t ind) const { return cat[ind]; }
    private:
        std::vector< std::string > cat;
        std::vector< pos > ranges;
};
#endif 
