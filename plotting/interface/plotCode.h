#ifndef PlotScript
#define PlotScript
 
//import ROOT classes
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TH2D.h"
 
//Set histogram colors and lines
void histcol(TH1D*, const Color_t);

//Set Stack colors
void StackCol(TH1D*, const Color_t);

//Set Histogram labelsizes
void HistLabelSizes(TH1* h, const double xLabel = 0.045, const double xTitle = 0.05, const double yLabel = 0.045, const double yTitle = 0.045);

//Order histograms in terms of yields (biggest first)
void yieldOrder(TH1D**);

//return color corresponding to bkg
Color_t bkgColor(const std::string&, const std::string& analysis = ""); 

//Plot background stack and compare to observed yields
void plotDataVSMC(TH1D* data, TH1D** bkg, const std::string* names, const unsigned nBkg, const std::string& file, const std::string& analysis = "", const bool ylog = false, const bool normToData = false, const std::string& header = "", TH1D* bkgSyst = nullptr, const bool* isSMSignal = nullptr, TH1D** signal = nullptr, const std::string* sigNames = nullptr, const unsigned nSig = 0, const bool sigNorm = true);

//Plot and compare multiple distributions or single distribution
void plotHistograms(TH1D** histos, const unsigned nHistos, const std::string* names, const std::string& file, const bool normalized = false, const bool log = false);
void plotHistograms(TH1D* hist, const std::string& name, const std::string& file, const bool log = false);
void plotHistograms(std::vector<TH1D*>& histos, const std::string* names, const std::string& file, const bool normalized = false, const bool log = false);

// plot 2D histogram
void plot2DHistogram( TH2D* hist, const std::string& outputFileName, 
			const std::string& title = "",
			const std::string& drawOption = "colztexte",
			double aspectRatio = 1 );

#endif
