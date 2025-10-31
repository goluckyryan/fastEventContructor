#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TString.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"

#include "misc.h"

#define MWIN 350.;

double ZeroCrossing(std::vector<std::pair<double, double>> points){

  if(points.size() < 2 || points.size() > 3) return TMath::QuietNaN(); // not enough points to find zero crossing

  if( points.size() == 3){ 
    
    double y0 = points[0].first;
    double y1 = points[1].first;
    double y2 = points[2].first;
    double x0 = points[0].second;
    double x1 = points[1].second;
    double x2 = points[2].second;
    
    // solve the quadratic equation ax^2 + bx + c = 0
    double a = (x0 - x1) * (x0 - x2) * (x1 - x2);
    double b = x0 * x2 * y1 * (-x0 + x2) + x1 * x1 * (x2 * y0 - x0 * y2) +  x1 * (-x2 * x2 * y0 + x0 * x0 * y2) ; 

    return b /a;


  }else if( points.size() == 2 ){

    double x1 = points[0].first;
    double x2 = points[1].first;
    double y1 = points[0].second;
    double y2 = points[1].second;

    if(y1 * y2 > 0) return -1; // no zero crossing

    // linear interpolation to find zero crossing
    double slope = (y2 - y1) / (x2 - x1);
    double zeroCrossingX = x1 - y1 / slope;

    return zeroCrossingX;

  }

  return TMath::QuietNaN();

};
