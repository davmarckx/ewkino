#include "../interface/Met.h"

Met::Met( const TreeReader& treeReader,
	    const bool readAllJECVariations, const bool readGroupedJECVariations ):
    PhysicsObject( treeReader._met, 0., treeReader._metPhi, treeReader._met,
		    treeReader.is2016(), treeReader.is2016PreVFP(), treeReader.is2016PostVFP(),
		    treeReader.is2017(), treeReader.is2018() ),
    _pt_JECDown( treeReader._met_JECDown ), 
    _phi_JECDown( treeReader._metPhi_JECDown ),
    _pt_JECUp( treeReader._met_JECUp ),
    _phi_JECUp( treeReader._metPhi_JECUp ),
    _pt_UnclDown( treeReader._met_UnclDown ),
    _phi_UnclDown( treeReader._metPhi_UnclDown ),
    _pt_UnclUp( treeReader._met_UnclUp ),
    _phi_UnclUp( treeReader._metPhi_UnclUp )
{
    if( readAllJECVariations ){
	for( const auto mapEl: treeReader._corrMETx_JECSourcesUp ){
            std::string key = mapEl.first;
            key = stringTools::removeOccurencesOf(key,"_corrMETx_");
            key = stringTools::removeOccurencesOf(key,"_JECSourcesUp");
            _JECSources.push_back( key ); // assume they are the same for Up/Down and x/y!
        }
	for( const std::string key: _JECSources ){
	    _pxy_JECSourcesUp.insert( {key, std::make_pair( 
		treeReader._corrMETx_JECSourcesUp.at("_corrMETx_"+key+"_JECSourcesUp"), 
		treeReader._corrMETy_JECSourcesUp.at("_corrMETy_"+key+"_JECSourcesUp"))} );
	    _pxy_JECSourcesDown.insert( {key, std::make_pair( 
                treeReader._corrMETx_JECSourcesDown.at("_corrMETx_"+key+"_JECSourcesDown"),   
                treeReader._corrMETy_JECSourcesDown.at("_corrMETy_"+key+"_JECSourcesDown"))} );
	}
    }
    if( readGroupedJECVariations ){
        for( const auto mapEl: treeReader._corrMETx_JECGroupedUp ){
            std::string key = mapEl.first;
            key = stringTools::removeOccurencesOf(key,"_corrMETx_");
            key = stringTools::removeOccurencesOf(key,"_JECGroupedUp");
            _JECGrouped.push_back( key ); // assume they are the same for Up/Down and x/y!
        }
	for( const std::string key: _JECGrouped ){
            _pxy_JECGroupedUp.insert( {key, std::make_pair( 
                treeReader._corrMETx_JECGroupedUp.at("_corrMETx_"+key+"_JECGroupedUp"),  
                treeReader._corrMETy_JECGroupedUp.at("_corrMETy_"+key+"_JECGroupedUp"))} );
            _pxy_JECGroupedDown.insert( {key, std::make_pair(  
                treeReader._corrMETx_JECGroupedDown.at("_corrMETx_"+key+"_JECGroupedDown"),   
                treeReader._corrMETy_JECGroupedDown.at("_corrMETy_"+key+"_JECGroupedDown"))} );
        }
    }
}


Met Met::variedMet( const double pt, const double phi ) const{
    Met variedMet( *this );
    variedMet.setLorentzVector( pt, eta(), phi, pt );
    return variedMet;
}

Met Met::variedMetPxPy( const double px, const double py) const{
    LorentzVector varvector = lorentzVectorPxPyPzEnergy( px, py, 0, sqrt(px*px + py*py));
    return variedMet( varvector.pt(), varvector.phi() );
}

Met Met::MetJECDown() const{
    return variedMet( _pt_JECDown, _phi_JECDown );
}


Met Met::MetJECUp() const{
    return variedMet( _pt_JECUp, _phi_JECUp );
}


Met Met::MetUnclusteredDown() const{
    return variedMet( _pt_UnclDown, _phi_UnclDown );
}


Met Met::MetUnclusteredUp() const{
    return variedMet( _pt_UnclUp, _phi_UnclUp );
}

Met Met::MetJECDown( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    std::pair< double, double > newpxy = std::make_pair( 0,0 );
    for( std::string test: this->_JECSources ){
        if(source==test) newpxy = this->_pxy_JECSourcesDown.at(source);
    }
    for( std::string test: this->_JECGrouped ){
        if(source==test) newpxy = this->_pxy_JECGroupedDown.at(source);
    }
    return variedMetPxPy( newpxy.first, newpxy.second );
}

Met Met::MetJECDown( const std::string source, unsigned flavor, JetCollection nomJets ) const{
    // rough approach
    // generate lorentzvector corresponding to met
    LorentzVector ret = LorentzVector(pt(), eta(), phi(), energy());

    for (const auto& jetPtr : nomJets) {
        double ptNom = jetPtr->pt();
        if (jetPtr->hadronFlavor() != flavor) continue;
        
        std::shared_ptr<Jet> varJet;
        varJet = std::make_shared< Jet >(jetPtr->JetJECDown( source ));

        double ptdiff = ptNom - varJet->pt();

        ret += LorentzVector(ptdiff, 0., jetPtr->phi(), ptdiff);
    }

    Met variedMet( *this );
    variedMet.setLorentzVector(ret.pt(), eta(), ret.phi(), ret.pt() );
    return variedMet; 
}

Met Met::MetJECUp( const std::string source, unsigned flavor, JetCollection nomJets ) const{
    // rough approach
    // generate lorentzvector corresponding to met
    LorentzVector ret = LorentzVector(pt(), eta(), phi(), energy());

    for (const auto& jetPtr : nomJets) {
        double ptNom = jetPtr->pt();
        if (jetPtr->hadronFlavor() != flavor) continue;
        
        std::shared_ptr<Jet> varJet;
        varJet = std::make_shared< Jet >(jetPtr->JetJECUp( source ));

        double ptdiff = ptNom - varJet->pt();

        ret += LorentzVector(ptdiff, 0., jetPtr->phi(), ptdiff);
    }

    Met variedMet( *this );
    variedMet.setLorentzVector(ret.pt(), eta(), ret.phi(), ret.pt() );
    return variedMet;
}

Met Met::MetJECUp( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    std::pair< double, double > newpxy = std::make_pair( 0,0 );
    for( std::string test: this->_JECSources ){
        if(source==test) newpxy = this->_pxy_JECSourcesUp.at(source);
    }
    for( std::string test: this->_JECGrouped ){
        if(source==test) newpxy = this->_pxy_JECGroupedUp.at(source);
    }
    return variedMetPxPy( newpxy.first, newpxy.second );
}

Met Met::MetHEM1516Up() const{
    return variedMetPxPy( this->px(), this->py() );
}

Met Met::MetHEM1516Down() const{
    return variedMetPxPy( this->px(), this->py() );
}

Met Met::getVariedMet( const std::string& variation, JetCollection jets ) const{
    if( variation == "nominal" ){
        return *this;
    } else if( variation == "JECDown" ){
        return this->MetJECDown();
    } else if( variation == "JECUp" ){
        return this->MetJECUp();
    } else if( variation == "JERDown" ){
        return *this;
    } else if( variation == "JERUp" ){
        return *this;
    } else if( variation == "UnclDown" ){
        return this->MetUnclusteredDown();
    } else if( variation == "UnclUp" ){
        return this->MetUnclusteredUp();
    } else if( variation == "HEM1516Up" ){
	return this->MetHEM1516Up();
    } else if( variation == "HEM1516Down" ){
	return this->MetHEM1516Down();
    } else if( variation.find("_flavor") != std::string::npos){
        
        if(stringTools::stringEndsWith(variation,"Up")){
            std::string jecvar = variation.substr(0, variation.size()-2);
            unsigned flavor = std::stoul(&jecvar.back());
            jecvar = jecvar.substr(0, jecvar.size()-8);
            return this->MetJECUp( jecvar, flavor,jets );
        }
        else{
            std::string jecvar = variation.substr(0, variation.size()-4);
            unsigned flavor = std::stoul(&jecvar.back());
            jecvar = jecvar.substr(0, jecvar.size()-8);
            return this->MetJECDown( jecvar, flavor, jets );
        }

    } else if( stringTools::stringEndsWith(variation,"Up") ){
        std::string jecvar = variation.substr(0, variation.size()-2);
        return this->MetJECUp( jecvar );
    } else if( stringTools::stringEndsWith(variation,"Down") ){
        std::string jecvar = variation.substr(0, variation.size()-4);
        return this->MetJECDown( jecvar );
    } else {
        throw std::invalid_argument( std::string("### ERROR ### in Met::getVariedMet: ")
	+ "met variation " + variation + " is unknown." );
    }
}


std::vector< double > Met::metPtVariations() const{
    return {
        pt(),
        MetJECDown().pt(),
        MetJECUp().pt(),
        MetUnclusteredDown().pt(),
        MetUnclusteredUp().pt()
    };
}


double Met::maxPtAnyVariation() const{
    auto variations = metPtVariations();
    return *std::max_element( variations.cbegin(), variations.cend() );
}


double Met::minPtAnyVariation() const{
    auto variations = metPtVariations();
    return *std::min_element( variations.cbegin(), variations.cend() );
}


std::ostream& Met::print( std::ostream& os ) const{
    os << "Met : ";
    os << "(pt = " << pt() << ", phi = " << phi() << ")";
    os << " / pt_JECDown = " << _pt_JECDown << " / phi_JECDown = " << _phi_JECDown << " / pt_JECUp = " << _pt_JECUp << " / phi_JECUp = " << _phi_JECUp << " / pt_UnclDown = " << _pt_UnclDown << " / phi_UnclDown = " << _phi_UnclDown << " / pt_UnclUp = " << _pt_UnclUp << " / phi_UnclUp = " << _phi_UnclUp;
    return os;
}
