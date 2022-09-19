/*
class to easily make and write a file of root trees for BDT training
*/

#ifndef TrainingTree_H
#define TrainingTree_H

//include other parts of code
#include "Category.h"
#include "Sample.h"

//include c++ library classes
#include <vector>
#include <map>

//include ROOT classes
#include "TFile.h"
#include "TTree.h"

class TrainingTree{
    public:
        //TrainingTree(const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector<std::string>&);
        TrainingTree(const std::string&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::map< std::string, float >&, const bool isSignal);
        TrainingTree(const std::string&, const Sample&, const Category&, const std::map<std::string, float>&, const bool isSignal);
        TrainingTree(const std::string&, const Sample&, const std::vector < std::vector < std::string > >&, const std::map< std::string, float >&, const bool isSignal);
        ~TrainingTree();

        //fill tree entry for given category
        void fill(const std::vector< size_t>&, const std::map< std::string, float>&);
        
        //void write(const std::string& );
    private:
        std::shared_ptr< Category > category;
        std::shared_ptr< Sample > sample;
        std::map< std::string, float > variableMap;
        std::vector< TTree* > trainingTrees;
        TFile* treeFile;
        
        //set up tree branch adresses
        void setBranches(const bool);
    
        //name of tree, depending on whether the category and whether it is signal
        std::string treeName(const size_t, const bool);

        //fill tree entry in given category's tree
        void fill(const size_t, const std::map<std::string, float>&);
};

//merge all training trees in given directory
void mergeTrees(const std::string&);

#endif
