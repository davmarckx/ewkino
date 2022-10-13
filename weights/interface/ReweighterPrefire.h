#ifndef ReweighterPrefire_H
#define ReweighterPrefire_H

#include "Reweighter.h"


class ReweighterPrefire : public Reweighter {

    public:
        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;
};


class ReweighterPrefireMuon : public Reweighter {

    public:
	virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;
};


class ReweighterPrefireECAL : public Reweighter {

    public:
        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;
};


#endif 
