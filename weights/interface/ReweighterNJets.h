#ifndef ReweighterNJets_H
#define ReweighterNJets_H

#include "Reweighter.h"


class ReweighterNJets : public Reweighter {

    public:

	ReweighterNJets( const std::map<unsigned int, double>&, bool );

	double getUnc( const Event& ) const;

        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;

    private:
	
	unsigned int _nMax = 0;
	bool _doBJets = false;
	std::map<unsigned int, double> _nJetUncertainties;
};

#endif 
