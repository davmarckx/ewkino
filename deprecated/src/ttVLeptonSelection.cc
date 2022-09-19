#include "../interface/treeReader.h"
#include "../interface/kinematicTools.h"

/*
 * Implementation of lepton selection from the different analyses from tZq and ttV
 */

bool treeReader::lepIsGoodtZq(const unsigned leptonIndex) const{
    return lepIsGood(leptonIndex);
}

bool treeReader::lepIsGoodttZ3l2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
}

bool treeReader::lepIsGoodttZ3l2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;

}

bool treeReader::lepIsGoodttZ3l(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttZ3l2016(leptonIndex);
    } else {
        return lepIsGoodttZ3l2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodttZ4l2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;

}

bool treeReader::lepIsGoodttZ4l2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;

}

bool treeReader::lepIsGoodttZ4l(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttZ4l2016(leptonIndex);
    } else {
        return lepIsGoodttZ4l2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodttW2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;

}

bool treeReader::lepIsGoodttW2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;

}

bool treeReader::lepIsGoodttW(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttW2016(leptonIndex);
    } else {
        return lepIsGoodttW2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodMultiAnalysis(const std::string& analysis, const unsigned leptonIndex) const{
    if(analysis == "tZq"){
        return lepIsGoodtZq(leptonIndex);
    } else if(analysis == "ttZ3l"){
        return lepIsGoodttZ3l(leptonIndex);
    } else if(analysis == "ttZ4l"){
        return lepIsGoodttZ4l(leptonIndex);
    } else if(analysis == "ttW"){
        return lepIsGoodttW(leptonIndex);
    } else {
        std::cerr << "Error: unrecognized analysis option in lepton selection" << std::endl;
        return false;
    }
}

