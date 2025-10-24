#include "BinaryReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"

#include <queue>
#include "class_DIG.h"
#include "misc.h"

#include "readTrace_S.C"

BinaryReader haha(1); // 100 is the deafult hit size

struct CompareEvent {
  bool operator()(const DIG& a, const DIG& b) {
    return a.EVENT_TIMESTAMP > b.EVENT_TIMESTAMP;
  }
};

void test(){

  // LoadChannelMapFromFile();
  // return;

  // readRawTrace("test.root");

  // return;

  haha.Open("data/TAC2_019/TAC2_019_000_0135_5", true);

  haha.ReadNextNHitsFromFile(false);
  for( int i = 0; i < haha.GetHitSize(); i++){
  // for( int i = 0; i < 10; i++){
    HIT hit = haha.GetHit(i);
    DIG dig = hit.DecodePayload();
    printf("%4d | %lX \n", i, haha.GetHit(i).gebHeader.timestamp);
    hit.Print();
    dig.Print();
  }

  BinaryReader haha2(1);
  haha2.Open("data/TAC2_019/TAC2_019_000_0099_T", true);

  haha2.ReadNextNHitsFromFile(false);
  for( int i = 0; i < haha2.GetHitSize(); i++){
  // for( int i = 0; i < 10; i++){
    HIT hit = haha2.GetHit(i);
    TDC tac = hit.DecodeTAC();
    printf("%4d | %lX \n", i, haha2.GetHit(i).gebHeader.timestamp);
    hit.Print();
    tac.print();
  }

  return;


  // haha.Open("data/dgs_run116/dgs_run116.gtd10_000_0136_2", true);
  haha.Open("data/haha_016/haha_016_000_0099_T", true);
  
  // haha.ReadNextNHitsFromFile(false);
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