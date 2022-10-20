#include "../interface/LeptonCollection.h"

//include c++ library classes 
#include <set>

//include other parts of code 
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"
#include "../../constants/particleMasses.h"


LeptonCollection::LeptonCollection( const TreeReader& treeReader ){

    //add muons to lepton collection
    for( unsigned m = 0; m < treeReader._nMu; ++m){
        push_back( Muon( treeReader, m ) );
    }

    //add electrons to lepton collection
    for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++ e){
        push_back( Electron( treeReader, e ) );
    } 

    //add taus to lepton collection
    for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t){
        push_back( Tau( treeReader, t ) );
    }
}


MuonCollection LeptonCollection::muonCollection() const{
    std::vector< std::shared_ptr< Muon > > muonVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isMuon() ){
            muonVector.push_back( std::static_pointer_cast< Muon >( leptonPtr ) );
        }
    }
    return MuonCollection( muonVector );
}


ElectronCollection LeptonCollection::electronCollection() const{
    std::vector< std::shared_ptr< Electron > > electronVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isElectron() ){
            electronVector.push_back( std::static_pointer_cast< Electron >( leptonPtr ) );
        }
    }
    return ElectronCollection( electronVector );
}


TauCollection LeptonCollection::tauCollection() const{
    std::vector< std::shared_ptr< Tau > > tauVector;
    for( const auto& leptonPtr : *this  ){
        if( leptonPtr->isTau() ){
            tauVector.push_back( std::static_pointer_cast< Tau >( leptonPtr ) );
        }
    }
    return TauCollection( tauVector );
}


LightLeptonCollection LeptonCollection::lightLeptonCollection() const{
    std::vector< std::shared_ptr< LightLepton > > lightLeptonVector;
    for( const auto leptonPtr : *this ){
        if( leptonPtr->isLightLepton() ){
            lightLeptonVector.push_back( std::static_pointer_cast< LightLepton >( leptonPtr ) );
        }
    }
    return LightLeptonCollection( lightLeptonVector );
}


void LeptonCollection::selectLooseLeptons(){
    selectObjects( &Lepton::isLoose );
}


void LeptonCollection::selectFOLeptons(){
    selectObjects( &Lepton::isFO );
}


void LeptonCollection::selectTightLeptons(){
    selectObjects( &Lepton::isTight );
}


LeptonCollection LeptonCollection::selectedCollection( void (LeptonCollection::*applySelection)() ) const{
    LeptonCollection lepCol( *this );
    (lepCol.*applySelection)();
    return lepCol;
}


LeptonCollection LeptonCollection::looseLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectLooseLeptons );
}


LeptonCollection LeptonCollection::FOLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectFOLeptons );
}


LeptonCollection LeptonCollection::tightLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectTightLeptons );
}


LeptonCollection LeptonCollection::leadingLeptonCollection( LeptonCollection::size_type numberOfLeptons ){
    if( numberOfLeptons > size() ){
        throw std::invalid_argument( "Trying to building collection of " + std::to_string( numberOfLeptons ) + " leptons for LeptonCollection of size " + std::to_string( size() ) + "." );
    }

    //make sure leptons are ordered by pT
    sortByPt();

    std::vector< std::shared_ptr< Lepton > > leptonVector;
    size_type counter = 0;
    for( const_iterator it = cbegin(); it != cend(); ++it ){

        //leptons are shared between collections!
        leptonVector.push_back( *it );
        ++counter;
        if( counter == numberOfLeptons ){
            break;
        }
    }
    return LeptonCollection( leptonVector );
}


void LeptonCollection::clean( bool (Lepton::*isFlavorToClean)() const, bool (Lepton::*isFlavorToCleanFrom)() const, bool (Lepton::*passSelection)() const, const double coneSize ){

    for( const_iterator l1It = cbegin(); l1It != cend(); ){
        Lepton& l1 = **l1It;

        //to increment iterator when no lepton is deleted 
        bool isDeleted = false;
        if( (l1.*isFlavorToClean)() ){

            for( const_iterator l2It = cbegin(); l2It != cend(); ++l2It ){

                //prevent comparing same objects 
                if( l1It == l2It ) continue;

                Lepton& l2 = **l2It;

                //make sure l2 passes required selection for cleaning 
                if( ! ( (l2.*isFlavorToCleanFrom)() && (l2.*passSelection)() ) ) continue;

                //clean within given cone size
                if( deltaR( l1, l2 ) < coneSize ){

                    l1It = erase( l1It );
                    isDeleted = true;

                    break;
                }
            }
        }
        if( ! isDeleted ){
            ++l1It;
        }
    }
}


void LeptonCollection::cleanElectronsFromMuons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isElectron, &Lepton::isMuon, passSelection, coneSize );
}


void LeptonCollection::cleanTausFromLightLeptons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isTau, &Lepton::isLightLepton, passSelection, coneSize );
}


void LeptonCollection::cleanElectronsFromLooseMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanElectronsFromFOMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isFO, coneSize );
}


void LeptonCollection::cleanTausFromLooseLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanTausFromFOLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isFO, coneSize );
}


void LeptonCollection::applyConeCorrection() const{
    for( const auto& leptonPtr : *this ){
        leptonPtr->applyConeCorrection();
    }
}


LeptonCollection::size_type LeptonCollection::numberOfMuons() const{
    return count( &Lepton::isMuon );
}


LeptonCollection::size_type LeptonCollection::numberOfElectrons() const{
    return count( &Lepton::isElectron );
}


LeptonCollection::size_type LeptonCollection::numberOfTaus() const{
    return count( &Lepton::isTau );
}


LeptonCollection::size_type LeptonCollection::numberOfLightLeptons() const{
    return count( &Lepton::isLightLepton );
}


LeptonCollection::size_type LeptonCollection::numberOfLooseLeptons() const{
    return count( &Lepton::isLoose );
}


LeptonCollection::size_type LeptonCollection::numberOfFOLeptons() const{
    return count( &Lepton::isFO );
}


LeptonCollection::size_type LeptonCollection::numberOfTightLeptons() const{
    return count( &Lepton::isTight );
}


//numbers signifying lepton flavor-charge combinations
enum LeptonCollection::FlavorCharge : unsigned int { OSSF_light, OSSF_tau, OS, SS, ZeroOrOneLepton };


LeptonCollection::FlavorCharge LeptonCollection::flavorChargeCombination() const{
    if( size() <= 1 ){
        return ZeroOrOneLepton;
    } 


    FlavorCharge currentState = SS;

    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        Lepton& l1 = **l1It;
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            Lepton& l2 = **l2It;

            if( l1.charge() != l2.charge() ){
                currentState = OS;

                if( sameFlavor( l1, l2 ) && l1.isLightLepton() ){

                    //OSSF_light can be returned because it has precedence over other combinations, as it is a requirement for Z boson reconstruction
                    return OSSF_light;
                } else if( sameFlavor( l1, l2 ) && l1.isTau() ){
                    currentState = OSSF_tau;
                }
            }
        }
    }
    return currentState;
}


bool LeptonCollection::hasOSSFPair() const{
    FlavorCharge flavorCharge = flavorChargeCombination();
    return ( ( flavorCharge == OSSF_light ) || ( flavorCharge == OSSF_tau ) );
}


bool LeptonCollection::hasLightOSSFPair() const{
    return ( flavorChargeCombination() == OSSF_light );
}


bool LeptonCollection::hasOSPair() const{
    FlavorCharge flavorCharge = flavorChargeCombination();
    return ( ( flavorCharge == OSSF_light ) || ( flavorCharge == OSSF_tau ) || ( flavorCharge == OS ) );
}


bool LeptonCollection::isSameSign() const{
    return ( flavorChargeCombination() == SS );
}


template< typename function_type> LeptonCollection::size_type LeptonCollection::numberOfUniquePairs( const function_type& pairSatisfiesCondition ) const{
    size_type numberOfPairs = 0;

    //there can be no pairs when there are less than 2 leptons
    if( size() <= 1 ){
        return 0;
    }

    //avoid double counting of leptons 
    std::set< const_iterator > usedLeptonIterators;

    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        if( usedLeptonIterators.find( l1It ) != usedLeptonIterators.cend() ) continue;
        Lepton& l1 = **l1It;
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            if( usedLeptonIterators.find( l2It ) != usedLeptonIterators.cend() ) continue;
            Lepton& l2 = **l2It;

            //if the lepton pair satisfies the condition, count it and make sure it can not be re-used 
            if( pairSatisfiesCondition( l1, l2 ) ){
                ++numberOfPairs;
                usedLeptonIterators.insert( {l1It, l2It} );

                //without this break there can be cases where l1 will be making a pair with yet another l2!
                break;
            }
        }
    }
    return numberOfPairs;
}


LeptonCollection::size_type LeptonCollection::numberOfUniqueOSSFPairs() const{
    return numberOfUniquePairs( oppositeSignSameFlavor ); 
}


LeptonCollection::size_type LeptonCollection::numberOfUniqueOSPairs() const{
    return numberOfUniquePairs( []( const Lepton& lhs, const Lepton& rhs ){ return lhs.charge() != rhs.charge(); } );
}


std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > LeptonCollection::bestZBosonCandidateIndicesAndMass() const{

    //currently the code only works when an OSSF pair is present 
    if( !hasLightOSSFPair() ){
        throw std::domain_error( "Finding the best leptonic Z decay candidate is only defined when two light leptons of opposite sign and same flavor are present in the event." );
    }

    //minimize mass difference with mZ over OSSF lepton pairs, and determine best mass and indices of the leptons forming this mass 
    double minDiff = std::numeric_limits< double >::max();
    double bestMass = 0;
    std::pair< size_type, size_type > indicesZCandidate = {99, 99};

    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        Lepton& l1 = **l1It;

        //only consider light leptons
        if( l1.isTau() ) continue;
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            Lepton& l2 = **l2It; 
            if( !oppositeSignSameFlavor( l1, l2 ) ) continue;

            double mass = ( l1 + l2 ).mass();
            double massDifference = std::abs( mass - particle::mZ );
            if( massDifference < minDiff ){
                minDiff = massDifference;
                bestMass = mass;
                indicesZCandidate = { l1It - cbegin(), l2It - cbegin() };
            }
        }
    }

    return { indicesZCandidate, bestMass };
}


std::pair< LeptonCollection::size_type, LeptonCollection::size_type > LeptonCollection::bestZBosonCandidateIndices() const{
    return bestZBosonCandidateIndicesAndMass().first;
}


double LeptonCollection::bestZBosonCandidateMass() const{
    return bestZBosonCandidateIndicesAndMass().second;
}


void LeptonCollection::removeTaus(){
    selectObjects( &Lepton::isLightLepton );
}

LeptonCollection LeptonCollection::buildVariedElectronCollection(
		    Electron (Electron::*variedElectron)() const ) const{
    std::vector< std::shared_ptr< Lepton > > lepVector;
    for( const auto& lepPtr : *this ){
	if( lepPtr->isElectron() ){
	    lepVector.push_back( std::make_shared< Electron >( 
	    	( (dynamic_cast<Electron*>(lepPtr.get()))->*variedElectron)() ) );
	}
	else{ lepVector.push_back( lepPtr ); }
    }
    return LeptonCollection( lepVector );
}

LeptonCollection LeptonCollection::electronScaleDownCollection() const{
    return buildVariedElectronCollection( &Electron::electronScaleDown );
}

LeptonCollection LeptonCollection::electronScaleUpCollection() const{
    return buildVariedElectronCollection( &Electron::electronScaleUp );
}

LeptonCollection LeptonCollection::electronResDownCollection() const{
    return buildVariedElectronCollection( &Electron::electronResDown );
}

LeptonCollection LeptonCollection::electronResUpCollection() const{
    return buildVariedElectronCollection( &Electron::electronResUp );
}

// sorting
void LeptonCollection::sortByPt( bool useConeCorrectedPt ){
    if( useConeCorrectedPt ){
	return sortByAttribute( [](const std::shared_ptr< Lepton >& lhs, 
				   const std::shared_ptr< Lepton >& rhs){ 
	    return lhs->pt() > rhs->pt(); } ); 
    } else{
	return sortByAttribute( [](const std::shared_ptr< Lepton >& lhs, 
                                   const std::shared_ptr< Lepton >& rhs){ 
            return lhs->uncorrectedPt() > rhs->uncorrectedPt(); } );
    }
}
