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

const float MWIN = 350.;

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

int eRange[2] = {1000, 2200};

TH1F * he[110];
TH1F * hTDiff[110];
TH1F * hMultiHits = new TH1F("hMultiHits", "Gamma Multiplicity; Number of Hits; Counts", 10, 0, 10);
TH2F * hIDvID = new TH2F("hIDvID", "Detector (2 multiplicity); Detector ID; Detector ID", 110, 0, 110, 110, 0, 110);
TH2F * hEE = new TH2F("hEE", "Energy-Energy (2 multiplicity); Energy 1 (a.u.); Energy 2 (a.u.)", 400, eRange[0], eRange[1], 400, eRange[0], eRange[1]);

//coincident bewteen det X and Y
int detX = 42;
int detY = 48;

TH2F * hGG = new TH2F("hGG", Form("Energy-Energy Det %d vs Det %d; Energy Det %d (a.u.); Energy Det %d (a.u.)", detX, detY, detX, detY), 100, eRange[0], eRange[1], 100, eRange[0], eRange[1]);
TH1F * hGTimeDiff = new TH1F("hGTimeDiff", Form("Time Difference Det %d - Det %d; Time Difference [ns]; Counts", detX, detY), 100, -250, 250);
std::vector<float> energyDetX, energyDetY;
std::vector<double> timeDetX, timeDetY;
std::vector<float> offsetX, offsetY;

void analyzer(TString rootFileName){

  LoadChannelMapFromFile();

  TFile * inFile = TFile::Open(rootFileName, "READ");
  TTree * tree = (TTree*) inFile->Get("tree");

  TTreeReader reader(tree);
  TTreeReaderValue<UInt_t>             nHits(reader, "NumHits");
  // TTreeReaderArray<UShort_t>              id(reader, "id");
  TTreeReaderArray<UShort_t>           detID(reader, "detID");
  TTreeReaderArray<UInt_t>     preRiseEnergy(reader, "pre_rise_energy");
  TTreeReaderArray<UInt_t>    postRiseEnergy(reader, "post_rise_energy");
  TTreeReaderArray<ULong64_t> eventTimestamp(reader, "event_timestamp");
  TTreeReaderArray<Short_t>     CFD_sample_0(reader, "CFD_sample_0");
  TTreeReaderArray<Short_t>     CFD_sample_1(reader, "CFD_sample_1");
  TTreeReaderArray<Short_t>     CFD_sample_2(reader, "CFD_sample_2");

  const int nEntries = tree->GetEntries();
  printf("Total number of entries: %d\n", nEntries);

  //^================================== Historgrams ==================================^//
  for( int i = 0; i < 110; i++ ){
    TString histName = Form("he%03d", i+1);
    TString histTitle = Form("Energy Spectrum for Detector %d; Energy [a.u.]; Counts", i+1);
    he[i] = new TH1F(histName, histTitle, 400, eRange[0], eRange[1]);

    TString histNameTDiff = Form("hTDiff%03d", i+1);
    TString histTitleTDiff = Form("Time Difference Spectrum for Detector %d; Time Difference [ns]; Counts", i+1);
    hTDiff[i] = new TH1F(histNameTDiff, histTitleTDiff, 200, -50, 50);
  }

  //^================================== Analysis Loop ==================================^//
  //loop over all entries

  double time0[110];
  
  for( int entry = 0; entry < nEntries ; entry++ ){
    reader.Next();  
    
    for ( int i = 0; i < 110; i++ ){
      time0[i] = TMath::QuietNaN();
    }
    double timeTAC = TMath::QuietNaN();
    energyDetX.clear();
    energyDetY.clear();
    timeDetX.clear();
    timeDetY.clear();
    offsetX.clear();
    offsetY.clear();

    // hMultiHits->Fill(*nHits);
    unsigned short gammaHit = 0;
    std::vector<std::pair<double, double>> points;

    double timestamp;

    for( int hit = 0; hit < *nHits; hit++ ){

      // short board = id[hit] / 100;
      // short channel = id[hit] % 100;

      timestamp = static_cast<double>(eventTimestamp[hit]) * 10.0; // in ns

      if( detID[hit] == 999 ){ // TAC channel
        timeTAC = timestamp  + preRiseEnergy[hit] / 1000; // in ns
      }else{

        if (detID[hit] == 0 ) continue;

        float energy = (postRiseEnergy[hit] - preRiseEnergy[hit] )/ MWIN;

        // printf("Entry: %d, Hit: %d, detID: %d, PreRiseEnergy: %u, PostRiseEnergy: %u, Energy: %.2f, Timestamp: %llu\n", 
        //         entry, hit, detID[hit], preRiseEnergy[hit], postRiseEnergy[hit], energy, eventTimestamp[hit]);

        he[detID[hit]-1]->Fill(energy);
        if( eRange[0] < energy && energy < eRange[1] ) gammaHit ++;

        points.clear();
        points.push_back( std::make_pair(    0.0, CFD_sample_0[hit]) );
        points.push_back( std::make_pair(  -10.0, CFD_sample_1[hit]) );
        points.push_back( std::make_pair(  -20.0, CFD_sample_2[hit]) );
        
        double zeroCross = ZeroCrossing(points);
        
        if(!TMath::IsNaN(zeroCross)) {        
          time0[detID[hit]-1] = timestamp  + zeroCross ; // in ns
        }

        if ( detID[hit] == detX && eRange[0] < energy && energy < eRange[1] ){
          energyDetX.push_back( energy );
          timeDetX.push_back( timestamp + zeroCross );
          offsetX.push_back( zeroCross );
          // printf("Det %d: Energy: %.2f | Timestamp: %.2f | Offset: %.2f | %.2f\n", detX, energy, timestamp, offsetX.back(), timeDetX.back());
        }
        if ( detID[hit] == detY && eRange[0] < energy && energy < eRange[1] ){ 
          energyDetY.push_back( energy );
          timeDetY.push_back( timestamp + zeroCross );
          offsetY.push_back( zeroCross );
        }
      
        // printf("Entry: %d, Hit: %d, detID: %d, PreRiseEnergy: %u, PostRiseEnergy: %u, Timestamp: %llu, ZeroCrossing: %.2f ns | time : %.2f\n", 
        //         entry, hit, detID[hit], preRiseEnergy[hit], postRiseEnergy[hit], eventTimestamp[hit], zeroCross, time0[detID[hit]-1] );

      }
    }

    hMultiHits->Fill(gammaHit);

    if(gammaHit == 2 ){
      for ( int hit1 = 0; hit1 < *nHits; hit1++ ){
        if( detID[hit1] == 0 || detID[hit1] == 999 ) continue;
        
        float energy1 = (postRiseEnergy[hit1] - preRiseEnergy[hit1] )/ MWIN;
        
        for ( int hit2 = hit1 + 1; hit2 < *nHits; hit2++ ){
          if( detID[hit2] == 0 || detID[hit2] == 999 ) continue;

          float energy2 = (postRiseEnergy[hit2] - preRiseEnergy[hit2] )/ MWIN;

          if( detID[hit1] <= detID[hit2] ){
            hIDvID->Fill( detID[hit1], detID[hit2] );
            hEE->Fill( energy1, energy2 );
          }else{
            hIDvID->Fill( detID[hit2], detID[hit1] );
            hEE->Fill( energy2, energy1 );
          } 

        }
      }
    }

    if (entry % (nEntries / 100 == 0 ? 1 : nEntries / 100) == 0) {
      int percent = (100 * entry) / nEntries;
      printf("\rProgress: [");
      int barWidth = 50;
      int pos = barWidth * percent / 100;
      for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("#");
        else if (i == pos) printf(">");
        else printf(" ");
      }
      printf("] %d%%", percent);
      fflush(stdout);
    }

    for ( int i = 0; i < 110; i++ ){
      if( TMath::IsNaN(time0[i]) || TMath::IsNaN(timeTAC) ) continue;
      hTDiff[i]->Fill( time0[i] - timeTAC );
    }

    //coincidence between det X and Y
    for( size_t i = 0; i < energyDetX.size(); i++ ){
      for( size_t j = 0; j < energyDetY.size(); j++ ){

        hGG->Fill( energyDetX[i], energyDetY[j] );
        float timeDiff = TMath::QuietNaN();
        if( energyDetX[i] < energyDetY[j] ) {
          timeDiff = timeDetY[i] - timeDetX[j];
        }else{
          timeDiff = timeDetX[j] - timeDetY[i];
        }
        // printf("Energy: %8.2f, %8.2f | Time: %.2f, %.2f | %.2f\n", energyDetX[i], energyDetY[j], timeDetX[i], timeDetY[j], timeDiff);
        hGTimeDiff->Fill( timeDiff );
      }
    }


  }
  printf("\n");

  //^================================== Draw Histograms ==================================^//
  gStyle->SetOptStat(111111);
  
  TCanvas * canvas = new TCanvas("canvas", "Semi-Online analysis", 1000, 1500);
  canvas->Divide(2,3);
  canvas->cd(1); hMultiHits->Draw();
  canvas->cd(2); hIDvID->Draw("colz");
  // canvas->cd(4); hEE->Draw("colz");

  canvas->cd(3); hGTimeDiff->Draw();
  canvas->cd(4); hGG->Draw("colz");

  canvas->cd(5); he[detX-1]->Draw();
  canvas->cd(6); he[detY-1]->Draw();


  printf("=================================\n");
  printf("      PlotE() to see Energy spectra\n");
  printf("  PlotTdiff() to see Time Difference spectra\n");
  printf("  newCanvas() to create a new canvas\n");
  printf("=================================\n");
}

//^===========================  Plots
void PlotE(){
  TCanvas * canvasE = new TCanvas("canvasE", "Semi-Online analysis", 1500, 1500);
  canvasE->Divide(10, 11);
  for( int i = 0; i < 110; i++ ){
    canvasE->cd(i+1);
    gPad->SetLeftMargin(0.1);
    gPad->SetRightMargin(0.01);
    gPad->SetTopMargin(0);
    gPad->SetBottomMargin(0.1);
    he[i]->Draw();
  }
}

void PlotTdiff(){
  TCanvas * canvasTDiff = new TCanvas("canvasTDiff", "Semi-Online analysis", 1500, 1500);
  canvasTDiff->Divide(10, 11);
  for( int i = 0; i < 110; i++ ){
    canvasTDiff->cd(i+1);
    gPad->SetLeftMargin(0.1);
    gPad->SetRightMargin(0.01);
    gPad->SetTopMargin(0);
    gPad->SetBottomMargin(0.1);
    hTDiff[i]->Draw();
  }
}

int nCanvas=0;
void newCanvas(int sizeX = 800, int sizeY = 600, int posX = 0, int posY = 0){
  TString name; name.Form("cNewCanvas%d", nCanvas);
  TCanvas * cNewCanvas = new TCanvas(name, name, posX, posY, sizeX, sizeY);
  nCanvas++;
  cNewCanvas->cd();
}

