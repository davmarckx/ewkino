#ifndef ReweighterFakeRate_H
#define ReweighterFakeRate_H

#include "Reweighter.h"

//include c++ library classes
#include <map>
#include <memory>

//include ROOT classes
#include "TH2.h"

class ReweighterFakeRate : public Reweighter {

    public:
	ReweighterFakeRate( const std::string&, const std::string&, 
                            const std::string&, const std::string&,
			    const std::string& );

        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;

    private:
	std::shared_ptr< TH2D > electronFakeRateMap;
	std::shared_ptr< TH2D > electronFakeRateMapStatUp;
        std::shared_ptr< TH2D > electronFakeRateMapStatDown;
	std::shared_ptr< TH2D > electronFakeRateMapUp;
	std::shared_ptr< TH2D > electronFakeRateMapDown;

	std::shared_ptr< TH2D > muonFakeRateMap;
	std::shared_ptr< TH2D > muonFakeRateMapStatUp;
        std::shared_ptr< TH2D > muonFakeRateMapStatDown;
        std::shared_ptr< TH2D > muonFakeRateMapUp;
        std::shared_ptr< TH2D > muonFakeRateMapDown;
};


#endif
