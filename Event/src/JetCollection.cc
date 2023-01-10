#include "../interface/JetCollection.h"


//include other parts of framework
#include "../interface/LeptonCollection.h"


JetCollection::JetCollection( const TreeReader& treeReader,
				const bool readAllJECVariations,
				const bool readGroupedJECVariations ){
    for( unsigned j = 0; j < treeReader._nJets; ++j ){
        this->push_back( Jet( treeReader, j, readAllJECVariations, readGroupedJECVariations ) ); 
    }
}


void JetCollection::selectGoodJets(){
    selectObjects( &Jet::isGood );
}


void JetCollection::selectGoodAnyVariationJets(){
    selectObjects( &Jet::isGoodAnyVariation );
}


void JetCollection::cleanJetsFromLeptons( const LeptonCollection& leptonCollection, bool (Lepton::*passSelection)() const, const double coneSize ){
    for( const_iterator jetIt = cbegin(); jetIt != cend(); ){
        Jet& jet = **jetIt;

        //increment iterator if jet is not deleted 
        bool isDeleted = false;
        for( LeptonCollection::const_iterator lIt = leptonCollection.cbegin(); lIt != leptonCollection.cend(); ++lIt ){
            Lepton& lepton = **lIt;

            //lepton must pass specified selection
            if( !(lepton.*passSelection)() ) continue;

            //remove jet if it overlaps with a selected lepton
            if( deltaR( jet, lepton ) < coneSize ){

                jetIt = erase( jetIt );
                isDeleted = true;
                break;
            }
        }
        if( !isDeleted ){
            ++jetIt;
        }
    }
}


JetCollection JetCollection::buildSubCollection( bool (Jet::*passSelection)() const ) const{
    std::vector< std::shared_ptr< Jet > > jetVector;
    for( const auto& jetPtr : *this ){
        if( ( *jetPtr.*passSelection )() ){

            //jets are shared between collections!
            jetVector.push_back( jetPtr );
        }
    }
    return JetCollection( jetVector );
}


JetCollection JetCollection::goodJetCollection() const{
    return buildSubCollection( &Jet::isGood );
}


JetCollection JetCollection::goodAnyVariationJetCollection() const{
    return buildSubCollection( &Jet::isGoodAnyVariation );
}


JetCollection JetCollection::looseBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedLoose );
}


JetCollection JetCollection::mediumBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedMedium );
}


JetCollection JetCollection::tightBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedTight );
}


JetCollection JetCollection::buildVariedCollection( Jet (Jet::*variedJet)() const ) const{
    std::vector< std::shared_ptr< Jet > > jetVector;
    for( const auto& jetPtr : *this ){

        //jets are NOT shared between collections!
        jetVector.push_back( std::make_shared< Jet >( (*jetPtr.*variedJet)() ) );
    }
    return JetCollection( jetVector );
}

JetCollection JetCollection::buildVariedCollection( Jet (Jet::*variedJet)(std::string) const, 
    std::string variationArg ) const{
    // similar to above but with argument passed to Jet::*variedJet
    std::vector< std::shared_ptr< Jet > > jetVector;
    for( const auto& jetPtr : *this ){

        //jets are NOT shared between collections!
        jetVector.push_back( std::make_shared< Jet >( (*jetPtr.*variedJet)( variationArg ) ) );
    }
    return JetCollection( jetVector );
}

JetCollection JetCollection::JECDownCollection() const{
    return buildVariedCollection( &Jet::JetJECDown );
}


JetCollection JetCollection::JECUpCollection() const{
    return buildVariedCollection( &Jet::JetJECUp );
}


JetCollection JetCollection::JERDownCollection() const{
    return buildVariedCollection( &Jet::JetJERDown );
}


JetCollection JetCollection::JERUpCollection() const{
    return buildVariedCollection( &Jet::JetJERUp );
}

JetCollection JetCollection::JECUpCollection( std::string source ) const{
    return buildVariedCollection( &Jet::JetJECUp, source );
}

JetCollection JetCollection::JECDownCollection( std::string source ) const{
    return buildVariedCollection( &Jet::JetJECDown, source );
}

JetCollection JetCollection::HEM1516UpCollection() const{
    return buildVariedCollection( &Jet::JetHEM1516Up );
}

JetCollection JetCollection::HEM1516DownCollection() const{
    return buildVariedCollection( &Jet::JetHEM1516Down );
}

JetCollection JetCollection::getVariedJetCollection( const std::string& variation) const{
    if( variation == "nominal" ){
        return this->goodJetCollection();
    } else if( variation == "JECDown" ){
        return this->JECDownCollection().goodJetCollection();
    } else if( variation == "JECUp" ){
        return this->JECUpCollection().goodJetCollection();
    } else if( variation == "JERDown" ){
        return this->JERDownCollection().goodJetCollection();
    } else if( variation == "JERUp" ){
        return this->JERUpCollection().goodJetCollection();
    } else if( variation == "UnclDown" ){
        return this->goodJetCollection();
    } else if( variation == "UnclUp" ){
        return this->goodJetCollection();
    } else if( variation == "HEM1516Up" ){
	return this->HEM1516UpCollection().goodJetCollection();
    } else if( variation == "HEM1516Down" ){
	return this->HEM1516DownCollection().goodJetCollection();
    } else if( stringTools::stringEndsWith(variation,"Up") ){
        std::string jecvar = variation.substr(0, variation.size()-2);
        return this->JECUpCollection( jecvar ).goodJetCollection();
    } else if( stringTools::stringEndsWith(variation,"Down") ){
        std::string jecvar = variation.substr(0, variation.size()-4);
        return this->JECDownCollection( jecvar ).goodJetCollection();
    } else {
        throw std::invalid_argument( std::string("ERROR in getVariedJetCollection: ")
	+ "jet variation " + variation + " is unknown." );
    }
}


JetCollection::size_type JetCollection::numberOfLooseBTaggedJets() const{
    return count( &Jet::isBTaggedLoose );
}


JetCollection::size_type JetCollection::numberOfMediumBTaggedJets() const{
    return count( &Jet::isBTaggedMedium );
}


JetCollection::size_type JetCollection::numberOfTightBTaggedJets() const{
    return count( &Jet::isBTaggedTight );
}


JetCollection::size_type JetCollection::numberOfGoodJets() const{
    return count( &Jet::isGood );
}


JetCollection::size_type JetCollection::numberOfGoodAnyVariationJets() const{
    return count( &Jet::isGoodAnyVariation );
}


void JetCollection::cleanJetsFromLooseLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isLoose, coneSize );
} 


void JetCollection::cleanJetsFromFOLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isFO, coneSize );
} 


void JetCollection::cleanJetsFromTightLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isTight, coneSize );
}


std::vector< JetCollection::size_type > JetCollection::countsAnyVariation( bool ( Jet::*passSelection )() const ) const{
    return {
        count( passSelection ), 
        JECDownCollection().count( passSelection ),
        JECUpCollection().count( passSelection ),
        JERDownCollection().count( passSelection ),
        JERUpCollection().count( passSelection )
    };
}


JetCollection::size_type JetCollection::minCountAnyVariation( bool ( Jet::*passSelection )() const ) const{
    const auto& counts = countsAnyVariation( passSelection );
    return *std::min_element( counts.cbegin(), counts.cend() );
}


JetCollection::size_type JetCollection::maxCountAnyVariation( bool ( Jet::*passSelection )() const ) const{
    const auto& counts = countsAnyVariation( passSelection );
    return *std::max_element( counts.cbegin(), counts.cend() );
}


JetCollection::size_type JetCollection::minNumberOfLooseBTaggedJetsAnyVariation() const{
    return minCountAnyVariation( &Jet::isBTaggedLoose );
}


JetCollection::size_type JetCollection::maxNumberOfLooseBTaggedJetsAnyVariation() const{
    return maxCountAnyVariation( &Jet::isBTaggedLoose );
}


JetCollection::size_type JetCollection::minNumberOfMediumBTaggedJetsAnyVariation() const{
    return minCountAnyVariation( &Jet::isBTaggedMedium );
}


JetCollection::size_type JetCollection::maxNumberOfMediumBTaggedJetsAnyVariation() const{
    return maxCountAnyVariation( &Jet::isBTaggedMedium );
}


JetCollection::size_type JetCollection::minNumberOfTightBTaggedJetsAnyVariation() const{
    return minCountAnyVariation( &Jet::isBTaggedTight );
}


JetCollection::size_type JetCollection::maxNumberOfTightBTaggedJetsAnyVariation() const{
    return maxCountAnyVariation( &Jet::isBTaggedTight );
}
