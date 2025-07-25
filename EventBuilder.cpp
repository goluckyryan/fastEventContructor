/*************************************** 
 * 
 * created by Ryan Tang 2025, July 10
 * 
 * Based on the EventBuidler.cpp,
 * This event builder use priority queue to build events from multiple files.
 * 
****************************************/

#include "BinaryReader.h"

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TString.h"
#include "TMacro.h"
#include "TClonesArray.h"
#include "TGraph.h"

#include <queue>
#include <thread>
#include <vector>

#define FULL_OUTPUT false
#define MAX_MULTI 400
#define MAX_READ_HITS 100000 // Maximum hits to read at a time

#include <sys/time.h> /** struct timeval, select() */
inline unsigned int getTime_us(){
  unsigned int time_us;
  struct timeval t1;
  struct timezone tz;
  gettimeofday(&t1, &tz);
  time_us = (t1.tv_sec) * 1000 * 1000 + t1.tv_usec;
  return time_us;
}

// Comparator for min-heap (smallest timestamp on top)
struct CompareEvent {
  bool operator()(const DIG& a, const DIG& b) {
    return a.EVENT_TIMESTAMP > b.EVENT_TIMESTAMP;
  }
};
  
//unsigned long long                        evID = 0;
unsigned int                            numHit = 0;
unsigned short                   id[MAX_MULTI] = {0}; 
unsigned int        pre_rise_energy[MAX_MULTI] = {0};  
unsigned int       post_rise_energy[MAX_MULTI] = {0};  
unsigned long long        timestamp[MAX_MULTI] = {0};


#if FULL_OUTPUT
  uint32_t                      baseline[MAX_MULTI] = {0};
  unsigned short                geo_addr[MAX_MULTI] = {0};
  unsigned short                   flags[MAX_MULTI] = {0}; // bit 0 : external disc, bit 1 : peak valid, bit 2 : offset, bit 3 : sync error, bit 4 : general error, bit 5 : pileup only, bit 6 : pileup
  unsigned long long last_disc_timestamp[MAX_MULTI] = {0}; // Last discriminator timestamp
  unsigned int            peak_timestamp[MAX_MULTI] = {0}; // Peak timestamp
  unsigned int           m1_begin_sample[MAX_MULTI] = {0}; // M1 begin sample
  unsigned int             m1_end_sample[MAX_MULTI] = {0}; // M1 end sample
  unsigned int           m2_begin_sample[MAX_MULTI] = {0}; // M2 begin sample
  unsigned int             m2_end_sample[MAX_MULTI] = {0}; // M2 end sample
  unsigned int               peak_sample[MAX_MULTI] = {0}; // Peak sample
  unsigned int               base_sample[MAX_MULTI] = {0}; // Base sample
#endif

//^============================================
int main(int argc, char* argv[]) {

  printf("=======================================================\n");
  printf("===          Event Builder  raw data --> root       ===\n");
  printf("=======================================================\n");  

  if( argc <= 4){
    printf("%s [outfile] [timeWindow] [file-1] [file-2] ... \n", argv[0]);
    printf("      outfile : output root file name\n");
    printf("   timeWindow : nano-sec; if < 0, no event build\n"); 
    printf("       file-X : the raw data file(s)\n");
    return -1;
  }

  unsigned int runStartTime = getTime_us();

  TString outFileName = argv[1];
  int timeWindow = atoi(argv[2]);

  const int nFile = argc - 3;
  TString inFileName[nFile];
  for( int i = 0 ; i < nFile ; i++){
    inFileName[i] = argv[i+3];
  }

  printf(" out file : \033[1;33m%s\033[m\n", outFileName.Data());
  if ( timeWindow < 0 ){
    printf(" Event building time window : no event build\n");
  }else{
    printf(" Event building time window : %d nsec \n", timeWindow);
  }
  printf(" Number of input file : %d \n", nFile);
  
  //*=============== setup reader
  printf(" Scaning input files...\n");
  uint64_t totalNumHits = 0;
  uint64_t totFileSize = 0; // Total file size in bytes
  BinaryReader ** reader = new BinaryReader *[nFile];

  std::vector<std::thread> threads; // Create a vector of threads for parallel scanning

  for( int i = 0 ; i < nFile ; i++){
    reader[i] = new BinaryReader((MAX_READ_HITS)); 
    reader[i]->Open(inFileName[i].Data());
    threads.emplace_back([](BinaryReader* reader) { 
      reader->Scan(true); 
      printf("%s | %6.1f MB | # hit : %10d (%d)\n", reader->GetFileName().c_str(), reader->GetFileSize()/1024./1024., reader->GetNumData(), reader->GetMaxHitSize());
    }, reader[i]);
  }

  uint64_t globalEarliestTime = UINT64_MAX; // Global earliest timestamp
  uint64_t globalLastTime = 0; // Global last timestamp

  // Wait for all threads to finish
  for (int i = 0; i < threads.size(); ++i) {
    threads[i].join();
    totalNumHits += reader[i]->GetNumData();
    totFileSize += reader[i]->GetFileSize();

    if (reader[i]->GetGlobalEarliestTime() < globalEarliestTime) globalEarliestTime = reader[i]->GetGlobalEarliestTime();    
    if (reader[i]->GetGlobalLastTime() > globalLastTime) globalLastTime = reader[i]->GetGlobalLastTime();

  }
  
  //*=============== group files by DigID and sort the fileIndex
  std::map<unsigned short, std::vector<BinaryReader*>> fileGroups;
  for( int i = 0 ; i < nFile ; i++){
    unsigned short digID = reader[i]->GetUniqueID(); // Combine DigID and channel to create a unique identifier
    if (!fileGroups.count(digID) ) {
      fileGroups[digID] = std::vector<BinaryReader*>();
    }
    fileGroups[digID].push_back(reader[i]);
  }
  // printf("Found %zu DigIDs\n", fileGroups.size());
  
  // // printf out the file groups
  // for( const std::pair<const unsigned short, std::vector<BinaryReader*>>& group : fileGroups) { // looping through the map
  //   unsigned short digID = group.first; // DigID
  //   const auto& readers = group.second; // Vector of BinaryReader pointers
  //   printf("----- DigID %03d has %zu files\n", digID, readers.size());
  //   for (size_t j = 0; j < readers.size(); j++) {
  //     printf("  File %zu: %s | %6.1f MB | num. of hit : %10d\n", j, readers[j]->GetFileName().c_str(), readers[j]->GetFileSize()/1024./1024., readers[j]->GetNumData());
  //   }
  // }
  
  printf("================= Total number of hits: %lu\n", totalNumHits);
  printf("                       Total file size: %.1f MB\n", totFileSize / (1024.0 * 1024.0));
  printf("                        Total Run Time: %.3f s = %.3f min\n", (globalLastTime - globalEarliestTime) / 1e8, (globalLastTime - globalEarliestTime) / 1e8 / 60.0);
  printf("           Total number of file groups: %zu\n", fileGroups.size());


  //*=============== create output file and setup TTree
  TFile * outFile = TFile::Open(outFileName.Data(), "RECREATE");
  
  TTree * outTree = new TTree("tree", outFileName.Data());
  // outTree->SetAutoSave(10000000); // autosave every 10 million bytes
  // outTree->SetAutoFlush(1000000); // auto-flush every 1 million bytes 
  outTree->SetDirectory(outFile);

  outTree->Branch(         "NumHits",          &numHit, "NumHits/i");
  outTree->Branch(              "id",               id, "id[NumHits]/s");
  outTree->Branch( "pre_rise_energy",  pre_rise_energy, "pre_rise_energy[NumHits]/i");
  outTree->Branch("post_rise_energy", post_rise_energy, "post_rise_energy[NumHits]/i");
  outTree->Branch( "event_timestamp",        timestamp, "event_timestamp[NumHits]/l");
#if FULL_OUTPUT
  outTree->Branch(           "baseline",            baseline, "baseline[NumHits]/i");
  outTree->Branch(           "geo_addr",            geo_addr, "geo_addr[NumHits]/s");
  outTree->Branch(              "flags",               flags, "flags[NumHits]/s"); 
  outTree->Branch("last_disc_timestamp", last_disc_timestamp, "last_disc_timestamp[NumHits]/l"); 
  outTree->Branch(     "peak_timestamp",      peak_timestamp, "peak_timestamp[NumHits]/i"); 
  outTree->Branch(    "m1_begin_sample",     m1_begin_sample, "m1_begin_sample[NumHits]/i"); 
  outTree->Branch(      "m1_end_sample",       m1_end_sample, "m1_end_sample[NumHits]/i"); 
  outTree->Branch(    "m2_begin_sample",     m2_begin_sample, "m2_begin_sample[NumHits]/i");
  outTree->Branch(      "m2_end_sample",       m2_end_sample, "m2_end_sample[NumHits]/i");
  outTree->Branch(        "peak_sample",         peak_sample, "peak_sample[NumHits]/i");
  outTree->Branch(        "base_sample",         base_sample, "base_sample[NumHits]/i");
#endif

  printf("...... build tree branches\n");

  printf("...... Filling the initial data to hitsQueue\n");

  //*=============== read n data from each file
  std::map<unsigned short, unsigned int> hitID; // store the hit ID for the current reader for each DigID
  std::map<unsigned short, short> fileID; // store the file ID for each DigID
  
  std::priority_queue<DIG, std::vector<DIG>, CompareEvent> hitsQueue; // Priority queue to store events

  for( const std::pair<const unsigned short, std::vector<BinaryReader*>>& group : fileGroups) { // looping through the map
    unsigned short digID = group.first; // DigID
    const auto& readers = group.second; // Vector of BinaryReader pointers

    hitID[digID] = 0; // Initialize hitID for this DigID
    fileID[digID] = 0; 

    BinaryReader* reader = readers[0];
    reader->ReadNextNHitsFromFile();

    for( int i = 0; i < reader->GetHitSize(); i++) hitsQueue.push(reader->GetHit(i).DecodePayload()); // Decode the hit payload and push it to the event queue

    if( reader->GetHitSize() == reader->GetNumData() ) {
      printf("\033[34m====== DigID %03d, file %s has more hits\033[0m\n", digID, reader->GetFileName().c_str());
      fileID[digID]++; // Increment the file ID for this DigID
      if( fileID[digID] >= readers.size() ) {
        fileID[digID] = -1; // Mark that there are no more files for this DigID
      }
    }

  }

  printf("...... Initial hitsQueue size: %zu\n", hitsQueue.size());
  printf("...... Start event buinding\n");

  //*=============== event building
  std::vector<DIG> events; // Vector to store events
  unsigned int eventID = 0;
  
  unsigned int hitProcessed = 0; // Number of hits processed
  double last_precentage = 0.0; // Last percentage printed


  do{

    do{

      DIG hit = hitsQueue.top(); // Get the top event from the priority queue
      int digID = hit.UniqueID; // Get the DigID from the event

      if( fileID[digID] >= 0  ){ // check shoudl read the next N data or next file for this DigID

        // printf("\033[34m====== DigID %03d, file %d, hitID %d\033[0m\n", digID, fileID[digID], hitID[digID]);

        BinaryReader* reader = fileGroups[digID][fileID[digID]];
        if( hitID[digID] >= reader->GetHitSize() - 1) { // If the hitID exceeds the number of hits in the current file

          reader->ReadNextNHitsFromFile(); // Read more hits from the current file
          
          if( reader->GetHitSize() > 0 ) {
            hitID[digID] = 0; // Reset hitID for this DigID
            // printf("\033[34m====== DigID %03d, file %s has more hits (%d)\033[0m\n\n", digID, reader->GetFileName().c_str(), reader->GetHitSize());
            for( int i = 0; i < reader->GetHitSize(); i++)  hitsQueue.push(reader->GetHit(i).DecodePayload()); 
          }else{ // load next file if no hits
  
            printf("\033[31m====== DigID %03d, file %s has No more hits \033[0m\n", digID, reader->GetFileName().c_str());
            fileID[digID]++;

            if( fileID[digID] >= fileGroups[digID].size() - 1 ) {
              fileID[digID] = -1; // Mark that there are no more files for this DigID
            }else{
              reader = fileGroups[digID][fileID[digID]];
              reader->ReadNextNHitsFromFile(); // Read more hits from the current file
              hitID[digID] = 0; // Reset hitID for this DigID
              for( int i = 0; i < reader->GetHitSize(); i++) hitsQueue.push(reader->GetHit(i).DecodePayload());   
            }
  
          }
        }

      }

      if( events.size() == 0 ) {
        events.push_back(hit); // Add the hit to the events vector
        hitsQueue.pop();
        hitProcessed++; // Increment the hitProcessed count
        hitID[digID]++; // Increment the hitID for this DigID
        if( timeWindow < 0 ) break;
      }else{
        if( hit.EVENT_TIMESTAMP - events.front().EVENT_TIMESTAMP <= timeWindow ) {
          events.push_back(hit); // Add the hit to the events vector
          hitsQueue.pop();
          hitProcessed++; // Increment the hitProcessed count
          hitID[digID]++; // Increment the hitID for this DigID
        }else{
          break;
        }
      }

    }while(!hitsQueue.empty()); // Loop until the event queue is empty

    
    if( events.size() > 0 ) {
      std::sort(events.begin(), events.end(), [](const DIG& a, const DIG& b) {
        return a.EVENT_TIMESTAMP < b.EVENT_TIMESTAMP; // Sort events by timestamp
      });
      
      if( events.size() > MAX_MULTI ) {
        printf("\033[31mWarning: More than %d hits in one event, truncating to %d hits.\033[0m\n", MAX_MULTI, MAX_MULTI);
        numHit = MAX_MULTI; // Set numHit to MAX_MULTI
      }else{
        numHit = events.size(); 
      }
      
      for( int i = 0; i < numHit; i++) {
        id[i]               = events[i].UniqueID; // Unique ID  = DigID * 10 + channel
        pre_rise_energy[i]  = events[i].PRE_RISE_ENERGY; // Pre-rise energy
        post_rise_energy[i] = events[i].POST_RISE_ENERGY; // Post-rise energy
        timestamp[i]        = events[i].EVENT_TIMESTAMP; // Timestamp
        #if FULL_OUTPUT
        baseline[i]            = events[i].baseline; // Baseline value
        geo_addr[i]            = events[i].geo_addr; // Geo address
        flags[i]               = events[i].flags; // Flags, bit 0 : external disc, bit 1 : peak valid, bit 2 : offset, bit 3 : sync error, bit 4 : general error, bit 5 : pileup only, bit 6 : pileup
        last_disc_timestamp[i] = events[i].last_disc_timestamp; // Last discriminator timestamp
        peak_timestamp[i]      = events[i].peak_timestamp; // Peak timestamp
        m1_begin_sample[i]     = events[i].m1_begin_sample; // M1 begin sample
        m1_end_sample[i]       = events[i].m1_end_sample; // M1 end sample
        m2_begin_sample[i]     = events[i].m2_begin_sample; // M2 begin sample
        m2_end_sample[i]       = events[i].m2_end_sample; // M2 end sample
        peak_sample[i]         = events[i].peak_sample; // Peak sample
        base_sample[i]         = events[i].base_sample; // Base sample
        #endif
      }
      outTree->Fill(); // Fill the TTree with the current event
    }

    double percentage = hitProcessed * 100./ totalNumHits;
    if( percentage >= last_precentage ) {
      size_t memoryUsage = sizeof(DIG) * hitsQueue.size();
      printf("Processed : %u, %.1f%% | %u/%lu | (%zu) %.3f Mb", eventID, percentage, hitProcessed, totalNumHits, hitsQueue.size(), memoryUsage / (1024. * 1024.));
      printf(" \n\033[A\r");
      last_precentage = percentage + 0.1;
    }
    
    // prepare for the next event
    eventID ++;
    events.clear(); // Clear the events vector for the next event

  }while(!hitsQueue.empty()); 

  //*=============== save some marco
  TMacro macro("info", "Earliest and Last Timestamps");
  macro.AddLine(Form("globalEarliestTime = %lu", globalEarliestTime));
  macro.AddLine(Form("globalLastTime = %lu", globalLastTime));
  macro.AddLine(Form("totalNumHits = %lu", totalNumHits));
  macro.AddLine(Form("totFileSizeMB = %.1f", totFileSize / (1024.0 * 1024.0))); // Convert to MB
  macro.Write("info");


  //*=============== summary
  printf("\n\n");
  printf("=======================================================\n");
  printf("===          Event Builder finished                 ===\n");
  printf("=======================================================\n");
  unsigned int runEndTime = getTime_us();
  printf("              Total time taken: %.3f s = %.3f min\n", (runEndTime - runStartTime) / 1000000.0, (runEndTime - runStartTime) / 1000000.0 / 60.0);
  printf("Total number of hits processed: %10u (%lu) | %.1f%%\n", hitProcessed, totalNumHits, hitProcessed * 100.0 / totalNumHits);
  printf("  Total number of events built: %10u\n", eventID);
  printf("     Number of entries in tree: %10lld\n", outTree->GetEntries());
  //clean up
  outFile->Write();
  outTree->Print();
  outFile->Close();
  for( int i = 0 ; i < nFile ; i++){
    if( reader[i] == nullptr ) continue; // Skip if reader is null
    delete reader[i]; // Delete each BinaryReader
  }
  delete[] reader; // Delete the array of BinaryReader pointers
  printf("Output file \033[31m%s\033[0m created successfully.\n", outFileName.Data());

  return 0;
}