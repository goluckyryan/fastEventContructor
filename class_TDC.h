#ifndef TDC_HIT_H
#define TDC_HIT_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <cmath>

enum TACTrash{
  Valid = 0,
  NoTrigger = 1,
  TDCoffsetInvalid = 2,
  VernierInvalid = 3
};

class TDC {
public:
  uint64_t timestampTrig;  // 64-bit
  uint64_t timestampTDC;
  uint32_t coarseTime;
  int tdcOffset; // in ns

  uint32_t trigType;
  uint32_t wheel;
  uint32_t multiplicity;
  uint32_t userRegister;
  uint32_t triggerBitMask;

  std::vector<uint32_t> fourNanoSecCounter;
  uint32_t vernierAB;
  uint32_t vernierCD;

  // calculated values
  uint32_t validBit;
  uint64_t baseTime;
  int vernier[4];
  int valid[4];
  double phaseTime[4];
  double avgPhaseTime;

  TACTrash isTrashData = TACTrash::Valid;

  std::vector<uint32_t> payload;
  int offset[4] = {0, 1, 2, 3};

  TDC() : timestampTrig(0), timestampTDC(0), coarseTime(0),
      trigType(0), wheel(0), multiplicity(0), userRegister(0), triggerBitMask(0),
      vernierAB(0), vernierCD(0), validBit(0), baseTime(0), avgPhaseTime(0) {
    fourNanoSecCounter.resize(4, 0);
    std::fill(vernier, vernier + 4, 0);
    std::fill(valid, valid + 4, 0);
    std::fill(phaseTime, phaseTime + 4, 0.0);
  }

  void printPayload() const {
    std::cout << "TAC-II payload:" << std::endl;
    for (size_t i = 0; i < payload.size(); ++i) {
      std::cout << "Word[" << std::setw(2) << std::setfill('0') << i << "] : 0x"
            << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
            << payload[i] << std::dec << std::endl;
    }
  }

  void TransformPackedDataToPayload(const std::vector<uint32_t>& packedData){
    payload.clear();
    payload.push_back(0x0000AAAA);             // 0 Fixed value
    payload.push_back(packedData[4] >> 16);    // 1 trigType
    payload.push_back(packedData[3] & 0xFFFF); // 2 timestamp Hight
    payload.push_back(packedData[2] >> 16);    // 3
    payload.push_back(packedData[2] & 0xFFFF); // 4 timestamp Low
    payload.push_back(packedData[4] & 0xFFFF); // 5 wheel
    payload.push_back(packedData[5] >> 16);    // 6 multiplicity
    payload.push_back(packedData[5] & 0xFFFF); // 7 userRegister
    payload.push_back(packedData[6] >> 16);    // 8 coarseTime
    payload.push_back(packedData[6] & 0xFFFF); // 9 triggerBitMask
    payload.push_back(packedData[7] >> 16);    // 10 fourNanoSecCounter[0]
    payload.push_back(packedData[7] & 0xFFFF); // 11 fourNanoSecCounter[1]
    payload.push_back(packedData[8] >> 16);    // 12 fourNanoSecCounter[2]
    payload.push_back(packedData[8] & 0xFFFF); // 13 fourNanoSecCounter[3]
    payload.push_back(packedData[9] >> 16);    // 14 vernierAB
    payload.push_back(packedData[9] & 0xFFFF); // 15 vernierCD
  } 

  void DecodePackedData(const std::vector<uint32_t>& packedData) {
    // payload = packedData;

    // if (payload.size() < 10) return; // basic safety check

    // timestampTrig = payload[2] + (static_cast<uint64_t>(payload[3] & 0x0000FFFF) << 32);
    // coarseTime = payload[6] >> 16;
    // timestampTDC = (timestampTrig & 0xFFFFFFFF0000ULL) + coarseTime;

    // timestampTrig *= 10; // convert to ns
    // timestampTDC *= 10;  // convert to ns

    // trigType = payload[4] >> 16;
    // wheel = payload[4] & 0xFFFF;
    // multiplicity = payload[5] >> 16;
    // userRegister = payload[5] & 0xFFFF;
    // triggerBitMask = payload[6] & 0xFFFF;

    // fourNanoSecCounter.clear();
    // fourNanoSecCounter.push_back((payload[7] >> 16) & 0xFFFF);
    // fourNanoSecCounter.push_back(payload[7] & 0xFFFF);
    // fourNanoSecCounter.push_back((payload[8] >> 16) & 0xFFFF);
    // fourNanoSecCounter.push_back(payload[8] & 0xFFFF);

    // vernierAB = payload[9] >> 16;
    // vernierCD = payload[9] & 0xFFFF;

    if( packedData[9] == 0x10021001 ) {
      isTrashData = TACTrash::NoTrigger;
      return;
    }

    TransformPackedDataToPayload(packedData);
    DecodePayload(payload);
  }

  void DecodePayload(const std::vector<uint32_t>& pl) {
    payload = pl;

    if (payload.size() < 16) return; // basic safety check

    timestampTrig = (static_cast<uint64_t>(payload[2]) << 32) +
                    (static_cast<uint64_t>(payload[3]) << 16) +
                    payload[4];

    coarseTime = payload[8];
    timestampTDC = (timestampTrig & 0xFFFFFFFF0000ULL) + coarseTime;

    timestampTrig *= 10; // convert to ns
    timestampTDC *= 10;  // convert to ns

    tdcOffset = static_cast<int>(timestampTDC - timestampTrig);
    if( tdcOffset < 0 || tdcOffset > 200 ) {
      isTrashData = TACTrash::TDCoffsetInvalid;
      return;
    }

    trigType = payload[1];
    wheel = payload[5];
    multiplicity = payload[6];
    userRegister = payload[7];
    triggerBitMask = payload[9];

    fourNanoSecCounter.clear();
    for (int i = 10; i < 14; ++i) fourNanoSecCounter.push_back(payload[i]);

    vernierAB = payload[14];
    vernierCD = payload[15];

    CalTime();
   
  }

  void CalTime(){

    validBit = vernierAB >> 12;
    if( validBit == 0 ) {
      isTrashData = VernierInvalid;
      return;
    }

    vernier[0] = (vernierAB >> 6) & 0x3F;
    vernier[1] = vernierAB & 0x3F;
    vernier[2] = (vernierCD >> 6) & 0x3F;
    vernier[3] = vernierCD & 0x3F;


    baseTime = timestampTrig - (timestampTrig % 262144); // each coarse time period is 2^18 * 10 ns = 2621440 ns

    std::fill(valid, valid + 4, 0);
    std::fill(phaseTime, phaseTime + 4, 0.0);
    int validCount = 0;
    avgPhaseTime = 0.0;

    for (int i = 0; i < 4; ++i) {
      if ((validBit >> i) & 0x1) {
        valid[i] = 1;
        validCount++;
        phaseTime[i] = baseTime + fourNanoSecCounter[i] * 4 + offset[i] - (vernier[i] * 0.05);
        avgPhaseTime += phaseTime[i];
      } else {
        valid[i] = 0;
        phaseTime[i] = 0.0;
      }
    }

    if (validCount > 0) {
      avgPhaseTime /= validCount;
    }
  }

  void print() const {
    printf("--------------------------  TAC-II data --------------------------\n");
    printf("MTRGtimestamp : 0x%012lX x 10 = %lu ns\n", (timestampTrig / 10), timestampTrig);
    printf("    Coarse TS : 0x%012X\n", coarseTime);
    printf(" TDCtimestamp : 0x%012lX x 10 = %lu ns\n", (timestampTDC / 10), timestampTDC);

    double timeDiff = 0;
    bool rollFlag = false;
    if (timestampTDC < timestampTrig) {
      timeDiff = timestampTDC + 0x10000 * 10 - timestampTrig;
      rollFlag = true;
    } else {
      timeDiff = timestampTDC - timestampTrig;
    }

    printf("abs time diff : %.0f ns%s\n", timeDiff, rollFlag ? " (rolled)" : "");
    printf("     trigType : 0x%04X\n", trigType);
    printf("        wheel : 0x%04X\n", wheel);
    printf("         User : 0x%04X\n", userRegister);
    printf("      trigger : 0x%04X\n", triggerBitMask);

    for (int i = 0; i < 4; ++i) {
      printf("  TDC 4ns (%d) : 0x%04X x 4 ns = %u\n", i, fourNanoSecCounter[i], fourNanoSecCounter[i] * 4);
    }

    printf("   Vernier AB : 0x%04X\n", vernierAB);
    printf("   Vernier CD : 0x%04X\n", vernierCD);
    printf("--------------------\n");
    for (int i = 0; i < 4; ++i) {
      printf("Vernier-%d : 0x%02X = %d\n", i, vernier[i], vernier[i]);
    }

    printf(" avg Phase Time : %.3f ns\n", avgPhaseTime);
    printf(" avg Phase Time - TrigTime : %.3f ns\n", avgPhaseTime - static_cast<double>(timestampTrig));

    printf("==================================================================\n");
  }
};


#endif