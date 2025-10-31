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
#include "analyzer.h" // for ZeroCrossing function and MWIN constant

int eRange[2] = {100, 6000};
int timeRange[2] = {0, 600};

TH1F * he[110];
TH2F * heID = new TH2F("heID", "Energy vs Detector ID; Detector ID; Energy [a.u.]", 110, 0, 110, 200, eRange[0], eRange[1]);

TH2F * heIDEven = new TH2F("heIDEven", "Energy vs Detector ID South; Detector ID; Energy [a.u.]", 55, 0, 110, 200, eRange[0], eRange[1]);
TH2F * heIDOdd  = new TH2F("heIDOdd",  "Energy vs Detector ID North; Detector ID; Energy [a.u.]", 55, 0, 110, 200, eRange[0], eRange[1]);

TH1F * hTDiff[110];
TH1F * hMultiHits = new TH1F("hMultiHits", "Gamma Multiplicity; Number of Hits; Counts", 10, 0, 10);
TH2F * hIDvID = new TH2F("hIDvID", "Detector (2 multiplicity); Detector ID; Detector ID", 110, 0, 110, 110, 0, 110);
TH2F * hEE = new TH2F("hEE", "Energy-Energy (2 multiplicity); Energy 1 (a.u.); Energy 2 (a.u.)", 400, eRange[0], eRange[1], 400, eRange[0], eRange[1]);

//coincident bewteen det X and Y
int detX = 70;
int detY = 62;

TH2F * hGG = new TH2F("hGG", Form("Energy-Energy Det %d vs Det %d; Energy Det %d (a.u.); Energy Det %d (a.u.)", detX, detY, detX, detY), 100, eRange[0], eRange[1], 100, eRange[0], eRange[1]);
TH1F * hGTimeDiff = new TH1F("hGTimeDiff", Form("Time Difference Det %d - Det %d; Time Difference [ns]; Counts", detX, detY), 500, -100, 100);
std::vector<float> energyDetX, energyDetY;
std::vector<double> timeDetX, timeDetY;
std::vector<float> offsetX, offsetY;

TH1F * hTT1 = new TH1F("hTT1", "timestamp diff from 1st hit of events; tick", 400, 0, 1600);
TH1F * hTT2 = new TH1F("hT21", "TrigTS diff from 1st hit of events; tick", 400, 0, 1600);

TH2F * hTDiffHitOrder = new TH2F("hTDiffHitOrder", "62 ; time diff [ns]; order", 1000,  timeRange[0], timeRange[1], 10, 0, 10 );
TH2F * hTDiffEnergy = new TH2F("hTDiffEnergy", "62", 1000,  timeRange[0], timeRange[1], 200, eRange[0], eRange[1] );

void analyzer(TString rootFileName){

  LoadChannelMapFromFile();

  TFile * inFile = TFile::Open(rootFileName, "READ");
  TTree * tree = (TTree*) inFile->Get("tree");

  TTreeReader reader(tree);
  TTreeReaderValue<UInt_t>             nHits(reader, "NumHits");
  // TTreeReaderArray<UShort_t>              id(reader, "id");
  TTreeReaderArray<UShort_t>           detID(reader, "detID");
  TTreeReaderArray<long long>          energy(reader, "energy");
  TTreeReaderArray<ULong64_t>        eventTS(reader, "eventTS");
  TTreeReaderArray<ULong64_t>         trigTS(reader, "trigTS");
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
    hTDiff[i] = new TH1F(histNameTDiff, histTitleTDiff, 1000, timeRange[0], timeRange[1]);
  }

  //^================================== Analysis Loop ==================================^//
  //loop over all entries

  double time0[110];
  int gsCount[110];

  uint64_t timestamp0 = 0;
  uint64_t trigTS0 = 0;
 
  unsigned int noTACEventCount = 0;
  
  for( int entry = 0; entry < nEntries ; entry++ ){
    reader.Next(); 
    
    for ( int i = 0; i < 110; i++ ){
      time0[i] = TMath::QuietNaN();
      gsCount[i]=-1;
    }
    double timeTAC = TMath::QuietNaN();
    energyDetX.clear();
    energyDetY.clear();
    timeDetX.clear();
    timeDetY.clear();
    offsetX.clear();
    offsetY.clear();
    double energy62 = TMath::QuietNaN();
    
    // hMultiHits->Fill(*nHits);
    unsigned short gammaHit = 0;
    std::vector<std::pair<double, double>> points;
    double timestamp;
    for( unsigned int hit = 0; hit < *nHits; hit++ ){
      
      if( detID[hit] == 999 &&  TMath::IsNaN(timeTAC)){ // TAC channel
        timestamp = static_cast<double>(eventTS[hit]) * 10.0; // in ns
        timeTAC = timestamp  + energy[hit] / 1000.; // in ns
      }

    }

    for( unsigned int hit = 0; hit < *nHits; hit++ ){

      if( hit == 0){
        if( timestamp0 == 0) {
          timestamp0 = eventTS[hit];
        }else{
          hTT1->Fill( eventTS[hit] - timestamp0 );
          timestamp0 = eventTS[hit];
        }
        if( trigTS0 == 0) {
          trigTS0 = trigTS[hit];
        }else{
          hTT2->Fill( trigTS[hit] - trigTS0);
          trigTS0 = trigTS[hit];
        }
      }

      // short board = id[hit] / 100;
      // short channel = id[hit] % 100;

      timestamp = static_cast<double>(eventTS[hit]) * 10.0; // in ns

      if( detID[hit] == 999 ){ // TAC channel
        continue;
      }else{

        if (detID[hit] == 0 ) continue;

        double eee = energy[hit] / MWIN;

        if( detID[hit] == 62) energy62 = eee;

        // printf("Entry: %d, Hit: %d, detID: %d, PreRiseEnergy: %u, PostRiseEnergy: %u, Energy: %.2f, Timestamp: %llu\n", 
        //         entry, hit, detID[hit], energy[hit], postRiseEnergy[hit], energy, eventTS[hit]);

        he[detID[hit]-1]->Fill(eee);
        heID->Fill( detID[hit], eee );
        if( detID[hit] % 2 == 0 ){
          heIDEven->Fill( detID[hit], eee );
        }else{
          heIDOdd->Fill( detID[hit], eee );
        }

        if( eRange[0] < eee && eee < eRange[1] ) {
          gsCount[detID[hit]-1] = gammaHit;
          gammaHit ++;
        }


        points.clear();
        points.push_back( std::make_pair(    0.0, CFD_sample_0[hit]) );
        points.push_back( std::make_pair(  -10.0, CFD_sample_1[hit]) );
        points.push_back( std::make_pair(  -20.0, CFD_sample_2[hit]) );
        
        double zeroCross = ZeroCrossing(points);
        
        if(!TMath::IsNaN(zeroCross)) {        
          time0[detID[hit]-1] = timestamp  + zeroCross ; // in ns
        }

        if ( detID[hit] == detX && eRange[0] < eee && eee < eRange[1] ){
          energyDetX.push_back( eee );
          // timeDetX.push_back( timestamp + zeroCross );
          timeDetX.push_back(zeroCross );
          offsetX.push_back( zeroCross );
          // printf("Det %d: Energy: %.2f | Timestamp: %.2f | Offset: %.2f | %.2f\n", detX, energy, timestamp, offsetX.back(), timeDetX.back());
        }
        if ( detID[hit] == detY && eRange[0] < eee && eee < eRange[1] ){ 
          energyDetY.push_back( eee );
          // timeDetY.push_back( timestamp + zeroCross );
          timeDetY.push_back( zeroCross );
          offsetY.push_back( zeroCross );
        }
      
        // printf("Entry: %d, Hit: %d, detID: %d, PreRiseEnergy: %u, PostRiseEnergy: %u, Timestamp: %llu, ZeroCrossing: %.2f ns | time : %.2f\n", 
        //         entry, hit, detID[hit], eee, postRiseEnergy[hit], eventTS[hit], zeroCross, time0[detID[hit]-1] );

      }

      if( TMath::IsNaN(timeTAC) ) noTACEventCount ++;

    }

    hMultiHits->Fill(gammaHit);

    if(gammaHit == 2 ){
      for ( unsigned int hit1 = 0; hit1 < *nHits; hit1++ ){
        if( detID[hit1] == 0 || detID[hit1] == 999 ) continue;
        
        float energy1 = energy[hit1]/ MWIN;
        
        for ( unsigned int hit2 = hit1 + 1; hit2 < *nHits; hit2++ ){
          if( detID[hit2] == 0 || detID[hit2] == 999 ) continue;

          float energy2 = energy[hit2]/ MWIN;

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

    if( gammaHit == 3){
      for ( int i = 0; i < 110; i++ ){
        if( TMath::IsNaN(time0[i]) || TMath::IsNaN(timeTAC) ) continue;
        hTDiff[i]->Fill( timeTAC - time0[i]);

        if( i == 61){
          hTDiffHitOrder->Fill(timeTAC - time0[i], gsCount[i]);     
          hTDiffEnergy->Fill(timeTAC - time0[i], energy62);     
        }

      }
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

    //========================= Progress bar
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


  }
  printf("\n");

  //^================================== Draw Histograms ==================================^//
  gStyle->SetOptStat(111111);
  
  TCanvas * canvas = new TCanvas("canvas", "Semi-Online analysis", 2000, 1500);
  canvas->Divide(4,3);

  // canvas->cd(1); hTT1->Draw();
  // canvas->cd(2); canvas->cd(2)->SetLogy(); hTT2->Draw(); 

  canvas->cd(1); heIDOdd->Draw("colz");
  canvas->cd(2); heIDEven->Draw("colz");
  canvas->cd(3); hTDiff[61]->Draw("colz");


  canvas->cd(4); hMultiHits->Draw();
  // canvas->cd(2); heID->Draw("colz");
  // canvas->cd(2); hIDvID->Draw("colz");
  // // canvas->cd(4); hEE->Draw("colz");

  canvas->cd(5); hGTimeDiff->Draw();
  canvas->cd(6); hGG->Draw("colz");
  canvas->cd(7); he[60]->Draw();
  canvas->cd(8); hTDiffHitOrder->Draw("colz");

  canvas->cd(9); he[detX-1]->Draw();
  canvas->cd(10); he[detY-1]->Draw();
  canvas->cd(11); hTDiff[60]->Draw();
  canvas->cd(12); hTDiffEnergy->Draw("colz");

  printf("=================================\n");
  printf("      PlotE() to see Energy spectra\n");
  printf("  PlotTdiff() to see Time Difference spectra\n");
  printf("  newCanvas() to create a new canvas\n");
  printf("=================================\n");

  printf(" no TAC event count : %u\n", noTACEventCount);
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

