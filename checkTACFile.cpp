#include "BinaryReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TStyle.h"
#include "TCanvas.h"

#include <queue>
#include "class_DIG.h"
#include "misc.h"


BinaryReader haha(10); // 100 is the deafult hit size

TH1F * htacTimeDiff = new TH1F("htacTimeDiff", "TAC Time Difference; Time Difference [us]; Counts", 2000, 0, 2000);
TH1F * htrigTimetdcTimeDiff = new TH1F("htrigTimetdcTimeDiff", "TDC Time - trig Time Difference; Time Difference [ns]; Counts", 2000, 0, 200);
TH1F * htrigTimetacTimeDiff = new TH1F("htrigTacTimeDiff", "TAC Time - trig Time Difference; Time Difference [ns]; Counts", 2000, -300, 0);

void checkTACFile(){

  // haha.Open("data/TAC2_022/TAC2_022_000_0099_T", true);
  // haha.Open("data/TAC2_202/TAC2_202_000_0099_T", true);
  haha.Open("data_slopebox/testD_005/testD_005_000_0099_T", true);
  printf("           File Size: %.3f Mbytes\n", haha.GetFileSize()/1024./1024.);

  int totalNumHits =  haha.GetFileSize() / 40; // each TAC hit is 40 bytes

  uint64_t globalEarliestTime = 0;
  uint64_t globalLastTime = 0;
  uint64_t oldTime = 0;

  int trashDataCount[3] = {0};
  int timeAnomalyCount[3] = {0};

  std::vector<TDC> tacHits[3];

  int eventID = 0;
  int goodEventCount = 0;
  do{
    haha.ReadNextNHitsFromFile(false);
    for( int i = 0; i < haha.GetHitSize(); i++){
      HIT hit = haha.GetHit(i);
      TDC tac = hit.DecodeTAC();
      eventID++;

      if( tac.isTrashData == TACTrash::NoTrigger ) {
        trashDataCount[0]++;
        continue;
      }
      if( tac.isTrashData == TACTrash::TDCoffsetInvalid ) {
        trashDataCount[1]++;
        continue;
      }
      if( tac.isTrashData == TACTrash::VernierInvalid ) {
        trashDataCount[2]++;
        continue;
      }

      if( globalEarliestTime == 0 ){
        globalEarliestTime = tac.timestampTrig;
        oldTime = tac.timestampTrig;
      }
      
      if( tac.timestampTrig < globalEarliestTime ){
        timeAnomalyCount[0]++;
        tacHits[0].push_back(tac);
        continue;
      }
      
      if( tac.timestampTrig - oldTime > 0x5F5E100 ) {
        timeAnomalyCount[1]++;
        tacHits[1].push_back(tac);
        continue;
      }
      
      if( tac.timestampTrig < oldTime ){
        // printf("---------------------------------- Event ID: %d \n", eventID);  
        // printf("Timestamp error found: oldTime: %16lX, %20lu\n", oldTime, oldTime);
        // printf("                       newTime: %16lX, %20lu \n", tac.timestampTrig , tac.timestampTrig);
        // printf("                       diff   : %16lX, %20lu \n", tac.timestampTrig - oldTime, tac.timestampTrig - oldTime);
        timeAnomalyCount[2]++;
        tacHits[2].push_back(tac);
        // hit.Print();
        // tac.print();
      }

      goodEventCount++;
      
      if( tac.timestampTrig < globalEarliestTime ){
        globalEarliestTime = tac.timestampTrig;
      }

      if( tac.timestampTrig > globalLastTime ){
        globalLastTime = tac.timestampTrig;
      }

      uint64_t timeDiff = tac.timestampTrig - oldTime; // in ns 
      htacTimeDiff->Fill( timeDiff / 1e3 );

      int trigTdcDiff = static_cast<int>(tac.timestampTDC - tac.timestampTrig); // in ns
      htrigTimetdcTimeDiff->Fill( trigTdcDiff );

      double tacTdcDiff = static_cast<double>(tac.avgPhaseTime - tac.timestampTrig); // in ns
      htrigTimetacTimeDiff->Fill( tacTdcDiff );

      oldTime = tac.timestampTrig;

      if( eventID % 1000000 == 0 ){
        printf("Processed %d events. Current TAC Time: %lu ns \n", eventID, tac.timestampTrig);
      }
    }
  }while( eventID < totalNumHits );

  printf(" Global Earliest TAC Time: %12lu ns\n", globalEarliestTime);
  printf("     Global Last TAC Time: %12lu ns\n", globalLastTime);
  printf("            TAC time span: %.3f sec\n", (globalLastTime - globalEarliestTime) /1e9);
  printf("   Total Events Processed: %8d \n", eventID);
  printf("Good TAC Events Processed: %8d \n", goodEventCount);
  printf("------------ Trash Data Counts: \n");
  printf("          No Trigger: %d \n", trashDataCount[0]);
  printf("  TDC Offset Invalid: %d \n", trashDataCount[1]);
  printf("     Vernier Invalid: %d \n", trashDataCount[2]);
  printf("------------ Time Anomaly Counts: \n");
  printf("  TAC time < globalEarliestTime: %d \n", timeAnomalyCount[0]);
  printf("     TAC time - oldTime > 1 sec: %d \n", timeAnomalyCount[1]);
  printf("             TAC time < oldTime: %d \n", timeAnomalyCount[2]);

  gStyle->SetOptStat(111111);

  TCanvas * c1 = new TCanvas("c1", "TAC Analysis", 2400, 800);
  c1->Divide(3,1);
  c1->cd(1); htacTimeDiff->Draw();
  c1->cd(2); htrigTimetdcTimeDiff->Draw();
  c1->cd(3); htrigTimetacTimeDiff->Draw();

}

