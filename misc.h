#ifndef MISC_H
#define MISC_H

#include <map>
#include <string>
#include <cstdio>

// Define channelMap as a global variable before the function
std::map<unsigned short, std::map<unsigned short, unsigned short>> channelMap;
std::map<unsigned short, std::map<unsigned short, unsigned short>> VMEDIGtoBoard;
unsigned short numberValidDetectors = 0;

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
    numberValidDetectors ++;
    chID = chID + 5; // the original chID is for BGO, and need to +5 for HPGe
    // printf("VME: %2d, Dig: %hu, Ch: %hu | boardID : %3hu detID: %2hu\n", VME, digID, chID, board, detID);
    channelMap[(unsigned short)board][chID] = detID;
    VMEDIGtoBoard[(unsigned short)VME][digID] = (unsigned short)board;
  }
  fclose(f);

  printf("Loaded channel map from file: %s\n", chanMapFile.c_str());
  printf("Total number of valid detectors: %hu\n", numberValidDetectors);

  return;
}

#include <vector>

std::pair<unsigned short, unsigned short> FindVMEDIGFromBoardID(unsigned short boardID){
  std::pair<unsigned short, unsigned short> result;
  for( const auto& vmePair : VMEDIGtoBoard ){
    unsigned short vme = vmePair.first;
    for( const auto& digPair : vmePair.second ){
      unsigned short dig = digPair.first;
      unsigned short bID = digPair.second;
      if( bID == boardID ){
        result = std::make_pair(vme, dig);
        printf("Found BoardID %3hu => VME: %2hu, DigID: %2hu\n", boardID, vme, dig);
        return result;
      }
    }
  }
  return result;
}

std::vector<int> FindBoardIDFromDetID(unsigned short detID){
  std::vector<int> boardIDs;
  for( const auto& boardPair : channelMap ){
    unsigned short boardID = boardPair.first;
    for( const auto& chPair : boardPair.second ){
      unsigned short channelID = chPair.first;
      unsigned short dID = chPair.second;
      if( dID == detID ){
        boardIDs.push_back(boardID);
        printf("Found DetID %2hu => BoardID: %3hu, ChannelID: %3hu\n", detID, boardID, channelID);
      }
    }
  }
  return boardIDs;
}

std::vector<int> FindVMEDIGCHFromDetID(unsigned short detID){
  std::vector<int> vmeDigCh;
  for( const auto& boardPair : channelMap ){
    unsigned short boardID = boardPair.first;
    for( const auto& chPair : boardPair.second ){
      unsigned short channelID = chPair.first;
      unsigned short dID = chPair.second;
      if( dID == detID ){
        std::pair<unsigned short, unsigned short> vmeDig = FindVMEDIGFromBoardID(boardID);
        vmeDigCh.push_back(vmeDig.first);  // VME
        vmeDigCh.push_back(vmeDig.second); // DIG
        vmeDigCh.push_back(channelID);      // CH
        printf("Found DetID %2hu => VME: %2hu, DigID: %2hu, ChannelID: %3hu\n", detID, vmeDig.first, vmeDig.second, channelID);
      }
    }
  }
  return vmeDigCh;
}


#endif // MISC_H