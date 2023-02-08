// Tools for manipulating systematics histograms

// include header
#include "../interface/systematicTools.h"


std::string systematicTools::systematicType( const std::string systematic ){
    // return a string identifier of the type of a sytematic. 
    // useful to decide more streamlined how they are processed when looping over them.

    // regular JEC, JER and Unclustered energy
    if(systematic=="JEC" or systematic=="JER" or systematic=="Uncl"){
        return std::string("acceptance");
    }
    // special selection for HEM15/16 issue
    if(systematic=="HEM1516"){
	return std::string("acceptance");
    }
    // split JEC
    if(systematic=="JECAll" or systematic=="JECGrouped"){
        return std::string("jecsplit");
    }
    // reweighting uncertainties 
    if(
	    systematic=="muonReco"
            or systematic=="muonIDSyst"
            or systematic=="muonIDStat"
            or systematic=="electronIDSyst"
            or systematic=="electronIDStat"
            or systematic=="pileup"
	    or systematic=="prefire"
            or systematic=="njets"
            or systematic=="nbjets"
            or systematic=="bTag_heavy" 
	    // (old method, replaced by bTag_shape)
            or systematic=="bTag_light" 
	    // (old method, replaced by bTag_shape)
    ) return std::string("weight");
    // b-tag shape uncertainty
    if(systematic=="bTag_shape") return std::string("bTag_shape");
    // scale uncertainties
    if(     systematic=="fScale"
            or systematic=="fScaleNorm"
            or systematic=="rScale"
            or systematic=="rScaleNorm"
            or systematic=="rfScales"
            or systematic=="rfScalesNorm"
    ) return std::string("scale");
    // pdf or qcd scale variations
    if(systematic=="pdfShapeVar" or systematic=="pdfNorm"
        or systematic=="qcdScalesShapeVar" or systematic=="qcdScalesNorm"){
        return std::string("lhe"); // special as need to take envelope/RMS
    }
    // parton shower variations
    if(systematic=="isrShape" or systematic=="isrNorm"
        or systematic=="fsrShape" or systematic=="fsrNorm"){
        return std::string("ps");
    }
    // electron reco uncertainty
    if(systematic=="electronReco") return std::string("electronReco");
    // if none of the above, throw an error
    std::string msg = "ERROR in systematicTools::systematicType:";
    msg.append( " systematic '" + systematic + "' not recognized." );
    throw std::runtime_error(msg);
}


void systematicTools::fillEnvelope( 
    std::map< std::string, std::shared_ptr<TH1D> > histMap,
    std::string upName, std::string downName, std::string tag ){
    // calculate bin-per-bin envelope of a series of histograms containing "tag" in name
    // note: the envelope entries must be initialized (to e.g. nominal) in the histMap 
    //       before calling this function!
    
    // check if required histograms are present
    if( histMap.find(upName)==histMap.end() ){
	std::string msg = "ERROR in systematicTools::fillEnvelope:";
	msg.append( " expected histogram '"+upName+"' not found." );
	throw std::runtime_error(msg);
    }
    if( histMap.find(downName)==histMap.end() ){
        std::string msg = "ERROR in systematicTools::fillEnvelope:";
        msg.append( " expected histogram '"+downName+"' not found." );
        throw std::runtime_error(msg);
    }
    std::shared_ptr<TH1D> upHist = histMap.at(upName);
    std::shared_ptr<TH1D> downHist = histMap.at(downName);
    // loop over histograms and select relevant ones
    for( const auto& el : histMap ){
        if(!stringTools::stringContains(el.first,tag)) continue;
        if(el.first==upName) continue;
        if(el.first==downName) continue;
        std::shared_ptr<TH1D> hist = el.second;
	// adapt the envelope
        for(int i=1; i<hist->GetNbinsX()+1; ++i){
            if( hist->GetBinContent(i) > upHist->GetBinContent(i) ){
                upHist->SetBinContent(i,hist->GetBinContent(i));
            }
            if( hist->GetBinContent(i) < downHist->GetBinContent(i) ){
                downHist->SetBinContent(i,hist->GetBinContent(i));
            }
        }
    }
}


void systematicTools::fillRMS( 
    std::map< std::string,std::shared_ptr<TH1D> > histMap,
    std::string upName, std::string downName, std::string tag ){
    // calculate bin-per-bin rms of a series of histograms containing "tag" in name
    // note: the rms entries must be added to the histMap as empty hist before this function!
    // note: the histmap must contain the nominal histogram as well!
    
    // check if all required histograms are present
    if( histMap.find("nominal")==histMap.end() ){
        std::string msg = "ERROR in systematicTools::fillRMS:";
        msg.append( " expected histogram 'nominal' not found." );
        throw std::runtime_error(msg);
    }
    if( histMap.find(upName)==histMap.end() ){
        std::string msg = "ERROR in systematicTools::fillRMS:";
        msg.append( " expected histogram '"+upName+"' not found." );
        throw std::runtime_error(msg);
    }
    if( histMap.find(downName)==histMap.end() ){
        std::string msg = "ERROR in systematicTools::fillRMS:";
        msg.append( " expected histogram '"+downName+"' not found." );
        throw std::runtime_error(msg);
    }
    std::shared_ptr<TH1D> nominalHist = histMap.at("nominal");
    std::shared_ptr<TH1D> varRMSHist = std::shared_ptr<TH1D>( dynamic_cast<TH1D*>(nominalHist->Clone()) );
    varRMSHist->Reset();
    std::shared_ptr<TH1D> upHist = histMap.at(upName);
    std::shared_ptr<TH1D> downHist = histMap.at(downName);
    int nElements = 0;
    // loop over histograms and select relevant ones
    for( const auto& el : histMap ){
        if(!stringTools::stringContains(el.first,tag)) continue;
        if(el.first==upName) continue;
        if(el.first==downName) continue;
        std::shared_ptr<TH1D> hist = el.second;
        nElements++;
	// add to sum of squares of differences
        for(int i=1; i<hist->GetNbinsX()+1; ++i){
            double var = hist->GetBinContent(i) - nominalHist->GetBinContent(i);
            varRMSHist->SetBinContent(i,varRMSHist->GetBinContent(i)+var*var);
        }
    }
    // catch exception of zero elements to average over 
    // (the denominator can be changed arbitrarily since numerator is zero as well)
    if( nElements==0 ) nElements=1;
    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
	// take mean and square root
        varRMSHist->SetBinContent(i,std::sqrt(varRMSHist->GetBinContent(i)/nElements));
	// set bin content
        upHist->SetBinContent(i,nominalHist->GetBinContent(i)
                                +varRMSHist->GetBinContent(i));
        downHist->SetBinContent(i,nominalHist->GetBinContent(i)
                                  -varRMSHist->GetBinContent(i));
    }
}
