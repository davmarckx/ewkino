#ifndef JetSelector_H
#define JetSelector_H


//include other parts of framework
#include "Jet.h"

class JetSelector{

    public:
        JetSelector( Jet* jPtr ) : jetPtr( jPtr ){}
        
        bool isGood() const;

        bool isBTaggedLoose() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016() ){
                return isBTaggedLoose2016();
            } else if( jetPtr->is2017() ){
                return isBTaggedLoose2017();
            } else {
                return isBTaggedLoose2018();
            }
        }


        bool isBTaggedMedium() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016() ){
                return isBTaggedMedium2016();
            } else if( jetPtr->is2017() ){
                return isBTaggedMedium2017();
            } else {
                return isBTaggedMedium2018();
            }
        }


        bool isBTaggedTight() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016() ){
                return isBTaggedTight2016();
            } else if( jetPtr->is2017() ){
                return isBTaggedTight2017();
            } else {
                return isBTaggedTight2018();
            }
        }

        bool inBTagAcceptance() const;

    private:
        Jet* jetPtr;

	bool isGoodttH() const;
	bool isGoodtZq() const;

        bool isGoodBasettH() const;
        bool isGood2016ttH() const;
        bool isGood2017ttH() const;
        bool isGood2018ttH() const;

	bool isGoodBasetZq() const;
	bool isGood2016tZq() const;
	bool isGood2017tZq() const;
	bool isGood2018tZq() const;

        bool isBTaggedLoose2016() const;
        bool isBTaggedLoose2017() const;
        bool isBTaggedLoose2018() const;

        bool isBTaggedMedium2016() const;
        bool isBTaggedMedium2017() const;
        bool isBTaggedMedium2018() const;

        bool isBTaggedTight2016() const;
        bool isBTaggedTight2017() const;
        bool isBTaggedTight2018() const;
};

#endif
