#ifndef MISC_H
#define MISC_H

#include <map>
#include <string>
#include <cstdio>

// Define channelMap as a global variable before the function
std::map<unsigned short, std::map<unsigned short, unsigned short>> channelMap;
unsigned short numberOfDetectors = 0;

void LoadChannelMapFromFile(){

  std::string chanMapFile = "GS_channel_map.txt";

  channelMap.clear();

  FILE * f = fopen(chanMapFile.c_str(), "r");
  if( f == NULL ){
    printf("Channel map file %s not found! \n", chanMapFile.c_str());
    return;
  }
  // Skip the first 2 rows
  char skipLine[256];
  fgets(skipLine, sizeof(skipLine), f);
  fgets(skipLine, sizeof(skipLine), f);

  unsigned short detID, digID, chID;
  int VME, board;
  while( fscanf(f, "%hu %d %hu %hu %d", &detID, &VME, &digID, &chID, &board) != EOF ){
    if ( VME <= 0 ) continue;
    numberOfDetectors ++;
    chID = chID + 5; // the original chID is for BGO, and need to +5 for HPGe
    // printf("VME: %2d, Dig: %hu, Ch: %hu | boardID : %3hu detID: %2hu\n", VME, digID, chID, board, detID);
    channelMap[(unsigned short)board][chID] = detID;
  }
  fclose(f);

  printf("Loaded channel map from file: %s\n", chanMapFile.c_str());
  printf("Total number of detectors: %hu\n", numberOfDetectors);
  return;
}

#endif // MISC_H