#ifndef HIT_H
#define HIT_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <arpa/inet.h>

#include "class_DIG.h"
#include "class_TDC.h"

class GEBHeader{
public:
  uint32_t type;
  uint32_t payload_lenght_byte;
  uint64_t timestamp;

  void Print() const{
    printf("           type : 0x%08X = %d\n", type, type);
    printf(" payload lenght : 0x%08X = %d bytes\n", payload_lenght_byte, payload_lenght_byte);
    printf("      timestamp : 0x%016lX = %lu\n", timestamp, timestamp);
  }
};

#define TRASH_DATA 666

class HIT {
public:
  GEBHeader gebHeader;  // Header information
  std::vector<uint32_t> payload;  // Payload data
  unsigned short UniqueID; // Unique ID for the hit, DigID * 100 + channel, from BinaryReader

  void Print() const {
    gebHeader.Print();
    printf("  Payload size: %zu words\n", payload.size());

    if (gebHeader.type != 99){
      unsigned short header_type   = (ntohl(payload[2]) >> 16) & 0xF; // for data type
      uint16_t packet_length = (ntohl(payload[0]) >> 16) & 0x7FF;
      printf("  gebHeader type : %u\n", header_type);
      printf("packet length : %u words\n", packet_length);
      for (size_t i = 0; i < payload.size(); ++i) {
        printf("%3zu: 0x%08X\n", i, ntohl(payload[i]));
      }
    }else{
      for (size_t i = 0; i < payload.size(); ++i) {
        printf("%3zu: 0x%08X\n", i, payload[i]);
      }
    }
  }

  void ConstructGEBHeaderTimestampFromTACPayload(){
    gebHeader.type = 99; // TDC data type
    gebHeader.payload_lenght_byte = 40; // 10 words
    gebHeader.timestamp = (static_cast<uint64_t>(payload[3] & 0x0000FFFF) << 32) + payload[2];
    // gebHeader.Print();
    // Print();
  } 

  DIG DecodePayload(bool withTrace = false) {
    DIG digHit;

    digHit.UniqueID = UniqueID; // Set the UniqueID from the argument

    if( gebHeader.type == 99 ){
      TDC tdcHit;
      tdcHit.DecodePackedData(payload);

      if( tdcHit.isTrashData ) {
        // printf(" Trash TDC data found for UniqueID: %u\n", UniqueID);
        digHit.HEADER_TYPE = TRASH_DATA;
        return digHit; // return empty digHit
      }

      // fill teh digHit with TDC data
      digHit.CH_ID = 99;
      digHit.USER_DEF = 99;
      digHit.HEADER_TYPE = 99;
      digHit.EVENT_TYPE = tdcHit.trigType;

      double tdc_avg_time = tdcHit.avgPhaseTime; // in ns

      //TODO to add offset correction here if needed
      uint64_t offset = 0; // in 10 ns unit

      digHit.EVENT_TIMESTAMP = static_cast<uint64_t>(tdc_avg_time / 10.0); // in 10 ns unit
      digHit.EVENT_TIMESTAMP += offset;

      digHit.PRE_RISE_ENERGY = static_cast<uint32_t>((tdc_avg_time - (digHit.EVENT_TIMESTAMP * 10)) * 1000); 

      // printf(" tdc_avg_time: %.3f ns, EVENT_TIMESTAMP: %lu, PRE_RISE_ENERGY: %u \n", tdc_avg_time, digHit.EVENT_TIMESTAMP, digHit.PRE_RISE_ENERGY);

    }else{
      digHit.DecodeData(payload, withTrace);
    }

    // if( (ntohl(payload[2]) >> 16) & 0xF > 8 ){
    //   printf(" Decoded DIG data for UniqueID: %u\n", UniqueID);
    //   for( int i  = 0; i < payload.size(); i++) printf("  payload[%2d] = 0x%08X\n", i, ntohl(payload[i]));
    // }

    return digHit;
  }


};

#endif