/*************************************** 
 * 
 * created by Ryan Tang 2025, Oct 22
 * 
 * 
****************************************/

#include "BinaryReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TMacro.h"
#include "TString.h"
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory> // added for unique_ptr
#include <vector>

#define MAX_TRACE_LEN 1250 
#define MAX_TRACE_MULTI 60
#define MAX_READ_HITS 100000 // Maximum hits to read at a time
#define MAX_QUEUE_SIZE 10000 // Maximum size of the data queue
#define MAX_MULTI 400
#define MWIN 350.

#include <sys/time.h> /** struct timeval, select() */
inline unsigned int getTime_us(){
  unsigned int time_us;
  struct timeval t1;
  struct timezone tz;
  gettimeofday(&t1, &tz);
  time_us = (t1.tv_sec) * 1000 * 1000 + t1.tv_usec;
  return time_us;
}

#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>

#include "misc.h"

struct trace_data {
  size_t n;
  double *y;
};

int fit_model(const gsl_vector * x, void *data, gsl_vector * f){
  size_t n = ((struct trace_data *)data)->n;
  double *y = ((struct trace_data *)data)->y;

  double A        = gsl_vector_get(x, 0);
  double T0       = gsl_vector_get(x, 1);
  double riseTime = gsl_vector_get(x, 2);
  double B        = gsl_vector_get(x, 3);

  for (size_t i = 0; i < n; ++i){
    double t = i;
    double Yi = A / (1 + exp(-(t - T0) / riseTime)) + B;
    gsl_vector_set(f, i, Yi - y[i]);
  }

  return GSL_SUCCESS;
}

// Comparator for min-heap (smallest timestamp on top)
struct CompareEvent {
  bool operator()(const DIG& a, const DIG& b) const {
    return a.EVENT_TIMESTAMP > b.EVENT_TIMESTAMP;
  }
};

class Data {
public:

  unsigned int evID = 0 ;

  unsigned int                            numHit = 0;
  unsigned short                   id[MAX_MULTI] = {0}; 
  unsigned short                detID[MAX_MULTI] = {0}; 
  unsigned int        pre_rise_energy[MAX_MULTI] = {0};
  unsigned int       post_rise_energy[MAX_MULTI] = {0};  
  unsigned long long        timestamp[MAX_MULTI] = {0};
  unsigned long long           trigTS[MAX_MULTI] = {0};
  
  // trace
  unsigned short                            traceCount = 0; 
  unsigned short           traceDetID[MAX_TRACE_MULTI] = {0};
  unsigned short             traceLen[MAX_TRACE_MULTI] = {0};
  unsigned short trace[MAX_TRACE_MULTI][MAX_TRACE_LEN] = {0};

  // trace analysis
  float tracePara[MAX_TRACE_MULTI][4]; // trace parameters, 0 = Amplitude, 1 = Rise time, 2 = Timestamp, 3 = Baseline
  float traceChi2[MAX_TRACE_MULTI]; // trace fit chi2

  Data(){}
  ~Data(){}

  void Reset() {

    numHit = 0;

    for (int i = 0; i < MAX_MULTI; i++) {
      id[i] = 0;
      detID[i] = 0;
      pre_rise_energy[i] = 0;
      post_rise_energy[i] = 0;
      timestamp[i] = 0;
      trigTS[i] = 0;
    }

    traceCount = 0; 
    for (int i = 0; i < MAX_TRACE_MULTI; i++) {
      traceDetID[i] = 0;
      traceLen[i] = 0;
      for (int j = 0; j < MAX_TRACE_LEN; j++) trace[i][j] = 0;
      for (int j = 0; j < 4; j++) tracePara[i][j] = TMath::QuietNaN();
      traceChi2[i] = TMath::QuietNaN();
    }

  }

  void FillData(std::vector<DIG> dig, bool saveTrace){

    numHit = dig.size();

    for( int i = 0; i <  dig.size(); i++) {
      id[i]                    = dig[i].UniqueID; // Unique ID  = DigID * 100 + channel
      unsigned short boardID = id[i]/100;
      unsigned short channel = id[i]%100;
      detID[i] = channelMap[boardID][channel];
      if (boardID == 99 && channel == 20) {
        detID[i] = 999; // TAC channel set to detID 0
      }
      pre_rise_energy[i]  = dig[i].PRE_RISE_ENERGY; // Pre-rise energy
      post_rise_energy[i] = dig[i].POST_RISE_ENERGY; // Post-rise energy
      timestamp[i]        = dig[i].EVENT_TIMESTAMP; // Timestamp
      trigTS[i]           = dig[i].TS_OF_TRIGGER_FULL; // Trigger timestamp

      // float eee = ((float)post_rise_energy[i] - (float)pre_rise_energy) / MWIN; 

      // printf(" post_rise_energy : %u, pre_rise_energy : %u, eee : %.3f \n", post_rise_energy, pre_rise_energy, eee);

      if( saveTrace ){

        if( detID[i] == 999 ) continue; // No trace for TAC channel
        if( detID[i] == 0 ) continue;   // No trace for detID 0

        if( dig[i].traceData.size() == 0 ) continue; // No trace data

        if( traceCount >= MAX_TRACE_MULTI ) {
          printf("\033[31mWarning: More than %d traces, truncating to %d traces. (event size %lu)\033[0m\n", MAX_TRACE_MULTI, MAX_TRACE_MULTI, dig.size());
        }else{
        
          // printf("iDet: %d, idKind: %d, traceLength: %d\n", idDet, idKind, dig[i].traceLength);
          traceLen[traceCount] = dig[i].traceData.size();
          traceDetID[traceCount] = id[i];
          uint16_t regulatedValue = 0;
          for( int j = 0; j < traceLen[traceCount]; j++){
            trace[traceCount][j] = dig[i].traceData[j];
          }

          traceCount++;
        }
        
      }
    }
  }

  void TraceAnalysis(){
    if( traceCount <= 0 ) return; // No traces to analyze
    
    for( int i = 0; i < traceCount; i++ ){
      if( traceLen[i] <= 0 ) continue; // Skip if trace length is zero
      
      // set the trace_data (use vector to manage memory and avoid leaks)
      int len = traceLen[i];
      if (len > MAX_TRACE_LEN) len = MAX_TRACE_LEN; // clamp to buffer size
      std::vector<double> yvec(len);
      for (int j = 0; j < len; ++j) yvec[j] = trace[i][j];
      struct trace_data d = { (size_t)len, yvec.data() };
          
      gsl_multifit_nlinear_fdf f;
      f.f = &fit_model;
      f.df = NULL;   // Use finite differences
      f.n = d.n;
      f.p = 4;
      f.params = &d;
      
      gsl_vector *x = gsl_vector_alloc(4);
      gsl_vector_set(x, 0, 100.0); // Initial guess for A
      gsl_vector_set(x, 1, 100.0); // Initial guess for T0
      gsl_vector_set(x, 2, 1.0);   // Initial guess for rise time
      gsl_vector_set(x, 3, 8000.0); // Initial guess for baseline
      
      const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
      gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();
      gsl_multifit_nlinear_workspace *w = gsl_multifit_nlinear_alloc(T, &fdf_params, d.n, 4);
      gsl_multifit_nlinear_init(x, &f, w);

      // Solve the system using iteration
      int status;
      size_t iter = 0, max_iter = 100;
      do {
        status = gsl_multifit_nlinear_iterate(w);
        if (status) break;
        status = gsl_multifit_test_delta(w->dx, w->x, 1e-5, 1e-5);
        iter++;
      } while (status == GSL_CONTINUE && iter < max_iter);

      // Store the results
      gsl_vector *result = gsl_multifit_nlinear_position(w);
      tracePara[i][0] = gsl_vector_get(result, 0);
      tracePara[i][1] = gsl_vector_get(result, 1);
      tracePara[i][2] = gsl_vector_get(result, 2);
      tracePara[i][3] = gsl_vector_get(result, 3);

      // Calculate chi2
      gsl_vector *f_res = gsl_multifit_nlinear_residual(w);
      double chi2 = 0.0;
      for (size_t k = 0; k < f_res->size; k++) {
        double r = gsl_vector_get(f_res, k);
        chi2 += r * r;
      }
      traceChi2[i] = chi2;

      gsl_multifit_nlinear_free(w);
      gsl_vector_free(x);
    }
  }
  
  void PrintTraceAnalysisResult(){
    for( int i = 0; i < traceCount; i++ ){
      if( traceLen[i] <= 0 ) continue; // Skip if trace length is zero
      
      for(int j = 0; j < traceLen[i]; j++) {
        printf("Trace[%d]| %3d %5u\n", i, j, trace[i][j]);
      }
      printf("======== Trace: %d A = %.2f, T0 = %.2f, Rise time = %.2f, Baseline = %.2f\n", 
            i, tracePara[i][0], tracePara[i][1], tracePara[i][2], tracePara[i][3]);
    }
  }

};


//^============================================
int main(int argc, char* argv[]) {

  printf("=======================================================\n");
  printf("===        Event Builder S  raw data --> root       ===\n");
  printf("=======================================================\n");  

  if( argc <= 4){
    printf("%s [outfile] [timeWindow] [save Trace] [trace Analysis] [file-1] [file-2] ... \n", argv[0]);
    printf("        outfile : output root file name\n");
    printf("     timeWindow : tick; if < 0, no event build\n"); 
    printf("     Save Trace : 0 : no, 1 : yes\n");
    printf(" trace Analysis : 0 : no, 1 : yes (single core), >1 : multi-core\n");
    printf("         file-X : the input file(s)\n");
    return -1;
  }

  unsigned int runStartTime = getTime_us();

  TString outFileName = argv[1];
  int timeWindow = atoi(argv[2]);
  const bool saveTrace = atoi(argv[3]);
  const short nWorkers = atoi(argv[4]);

  Data data;

  const int nFile = argc - 5;
  TString inFileName[nFile];
  for( int i = 0 ; i < nFile ; i++){
    inFileName[i] = argv[i+5];
  }

  printf(" out file : \033[1;33m%s\033[m\n", outFileName.Data());
  if ( timeWindow < 0 ){
    printf(" Event building time window : no event build\n");
  }else{
    printf(" Event building time window : %d ticks \n", timeWindow);
  }
  printf("       Save Trace ? %s\n", saveTrace ? "Yes" : "No");
  printf("   Trace Analysis ? %s %s\n", nWorkers ? "Yes" : "No", nWorkers > 0 ? Form("(%d-core)", nWorkers) : "");
  if( saveTrace ){
    printf("  MAX Trace Length : %d\n", MAX_TRACE_LEN);
    printf("   MAX Trace Multi : %d\n", MAX_TRACE_MULTI);
  }
  printf("     MAX Hits read : %d\n", MAX_READ_HITS);
  printf("    MAX Queue Size : %d\n", MAX_QUEUE_SIZE);
  printf(" Number of input file : %d \n", nFile);
  
  //*=============== setup reader
  printf(" Scaning input files...\n");
  uint64_t totalNumHits = 0;
  uint64_t totFileSize = 0; // Total file size in bytes
  BinaryReader ** reader = new BinaryReader *[nFile];

  unsigned short globalRunID = 0;

  std::vector<std::thread> threads;

  for( int i = 0 ; i < nFile ; i++){
    reader[i] = new BinaryReader((MAX_READ_HITS)); 
    reader[i]->Open(inFileName[i].Data());
    globalRunID = reader[i]->GetRunID(); // Get the run ID from the first file
    if( i > 0 && globalRunID != reader[i]->GetRunID() ) {
      printf("\033[31mWarning: Run ID mismatch between files. Using the first file's Run ID: %d.\033[0m\n", globalRunID);
    }
    threads.emplace_back([](BinaryReader* reader) { 
      reader->Scan(true); 
      //printf("%s | %6.1f MB | # hit : %10d (%d)\n", reader->GetFileName().c_str(), reader->GetFileSize()/1024./1024., reader->GetNumData(), reader->GetMaxHitSize());
    }, reader[i]);
  }

  uint64_t globalEarliestTime = UINT64_MAX; // Global earliest timestamp
  uint64_t globalLastTime = 0; // Global last timestamp

  // Wait for all threads to finish
  for (int i = 0; i < threads.size(); ++i) {
    threads[i].join();
    totalNumHits += reader[i]->GetNumData();
    totFileSize += reader[i]->GetFileSize();

    if (reader[i]->GetChannel() < 10 ){
      if (reader[i]->GetGlobalEarliestTime() < globalEarliestTime) globalEarliestTime = reader[i]->GetGlobalEarliestTime();
      if (reader[i]->GetGlobalLastTime() > globalLastTime) globalLastTime = reader[i]->GetGlobalLastTime();
    }
  }

  threads.clear();
  
  //*=============== group files by DigID and sort the fileIndex
  std::map<unsigned short, std::vector<BinaryReader*>> fileGroups;
  for( int i = 0 ; i < nFile ; i++){
    unsigned short uniqueID = reader[i]->GetUniqueID();
    if (!fileGroups.count(uniqueID) ) {
      fileGroups[uniqueID] = std::vector<BinaryReader*>();
    }
    fileGroups[uniqueID].push_back(reader[i]);
  }
  // printf("Found %zu DigIDs\n", fileGroups.size());
  
  // printf out the file groups
  // for( const std::pair<const unsigned short, std::vector<BinaryReader*>>& group : fileGroups) { // looping through the map
  //   unsigned short uniqueID = group.first; // DigID
  //   const auto& readers = group.second; // Vector of BinaryReader pointers
  //   printf("----- DigID %03d has %zu files\n", uniqueID, readers.size());
  //   for (size_t j = 0; j < readers.size(); j++) {
  //     printf("  File %zu: %s | %6.1f MB | num. of hit : %10d\n", j, readers[j]->GetFileName().c_str(), readers[j]->GetFileSize()/1024./1024., readers[j]->GetNumData());
  //   }
  // }
  
  printf("================= Total number of hits: %lu\n", totalNumHits);
  printf("                       Total file size: %.1f MB\n", totFileSize / (1024.0 * 1024.0));
  printf("                        Total Run Time: %.3f s = %.3f min\n", (globalLastTime - globalEarliestTime) / 1e8, (globalLastTime - globalEarliestTime) / 1e8 / 60.0);


  //*=============== create output file and setup TTree
  TFile * outFile = TFile::Open(outFileName.Data(), "RECREATE");
  TTree * outTree = new TTree("tree", outFileName.Data());
  outTree->SetDirectory(outFile);

  outTree->Branch("evID",   &data.evID, "evID/i");
  
  outTree->Branch(         "NumHits",          &data.numHit, "NumHits/i");
  outTree->Branch(              "id",               data.id, "id[NumHits]/s");
  outTree->Branch(           "detID",            data.detID, "detID[NumHits]/s");
  outTree->Branch( "pre_rise_energy",  data.pre_rise_energy, "pre_rise_energy[NumHits]/i");
  outTree->Branch("post_rise_energy", data.post_rise_energy, "post_rise_energy[NumHits]/i");
  outTree->Branch(         "eventTS",        data.timestamp, "eventTS[NumHits]/l");
  outTree->Branch(          "trigTS",           data.trigTS, "trigTS[NumHits]/l");
  
  if( saveTrace ){
    outTree->Branch( "traceCount",  &data.traceCount, "traceCount/s");
    outTree->Branch( "traceDetID",   data.traceDetID, "traceDetID[traceCount]/s");
    outTree->Branch(   "traceLen",     data.traceLen, "traceLen[traceCount]/s");
    outTree->Branch(      "trace",        data.trace, Form("trace[traceCount][%d]/s", MAX_TRACE_LEN));
  }
  if( nWorkers > 0 ){ 
    outTree->Branch("tracePara", data.tracePara, "tracePara[traceCount][4]/F");
    outTree->Branch("traceChi2", data.traceChi2, "traceChi2[traceCount]/F");
  }

  LoadChannelMapFromFile();

  //*=============== read n data from each file
  std::map<unsigned short, unsigned int> hitID; // store the hit ID for the current reader for each DigID
  std::map<unsigned short, short> fileID; // store the file ID for each DigID
  
  std::priority_queue<DIG, std::vector<DIG>, CompareEvent> eventQueue; // Priority queue to store events for each thread

  unsigned long skippedTrashData = 0;

  for( const std::pair<const unsigned short, std::vector<BinaryReader*>>& group : fileGroups) { // looping through the map
    unsigned short uniqueID = group.first; // DigID
    const auto& readers = group.second; // Vector of BinaryReader pointers

    hitID[uniqueID] = 0; // Initialize hitID for this DigID
    fileID[uniqueID] = 0; 

    BinaryReader* reader = readers[0];

    do{
      reader->ReadNextNHitsFromFile();
      for( int i = 0; i < reader->GetHitSize(); i++) {
        std::unique_ptr<DIG> decodedHit = std::make_unique<DIG>(reader->GetHit(i).DecodePayload(saveTrace)); // Decode the hit payload
        if( decodedHit->HEADER_TYPE == TRASH_DATA ) {
          skippedTrashData ++;
          hitID[uniqueID] ++;
          continue; // skip trash data
        }
        eventQueue.push(std::move(*decodedHit)); // Move the decoded hit into the event queue
      }
      if( hitID[uniqueID] == 0 ) break;
      if( hitID[uniqueID] >= reader->GetNumData() - 1 ) {
        printf("\033[31m====== DigID %03d, file %s has No more hits \033[0m\n", uniqueID, reader->GetFileName().c_str());
        break;
      }
    }while( hitID[uniqueID] >= reader->GetHitSize() - 1 );
    // printf(" UniqueID %03d | %zu files | Initial hits loaded: %d \n", uniqueID, readers.size(), reader->GetHitSize());

    if( reader->GetHitSize() == reader->GetNumData() ) {
      fileID[uniqueID]++; // Increment the file ID for this DigID
      if( fileID[uniqueID] >= readers.size() ) {
        fileID[uniqueID] = -1; // Mark that there are no more files for this DigID
      }else{
        printf("\033[34m====== DigID %03d, file %s done, read next file\033[0m\n", uniqueID, reader->GetFileName().c_str()); 
      }
    }
  }

  printf("Initial loading done. Start event building...\n");
  printf(" eventQueue size : %lu \n", eventQueue.size()); 

  //*=============== event building
  std::vector<DIG> events; // Vector to store events
  unsigned int eventID = 0;
  
  unsigned int hitProcessed = skippedTrashData; // Number of hits processed
  double last_precentage = 0.0; // Last percentage printed

  std::mutex queueMutex;
  using DataPtr = std::unique_ptr<Data>;
  std::queue<DataPtr> dataQueue; // Queue of unique_ptr to avoid expensive Data copies
  std::mutex outQueueMutex; // Mutex for output queue and file writing
  // Use an ordered map keyed by evID so the writer thread can emit entries in evID order
  std::map<unsigned int, DataPtr> outputMap;
  unsigned int nextWriteEvID = 0;
  std::condition_variable trace_cv; // Condition variable for thread synchronization
  std::atomic<bool> done(false); // to flag all data is processed. to tell the threads to finish

  std::atomic<bool> allEventProcessed(false);
  std::condition_variable fileCv; // Condition variable for file writing
  std::thread outTreeThread;

  DataPtr tempData; // used for multi-threaded processing (allocated when needed)

  if( nWorkers > 1 ) {
       
    //* Create worker threads for trace analysis
    for (int i = 0; i < nWorkers; i++) {
      threads.emplace_back([i, &dataQueue, &outputMap, &queueMutex, &trace_cv, &fileCv, &done, &outQueueMutex]() {
        DataPtr localData;
        int count = 0;
        while (true) {

          {
            std::unique_lock<std::mutex> lock(queueMutex);
            trace_cv.wait(lock, [&] { return !dataQueue.empty() || done; });
            if (dataQueue.empty() && done) break;
            localData = std::move(dataQueue.front()); // move pointer out of queue (no Data copy)
            dataQueue.pop();
          }
          // Process data
          if (localData) localData->TraceAnalysis(); // Perform trace analysis if enabled
          count++;

          {// push processed data to ordered output map
            std::lock_guard<std::mutex> lock(outQueueMutex);
            if (localData) {
              unsigned int id = localData->evID;
              // emplace will insert the DataPtr keyed by evID; if a duplicate key exists, it will not overwrite
              outputMap.emplace(id, std::move(localData));
            }
            fileCv.notify_one(); // Notify the output tree thread to write data
          }
        }

        printf("Trace worker %2d finished processing. total processed event : %d\n", i, count);
      });
    }

    //* Create a thread to write the output tree
    outTreeThread = std::thread([&]() {
      int count = 0;
      while (true) {

        std::unique_lock<std::mutex> lock(outQueueMutex);
        // Wait until either (a) the expected next evID is available in the map, or (b) all events processed
        fileCv.wait(lock, [&] {
          if (allEventProcessed) return !outputMap.empty() || outputMap.empty();
          return outputMap.find(nextWriteEvID) != outputMap.end();
        });

        if (allEventProcessed && outputMap.empty()) {
          printf("Writing data to output tree: %d items processed, out Queue size %ld\n", count, (long)outputMap.size());
          break; // Exit if all data is processed and output map is empty
        }

        // Prefer to write the expected next evID. If we're finishing (allEventProcessed) and the exact ID
        // isn't available, fall back to writing the smallest available evID.
        auto it = outputMap.find(nextWriteEvID);
        if (it == outputMap.end()) {
          // Not found; in normal operation we should wait until nextWriteEvID is produced.
          // If we're here, it means allEventProcessed==true (because the waiter would have blocked otherwise).
          it = outputMap.begin();
        }

        DataPtr temp_data = std::move(it->second);
        unsigned int writtenEv = it->first;
        outputMap.erase(it);
        lock.unlock(); // Release outQueueMutex as soon as possible

        // Copy into the tree-fill object (one copy here)
        //TODO use TBranch SetAddress to directly point to the data in temp_data to avoid this copy
        if (temp_data) data = *temp_data;

        outTree->Fill(); // Fill the tree with the data
        outFile->cd(); // Ensure the output file is set as the current directory

        // Advance nextWriteEvID to the next expected value. If we wrote a later ID while finishing,
        // set nextWriteEvID to writtenEv + 1 so subsequent writes continue in increasing order.
        nextWriteEvID = writtenEv + 1;

        count++;
      }
      outTree->Write(); // Write the tree to the output file
      printf("Output tree written to file: %s. total processed event : %d\n", outFileName.Data(), count);
    });

  }

  printf("Starting event building loop...\n");

  do{
    ///============================== forming events and check is need to load more hits from files
    do{

      events.push_back(eventQueue.top()); 
      int uniqueID = events.back().UniqueID; 
      hitID[uniqueID]++;
      hitProcessed ++;

      if( fileID[uniqueID] >= 0  ){ // check shoudl read the next N data or next file for this DigID

        BinaryReader* reader = fileGroups[uniqueID][fileID[uniqueID]];
        if( hitID[uniqueID] >= reader->GetHitSize() * 0.9) { // If the hitID exceeds the number of hits in the current file
          reader->ReadNextNHitsFromFile(); // Read more hits from the current file
          if( reader->GetHitSize() > 0 ) {
            hitID[uniqueID] = 0; // Reset hitID for this DigID
            for( int i = 0; i < reader->GetHitSize(); i++)  {
              std::unique_ptr<DIG> decodedHit = std::make_unique<DIG>(reader->GetHit(i).DecodePayload(saveTrace)); // Decode the hit payload
              if( decodedHit->HEADER_TYPE == TRASH_DATA ) {
                skippedTrashData ++;
                hitID[uniqueID] ++;
                hitProcessed ++;
                printf(" skipped trash data for DigID %03d | skippedData %lu \n", uniqueID, skippedTrashData );
                continue; // skip trash data
              }
              eventQueue.push(std::move(*decodedHit)); 
            }
          }
        }

        if( reader->GetHitSize() == 0 ) { // load next file if no hits
          if( fileID[uniqueID] < fileGroups[uniqueID].size() - 1 ) {
            reader->DeleteHits(); // Delete the hits from the current reader, free memory
            fileID[uniqueID]++;
            // printf("\033[34m====== No hits in current file for DigID %03d, loading next file...%s\033[0m\n", uniqueID, readers[fileID[uniqueID]]->GetFileName().c_str());
            reader = fileGroups[uniqueID][fileID[uniqueID]];
            reader->ReadNextNHitsFromFile(); // Read more hits from the next file

            hitID[uniqueID] = 0; // Reset hitID for this DigID
            for( int i = 0; i < reader->GetHitSize(); i++)  {
              std::unique_ptr<DIG> decodedHit = std::make_unique<DIG>(reader->GetHit(i).DecodePayload(saveTrace)); // Decode the hit payload
              if( decodedHit->HEADER_TYPE == TRASH_DATA ) {
                skippedTrashData ++;
                hitID[uniqueID] ++;
                hitProcessed ++;
                printf(" skipped trash data for DigID %03d | skippedData %lu \n", uniqueID, skippedTrashData );
                continue; // skip trash data
              }
              eventQueue.push(std::move(*decodedHit)); 
            }

          } else {
            fileID[uniqueID] = -1; // Mark that there are no more files for this DigID
          }
        }
      }      

      if( timeWindow >= 0 && events.size() > 0 && events.back().EVENT_TIMESTAMP - events.front().EVENT_TIMESTAMP > timeWindow) {
        events.pop_back(); // Remove the last event if it exceeds the time window
        hitID[uniqueID]--; // Decrement the hitID for this DigID
        hitProcessed--; // Decrement the hitProcessed count
        break;
      }

      eventQueue.pop();
      if( timeWindow < 0 ) break;

    }while(!eventQueue.empty()); // Loop until the event queue is empty

    // printf("Event %u formed with %lu hits.\n", eventID, events.size());

    ///============================== process the events
    if( events.size() > 0 ) {
      std::sort(events.begin(), events.end(), [](const DIG& a, const DIG& b) {
        return a.EVENT_TIMESTAMP < b.EVENT_TIMESTAMP; // Sort events by timestamp
      });
      
      if( nWorkers > 1 ) {
        // Multi-threaded processing
        while(true){

          std::unique_lock<std::mutex> lock(queueMutex); // Lock the queue mutex
          if (dataQueue.size() < MAX_QUEUE_SIZE) { // Check if the queue size is within the limit
            tempData.reset(new Data()); // allocate new Data once per push
            tempData->Reset();
            tempData->evID = eventID; // Set the event ID
            // printf("Main thread pushing event %u to dataQueue (size: %ld)\n", eventID, dataQueue.size());
            //TODO, the dataQueue can be a queue of events, and the FillData can be done in the worker thread
            tempData->FillData(events, saveTrace); // Fill data with the events
            dataQueue.push(std::move(tempData)); // move the pointer into the queue (no copy)
            lock.unlock(); // Unlock the queue mutex
            trace_cv.notify_one(); // Notify one of the worker threads to start processing
            break;
          }
          lock.unlock(); // Unlock the queue mutex 
          if( dataQueue.size() >= MAX_QUEUE_SIZE ) {
            //wait for 10 miniseconds before checking the queue again
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep for
            continue;
          }

        }

      } else if( nWorkers == 1){
        data.Reset();
        data.evID = eventID; // Set the event ID
        data.FillData(events, saveTrace); // Fill data with the events
        data.TraceAnalysis(); // Perform trace analysis if enabled

        outTree->Fill(); // Fill the tree with the processed data
        
      } else {
        // Single-threaded processing, o trace analysis
        data.Reset();
        data.evID = eventID; // Set the event ID
        data.FillData(events, saveTrace);

        outTree->Fill(); // Fill the tree
      }

    }
    
    ///=============== print progress
    double percentage = hitProcessed * 100/ totalNumHits;
    if( percentage >= last_precentage ) {
      size_t memoryUsage = sizeof(DIG) * eventQueue.size();
      printf("Processed : %u, %.0f%% | %u/%lu | %.3f Mb", eventID, percentage, hitProcessed, totalNumHits, memoryUsage/ (1024. * 1024.));
      printf(" \n\033[A\r");
      last_precentage = percentage + 1.0;
    }
    
    // prepare for the next event
    eventID ++;
    events.clear(); // Clear the events vector for the next event

  }while(!eventQueue.empty()); 
  
  if ( nWorkers > 1 ){

    printf("\nWait for all threads to finish processing...\n");
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      printf("dataQueue size before finishing: %ld\n", dataQueue.size());
    }
    done = true; // All data is processed, set the done flag to true
    trace_cv.notify_all();
    for( int i = 0; i < nWorkers; i++) {
      threads[i].join(); // Wait for all threads to finish
    }
    printf("All trace analysis threads finished processing.\n");
    allEventProcessed = true;
    fileCv.notify_all(); // Notify the output tree thread to finish writing data
    {
      std::unique_lock<std::mutex> lock(outQueueMutex);
      printf("outQueue size before finishing: %ld\n", (long)outputMap.size());
    }
    outTreeThread.join(); // Wait for the output tree thread to finish
    printf("Output tree thread finished writing to file.\n");
  }

  //*=============== save some marco
  //Save the global earliest and last timestamps as a TMacro
  TMacro macro("info", "Earliest and Last Timestamps");
  macro.AddLine(Form("globalEarliestTime = %lu", globalEarliestTime));
  macro.AddLine(Form("globalLastTime = %lu", globalLastTime));
  macro.AddLine(Form("totalNumHits = %lu", totalNumHits));
  macro.AddLine(Form("totFileSizeMB = %.1f", totFileSize / (1024.0 * 1024.0))); // Convert to MB
  macro.Write("info");

  TMacro macro2("trace_info", "Maximum Trace Length"); //this macro is for read Raw trace
  macro2.AddLine(Form("%d", MAX_TRACE_LEN));
  macro2.Write("trace_info");

  //*=============== summary
  printf("=======================================================\n");
  printf("===          Event Builder S finished               ===\n");
  printf("=======================================================\n");
  unsigned int runEndTime = getTime_us();
  printf("              Total time taken: %.3f s = %.3f min\n", (runEndTime - runStartTime) / 1000000.0, (runEndTime - runStartTime) / 1000000.0 / 60.0);
  printf("Total number of hits processed: %10u (%lu)\n", hitProcessed, totalNumHits);
  printf("                  Events / sec: %10.2f\n", eventID / ((runEndTime - runStartTime) / 1000000.0));
  printf("      skipped trash data count: %10lu\n", skippedTrashData);
  printf("  Total number of events built: %10u\n", eventID);
  printf("     Number of entries in tree: %10lld\n", outTree->GetEntries());
  //clean up
  outFile->Write();
  outFile->Close();
  for( int i = 0 ; i < nFile ; i++){
    delete reader[i]; // Delete each BinaryReader
  }
  delete[] reader; // Delete the array of BinaryReader pointers
  printf("Output file \033[31m%s\033[0m created successfully.\n", outFileName.Data());

  return 0;
}