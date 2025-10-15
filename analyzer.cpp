#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TString.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"


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

void analyzer(TString rootFileName){

  TFile * inFile = TFile::Open(rootFileName, "READ");

  TTree * tree = (TTree*) inFile->Get("tree");

  TTreeReader reader(tree);
  TTreeReaderValue<UInt_t>             nHits(reader, "NumHits");
  TTreeReaderArray<UShort_t>              id(reader, "id");
  TTreeReaderArray<UInt_t>     preRiseEnergy(reader, "pre_rise_energy");
  TTreeReaderArray<UInt_t>    postRiseEnergy(reader, "post_rise_energy");
  TTreeReaderArray<ULong64_t> eventTimestamp(reader, "event_timestamp");
  TTreeReaderArray<Short_t>     CFD_sample_0(reader, "CFD_sample_0");
  TTreeReaderArray<Short_t>     CFD_sample_1(reader, "CFD_sample_1");
  TTreeReaderArray<Short_t>     CFD_sample_2(reader, "CFD_sample_2");

  const int nEntries = tree->GetEntries();
  printf("Total number of entries: %d\n", nEntries);


  TH1F * hTimeDiff = new TH1F("hTimeDiff", "Time Difference between Channel 7 and Channel 20; Time Difference (ns); Counts", 100, 2880, 2890);

  //loop over all entries
  for( int entry = 0; entry < nEntries ; entry++ ){
    reader.Next();  

    double time0 = TMath::QuietNaN();
    double timeTAC = TMath::QuietNaN();

    for( int hit = 0; hit < *nHits; hit++ ){

      short board = id[hit] / 100;
      short channel = id[hit] % 100;


      if( channel == 7 ){

        std::vector<std::pair<double, double>> points;

        points.push_back( std::make_pair(    0.0, CFD_sample_0[hit]) );
        points.push_back( std::make_pair(  -10.0, CFD_sample_1[hit]) );
        points.push_back( std::make_pair(  -20.0, CFD_sample_2[hit]) );

        double zeroCross = ZeroCrossing(points);

        if(TMath::IsNaN(zeroCross)) continue;

        double offset = 0;

        time0 = eventTimestamp[hit] * 10  + zeroCross ; // in ns

        // printf("Entry: %d, Hit: %d, ID: %d, PreRiseEnergy: %u, PostRiseEnergy: %u, Timestamp: %llu, ZeroCrossing: %.2f ns\n", 
        //         entry, hit, id[hit], preRiseEnergy[hit], postRiseEnergy[hit], eventTimestamp[hit], zeroCross);

      }

      if( channel == 20 ){
        timeTAC = eventTimestamp[hit] * 10  + preRiseEnergy[hit] / 1000; // in ns
      }
      
    }

    hTimeDiff->Fill(time0 - timeTAC);

  }

  gStyle->SetOptStat(111111);

  hTimeDiff->Draw();

}
