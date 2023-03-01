#ifndef ReweighterTrigger_H
#define ReweighterTrigger_H

#include "Reweighter.h"

//include c++ library classes
#include <map>
#include <memory>

//include ROOT classes
#include "TH2.h"

class ReweighterTrigger : public Reweighter {

    public:
	// UL constructor
<<<<<<< HEAD
	ReweighterTrigger( const std::string&, const std::string& );


=======
	ReweighterTrigger( const std::string&, const std::string&, const double );
>>>>>>> master

        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;

    private:
<<<<<<< HEAD
=======
	double systUnc = 0;
>>>>>>> master
	bool isUL = false; // set to true if UL constructor is used
	// for UL:
	std::shared_ptr< TH2 > triggerWeights_ee;
        std::shared_ptr< TH2 > triggerWeights_mm;
        std::shared_ptr< TH2 > triggerWeights_me;
        std::shared_ptr< TH2 > triggerWeights_em;

        double weight( const Event&, const std::shared_ptr< TH2 >& ) const;
};


#endif
