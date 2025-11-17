#include "BinaryReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"

#include <queue>
#include "class_DIG.h"
#include "misc.h"

#include "readTrace_S.C"

BinaryReader haha(10); // 100 is the deafult hit size

TH1F * htacTimeDiff = new TH1F("htacTimeDiff", "TAC Time Difference; Time Difference [ns]; Counts", 2000, 0, 2000);
TH1F * htrigTimetdcTimeDiff = new TH1F("htrigTimetdcTimeDiff", "TDC Time - trig Time Difference; Time Difference [ns]; Counts", 2000, 0, 200);
TH1F * htrigTimetacTimeDiff = new TH1F("htrigTacTimeDiff", "TAC Time - trig Time Difference; Time Difference [ns]; Counts", 2000, -300, 0);

void checkTACFile(){

  // haha.Open("data/TAC2_022/TAC2_022_000_0099_T", true);
  haha.Open("data/TAC2_202/TAC2_202_000_0099_T", true);
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

      uint64_t timeDiff = tac.timestampTrig - oldTime;
      htacTimeDiff->Fill( timeDiff );

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

  printf("Global Earliest TAC Time: %lu ns\n", globalEarliestTime);
  printf("    Global Last TAC Time: %lu ns\n", globalLastTime);
  printf("           TAC time span: %.3f sec\n", (globalLastTime - globalEarliestTime) /1e9);
  printf("Total Events Processed: %d \n", eventID);
  printf("Good TAC Events Processed: %d \n", goodEventCount);
  printf("Trash Data Counts: \n");
  printf("  No Trigger: %d \n", trashDataCount[0]);
  printf("  TDC Offset Invalid: %d \n", trashDataCount[1]);
  printf("  Vernier Invalid: %d \n", trashDataCount[2]);
  printf("Time Anomaly Counts: \n");
  printf("  TAC time < globalEarliestTime: %d \n", timeAnomalyCount[0]);
  printf("  TAC time - oldTime > 1 sec: %d \n", timeAnomalyCount[1]);
  printf("  TAC time < oldTime: %d \n", timeAnomalyCount[2]);


  TCanvas * c1 = new TCanvas("c1", "TAC Analysis", 2400, 800);
  c1->Divide(3,1);
  c1->cd(1); htacTimeDiff->Draw();
  c1->cd(2); htrigTimetdcTimeDiff->Draw();
  c1->cd(3); htrigTimetacTimeDiff->Draw();

}


void test(){

  checkTACFile();
  return;


  haha.Open("data/TAC2_022/TAC2_022_000_0121_7", true);
  haha.ReadNextNHitsFromFile(false);
  for( int i = 0; i < haha.GetHitSize(); i++){
    HIT hit = haha.GetHit(i);
    DIG dig = hit.DecodePayload();
    dig.Print();
  }


  // LoadChannelMapFromFile();
  // return;

  

  // haha.Open("data/dgs_run116/dgs_run116.gtd10_000_0136_2", true);
  // haha.Open("data/haha_016/haha_016_000_0099_T", true);
  // haha.Open("data/TAC2_022/TAC2_022_000_0099_T", true);
  haha.Open("data_slopebox/testC_001_000_0099_T", true);
  haha.ReadNextNHitsFromFile(false);
  for( int i = 0; i < haha.GetHitSize(); i++){
    HIT hit = haha.GetHit(i);
    TDC tac = hit.DecodeTAC();
    tac.print();
  }

  return;

  // // haha.ReadNextNHitsFromFile(false);
  // // haha.ReadNextNHitsFromFile(false);
  // HIT hit = haha.GetHit(0);
  // DIG dig = hit.DecodePayload(true);
  // dig.Print();
  // dig.PrintTraceData();

  // haha.GetHit(1).Print();

  // haha.Scan(true, false);
  // printf("           File Size: %.3f Mbytes\n", haha.GetFileSize()/1024./1024.);
  // printf("Global Earliest Time: %lu x 10 ns\n", haha.GetGlobalEarliestTime());
  // printf("    Global Last Time: %lu x 10 ns\n", haha.GetGlobalLastTime());
  // printf("           time span: %.3f sec\n", (haha.GetGlobalLastTime() - haha.GetGlobalEarliestTime()) * 10. /1e9);
  // printf(" Max Hit Size Needed: %u\n", haha.GetMaxHitSize());

  haha.ReadNextNHitsFromFile(false);
  for( int i = 0; i < haha.GetHitSize(); i++){
    printf("%4d | %X %X | %lX \n", i, haha.GetHit(i).payload[3], haha.GetHit(i).payload[2], haha.GetHit(i).gebHeader.timestamp);
  }

  // printf(" hit count: %d \n", haha.GetNumData());


  // haha.ReadNextNHitsFromFile(false);
  // haha.ReadNextNHitsFromFile(false);
  // haha.ReadNextNHitsFromFile(true);
  // haha.ReadNextNHitsFromFile(true);
  // haha.ReadNextNHitsFromFile(true);


  // size_t memSize = haha.GetMemoryUsageBytes();
  // printf("Memory %zu bytes = %.3f MB\n", memSize, memSize / (1024.0 * 1024.0));  

  // printf(" number of hits: %u \n", haha.GetHitSize());


  // std::priority_queue<DIG, std::vector<DIG>, CompareEvent> hitsQueue; // Priority queue to store events


  // for( int i = 0; i < haha.GetHitSize(); i++){
  //   hitsQueue.push(haha.GetHit(i).DecodePayload());
  //   // printf(" %3d | %lu | %d | %d %d\n", i,   hit.header.timestamp, event.channel, event.pre_rise_energy, event.post_rise_energy);
  // }

  // printf(" number of hits: %u \n", haha.GetHitSize());

  // for( int i = 0; i < 1; i++){
  //   HIT hit = haha.GetHit(i);
  //   hit.Print();

  //   DIG digHit = hit.DecodePayload();

  //   digHit.Print();

  // //   // printf(" %3d | %lu | %d | %d %d\n", i,   hit.header.timestamp, event.channel, event.pre_rise_energy, event.post_rise_energy);
  // }

  


  // Event event = hit.DecodePayload();

  // event.Print();


  // GEBHeader header;

  // header = haha.Read<GEBHeader>();
  // header.Print();

  // haha.Seek( sizeof(GEBHeader) + header.payload_lenght_byte );

  // header = haha.Read<GEBHeader>();
  // header.Print();


  
  // uint32_t word;

  // for( int i = 0; i< 220 ; i++){
  //   word = haha.read<uint32_t>();
  //   printf("%3d | 0x%08X\n", i, word);
  // }

  return;


  //############# compare tree

  TFile * file[2];
  TTree * tree[2];
  unsigned long size[2];
  UInt_t numHit[2];
  ULong64_t timestamp[2][10000];  

  file[0] = new TFile("test1.root", "READ");
  file[1] = new TFile("test2.root", "READ");

  for( int i = 0; i < 2 ; i++ ){ 
    tree[i] = (TTree*)file[i]->Get("tree");
    size[i] = tree[i]->GetEntries();
    tree[i]->SetBranchAddress("NumHits", &numHit[i]);
    tree[i]->SetBranchAddress("event_timestamp", &timestamp[i]);

  }

  TH1F * h1 = new TH1F("h1", "Timestamp ", 3000, -1000, 2000);

  int ID = 1;
  for( unsigned long i = 0; i < size[ID]-1 ; i++ ){
    tree[ID]->GetEntry(i);

    ULong64_t ts1 = timestamp[ID][0];

    tree[ID]->GetEntry(i+1);

    ULong64_t ts2 = timestamp[ID][0];


    if( ts2 > ts1 ) {
      h1->Fill(ts2 - ts1);
    }else{
      h1->Fill((ts1 - ts2)*(-1.));
    }

    if( ts2 < ts1 ){
      printf("Entry %lu | %llu, %llu\n", i, ts1 , ts2);
    }

    if( ts2-ts1 < 1000 ){
      printf("Entry %lu | %llu, %llu\n", i, ts1 , ts2);
    }

  }

  h1->Draw();

  unsigned long maxEvent = 100;
  // for( unsigned long i = 0; i < size1 && i < size2 && i < maxEvent; i++){
  //   tree1->GetEntry(i);
  //   tree2->GetEntry(i);

  //   if( numHit1 != numHit2 ){
  //     printf("Mismatch at entry %lu: numHit1 = %d, numHit2 = %d\n", i, numHit1, numHit2);
  //     continue;
  //   }

  //   for( int j = 0; j < numHit1; j++){
  //     if( timestamp1[j] != timestamp2[j] ){
  //       printf("Mismatch at entry %lu, hit %d: timestamp1 = %llu, timestamp2 = %llu\n", i, j, timestamp1[j], timestamp2[j]);
  //     }
  //   }

  //   // Add more checks as needed
  // }






}