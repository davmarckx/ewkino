#ifndef DRAW_LUMI_H
#define DRAW_LUMI_H
#include "TPad.h"
#include "TString.h"

void drawLumi(TPad*, const TString& extraText = "Preliminary", const TString& lumiText = "41.4 fb^{-1} (13 TeV)");

#endif

