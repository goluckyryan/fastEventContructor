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

void analyzer_tac(TString rootFileName){

  LoadChannelMapFromFile();

  TFile * inFile = TFile::Open(rootFileName, "READ");
  TTree * tree = (TTree*) inFile->Get("tree");

  TTreeReader reader(tree);
  TTreeReaderValue<UInt_t>             nHits(reader, "NumHits");
  // TTreeReaderArray<UShort_t>              id(reader, "id");
  TTreeReaderArray<UShort_t>           detID(reader, "detID");
  TTreeReaderArray<Double_t>          energy(reader, "energy");
  TTreeReaderArray<ULong64_t>        eventTS(reader, "eventTS");
  TTreeReaderArray<ULong64_t>         trigTS(reader, "trigTS");
  TTreeReaderArray<Short_t>     CFD_sample_0(reader, "CFD_sample_0");
  TTreeReaderArray<Short_t>     CFD_sample_1(reader, "CFD_sample_1");
  TTreeReaderArray<Short_t>     CFD_sample_2(reader, "CFD_sample_2");

  const int nEntries = tree->GetEntries();
  printf("Total number of entries: %d\n", nEntries);

  
  TH1F * htacDiff = new TH1F("htacDiff", "TAC trigTime Difference - 62 trigTime; Time Difference [10 ns]; Counts", 1000, -1000, 1000);

  TH2F * htacDiffID = new TH2F("htacDiffID", "TAC trigTime Difference vs DetID; DetID; Time Difference [10 ns]", 110, 0, 110, 1000, -1000, 1000);
  
  TH1F * hTrigTime = new TH1F("hTrigTime", "Trigger Time Distribution; Trigger Time [10 ns]; Counts", 1000, 0, 1000);
  TH1F * hTrigTime2 = new TH1F("hTrigTime2", "Trigger Time Distribution; Trigger Time [10 ns]; Counts", 1000, 0, 100000);

  TH2F * heventTStrigTSDiff = new TH2F("heventTStrigTSdiff", "eventTS - trigTS vs DetID; DetID; Time Difference [10 ns]", 111, 0, 111, 2000, -2500, 0);
  TH1F * heventTStrigTSDiff062 = new TH1F("heventTStrigTSdiff_proj", "eventTS - trigTS (GS062); Time Difference [10 ns]; Counts", 2000, -2500, 0);

  TH1F * hTACMultiplicity = new TH1F("hTACMultiplicity", "TAC Multiplicity; TAC Multiplicity; Counts", 10, 0, 10);
  TH1F * hTACMultiplicity2 = new TH1F("hTACMultiplicity2", "TAC Multiplicity w/ GS; TAC Multiplicity; Counts", 10, 0, 10);

  TH1F * hTACMultiVsTimeDiff[7];
  for( int i = 0; i < 7; i++ ){
    hTACMultiVsTimeDiff[i] = new TH1F( Form("hTACMultiVsTimeDiff_%d", i+1), Form("TAC Time Diff to pervious for TAC Multiplicity %d; TAC Time Diff to pervious [10 ns]; Counts", i+1), 500, 0, 1000);
    hTACMultiVsTimeDiff[i]->SetLineColor(i+1);
  }

  TH1F * heventTimeSpan = new TH1F("heventTimeSpan", "Event Time Span per event; Event Time Span [10 ns]; Counts", 3000, 0, 3000);

  //^================================== Analysis Loop ==================================^//
  ULong64_t lastFirstTrigTime = 0;

  int withTACcount = 0; // hasGS && hasTAC
  int noTACcount = 0;   // hasGS && !hasTAC
  int onlyTACcount = 0;  // !hasGS && hasTAC
  int noTACandGScount = 0; // !hasGS && !hasTAC

  int displayLimit = 100;
  int displayCount = 0;

  for( int entry = 0; entry < nEntries ; entry++ ){
    reader.Next();  
    
    
    ULong64_t tacTrigTime = 0;
    bool hasTAC = false;
    for( unsigned int hit = 0; hit < *nHits; hit++ ){
      if( detID[hit] == 999 ){ // TAC channel
        hasTAC = true;
        tacTrigTime = trigTS[hit];
        break;
      }
    }
    
    int TACMultiplicity = 0;
    std::vector<ULong64_t> timeDiffToFirstTAC;
    for( unsigned int hit = 0; hit < *nHits; hit++ ){
      if(detID[hit] == 999 ) {
        TACMultiplicity++;
        if( tacTrigTime != 0 ){
          timeDiffToFirstTAC.push_back(trigTS[hit]);
        }
      }
    }
    hTACMultiplicity->Fill( TACMultiplicity );
    if( TACMultiplicity < 7 ) {
      for( unsigned int i = 1; i < timeDiffToFirstTAC.size(); i++ ){
        int diff = (int)(timeDiffToFirstTAC[i] - timeDiffToFirstTAC[i-1]);
        hTACMultiVsTimeDiff[TACMultiplicity]->Fill( diff);
      }
    }
    
    ULong64_t currentFirstTrigTime = 0;
    for( unsigned int hit = 0; hit < *nHits; hit++ ){
      if(detID[hit] != 999 && detID[hit] != 0){
        currentFirstTrigTime = trigTS[hit];
        break;
      }
    }
    
    if( lastFirstTrigTime > 0 && currentFirstTrigTime > 0 ){
      hTrigTime->Fill( (long)(currentFirstTrigTime - lastFirstTrigTime) );
      hTrigTime2->Fill( (long)(currentFirstTrigTime - lastFirstTrigTime) );
      // if( (long)(currentFirstTrigTime - lastFirstTrigTime) < 1000 ) {
      //   printf("Entry %5d : First trig time diff = %12llu\n", entry, (currentFirstTrigTime - lastFirstTrigTime) );
      //   printf("      %5s      current trig time = %12llu\n", "", currentFirstTrigTime );
      //   printf("      %5s         last trig time = %12llu\n", "", lastFirstTrigTime );
      // }
    }
    lastFirstTrigTime = currentFirstTrigTime;
    
    bool hasGS = false;

    // bool displayCondition = ( TACMultiplicity == 3 ) && ( displayCount < displayLimit );
    bool displayCondition = ( displayCount < displayLimit );
      
    if( displayCondition ) {
      printf("================================= %5d\n", entry);

      for( unsigned int i = 0; i < timeDiffToFirstTAC.size(); i++ ){
        printf("    TAC Hit %2d : trigTime = %12llu ", i, timeDiffToFirstTAC[i] );
        if( i > 0 ){
          printf("| %12llu\n", timeDiffToFirstTAC[i] - timeDiffToFirstTAC[i-1] );
        }else{
          printf("\n");
        }
      }
    }

    ULong64_t firstGSeventTS = 0;;
    ULong64_t lastGSeventTS = 0;

    for( unsigned int hit = 0; hit < *nHits; hit++ ){

      if( displayCondition ){
        Long64_t timeDiff = 0;
        if( eventTS[hit] < trigTS[hit] ) {
          timeDiff = trigTS[hit] - eventTS[hit];
        }else{
          timeDiff = eventTS[hit] - trigTS[hit];
          timeDiff = -timeDiff;
        } 
        printf("%5d | %5d | %12llu | %12llu | %12lld\n", hit, detID[hit], eventTS[hit], trigTS[hit], timeDiff ); 
        displayCount++;
      }

      if( detID[hit] == 0 ) continue; // BGO

      if( !(detID[hit] >= 1 && detID[hit] <= 110) ) continue; // not GS 
      
      hasGS = true;
      if( firstGSeventTS == 0 ){
        firstGSeventTS = eventTS[hit];
      }
      lastGSeventTS = eventTS[hit];
      
      if( detID[hit] == 62 && tacTrigTime != 0 ){
        long timeDiff = (long)(tacTrigTime - trigTS[hit]); // in ns
        htacDiff->Fill( timeDiff );
      }


      if( tacTrigTime != 0 ){
        long timeDiff = (long)(tacTrigTime - trigTS[hit]); // in ns
        htacDiffID->Fill( detID[hit], timeDiff );
      }

      if( 0 <= detID[hit] && detID[hit] <= 110 ){
        heventTStrigTSDiff->Fill( detID[hit], (long)(eventTS[hit] - trigTS[hit]) );
      }

      if( detID[hit] == 62 ){
        heventTStrigTSDiff062->Fill( (long)(eventTS[hit] - trigTS[hit]) );
      }

    }

    if( hasGS && hasTAC ){
      withTACcount++;

      hTACMultiplicity2->Fill( TACMultiplicity );

    }else if( hasGS && !hasTAC ){
      noTACcount++;
    }else if( !hasGS && hasTAC ){
      onlyTACcount++;
    }else{
      noTACandGScount++;
    }

    if( firstGSeventTS != 0 && lastGSeventTS != 0 ){
      if( displayCondition ) {
        printf("    First GS eventTS : %12llu\n", firstGSeventTS );
        printf("    Last  GS eventTS : %12llu\n", lastGSeventTS );
        printf("    Event Time Span  : %12llu\n", (lastGSeventTS - firstGSeventTS) );
      }
      heventTimeSpan->Fill( (long)(lastGSeventTS - firstGSeventTS) );
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

  printf("Analysis Summary:\n");
  printf("  Events with GS and TAC: %d\n", withTACcount);
  printf("  Events with GS but no TAC: %d\n", noTACcount);
  printf("  Events with TAC but no GS: %d\n", onlyTACcount);
  printf("  Events with no GS and no TAC: %d\n", noTACandGScount);  

  int totalCount = withTACcount + noTACcount + onlyTACcount + noTACandGScount;
  printf("  Total Events Processed: %d\n", totalCount);
  printf("  Number of Entries : %d\n", nEntries );

  //^================================== Draw Histograms ==================================^//
  gStyle->SetOptStat(111111);
  
  TCanvas * canvas = new TCanvas("canvas", "Semi-Online analysis", 2000, 1500);
  canvas->Divide(2,2);

  // canvas->cd(1); htacDiff->Draw();
  // canvas->cd(2); htacDiffID->Draw("colz");
  // canvas->cd(3); hTrigTime->Draw();

  canvas->cd(1); 
  // for( int i = 2; i < 7; i++ ){
  //   hTACMultiVsTimeDiff[i]->Draw("same");
  // }
  heventTimeSpan->Draw();

  canvas->cd(2); 
  hTACMultiplicity->Draw();
  hTACMultiplicity2->SetLineColor(kRed);
  hTACMultiplicity2->Draw("same");
  
  canvas->cd(3); heventTStrigTSDiff062->Draw();
  canvas->cd(4); heventTStrigTSDiff->Draw("colz");



}

