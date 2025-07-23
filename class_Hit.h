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


class HIT {
public:
  GEBHeader header;  // Header information
  std::vector<uint32_t> payload;  // Payload data

  unsigned short UniqueID; // Unique ID for the hit, combining DigID and channel

  void Print() const {
    header.Print();
    printf("  Payload size: %zu words\n", payload.size());
    unsigned short header_type   = (ntohl(payload[2]) >> 16) & 0xF; // for data type
    uint16_t packet_length = (ntohl(payload[0]) >> 16) & 0x7FF;
    printf("  header type : %u\n", header_type);
    printf("packet length : %u words\n", packet_length);
    for (size_t i = 0; i < payload.size(); ++i) {
      printf("%3zu: 0x%08X\n", i, ntohl(payload[i]));
    }
  }

  DIG DecodePayload(){
    DIG digHit;

    digHit.DecodeData(payload);
    digHit.UniqueID = UniqueID; // Set the UniqueID from the argument

    return digHit;
  }


};

#endif