#include "../interface/drawLumi.h"

//include ROOT classes
#include "TLatex.h"
#include "TLine.h"

void drawLumi(TPad* pad, const TString& extraText, const TString& lumiText){
  	const float l = pad->GetLeftMargin();
  	const float t = pad->GetTopMargin();
  	const float r = pad->GetRightMargin();
  	//const float b = pad->GetBottomMargin();

	float CMSTextSize = pad->GetTopMargin()*0.8;
	float lumiTextSize = pad->GetTopMargin()*0.6;

	//float CMSTextOffset = pad->GetTopMargin()*0.2;
	float lumiTextOffset = pad->GetTopMargin()*0.2;
	
	pad->cd();
	//Define latex text to draw on plot
	TLatex latex(l,1+lumiTextOffset*t,"CMS");
	latex.SetNDC();
	latex.SetTextAngle(0);
	latex.SetTextColor(kBlack); 

	latex.SetTextFont(61);
	latex.SetTextAlign(11); 
	latex.SetTextSize(CMSTextSize);
	const float cmsX = latex.GetXsize();
	latex.DrawLatex(l,1  -t + lumiTextOffset,"CMS");

	const float extraTextSize = CMSTextSize*0.76;	 
	latex.SetTextFont(52);
	latex.SetTextSize(extraTextSize);
	latex.SetTextAlign(11);
	latex.DrawLatex(l + 1.2*cmsX, 1-t+lumiTextOffset, extraText);

	latex.SetTextFont(42);
	latex.SetTextAlign(31);
	latex.SetTextSize(lumiTextSize);  
	latex.DrawLatex(1-r,1-t+lumiTextOffset,lumiText);
	return;
}
