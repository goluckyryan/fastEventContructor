#ifndef DIG_HIT_H
#define DIG_HIT_H

#include <stdio.h> /// for FILE
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <time.h> // time in nano-sec
#include <stdint.h>
#include <arpa/inet.h>
#include <stdexcept>


class DIG_Hit { // for DIG data header type 7 or 8
public:
  // unsigned int FIXED_AAAAAAAA; //Word 0
  unsigned int CH_ID;          //Word 1 (3:0)
  unsigned int USER_DEF;       //Word 1 (15:4)
  unsigned int PACKET_LENGTH;  //Word 1 (26:16)
  unsigned int GEO_ADDR;       //Word 1 (31:27)

  uint64_t EVENT_TIMESTAMP;    //Word 2 and Word 3 (15:0)

  unsigned int HEADER_TYPE;           //Word 3 (19:16)
  bool PEQ_BYPASS;                    //Word 3 (20)
  bool TRIG_TS_MODE;                  //Word 3 (21)
  bool CFD_ESUM_MODE;                 //Word 3, bit 22; new item added 20210817
  unsigned int EVENT_TYPE;            //Word 3 (25:23) (trigger type code from bits 10:8 of 1st word in trigger accept message)
  unsigned int HEADER_LENGTH;         //Word 3 (31:26)

  bool EARLY_PRE_RISE_SELECT; //Word 4 (bit 4)
  bool WRITE_FLAGS;           //Word 4 (bit 5)
  bool VETO_FLAG;             //Word 4 (bit 6)
  bool EXTERNAL_DISC_FLAG;    //Word 4 (bit 8)
  bool PEAK_VALID_FLAG;       //Word 4 (bit 9)
  bool OFFSET_FLAG;           //Word 4 (bit 10)
  bool PILEUP_ONLY_FLAG;      //Word 4 (bit 14)
  bool PILEUP_FLAG;           //Word 4 (bit 15)
    
  
  unsigned int SAMPLED_BASELINE;    //Word 6 (23:0)
  unsigned short PILEUP_COUNT;  //Word 6 (27:24) in LED mode, Word 7 (15:14) and Word 7 (31:31) in CFD mode.
  
  
  unsigned int PRE_RISE_ENERGY;       //Word 8 (23:0)
  unsigned int POST_RISE_ENERGY;      //Word 9 (15:0) AND Word 8 (31:24)
  unsigned short PEAK_TIMESTAMP;      //Word 9 (31:16)
  
  unsigned short P2_SUM;            //Word 10 (13:0) and word 13 (9:0)
  bool P2_MODE;                    //Word 10 (14)
  bool CAPTURE_PARST_TS;           //Word 10 (15)
  unsigned short TS_OF_TRIGGER;    //Word 10 (31:16)
  
  unsigned int MULTIPLEX_DATA;         //Word 11 (23:0)  (multiplex data field)
  unsigned int LAST_POST_RISE_M_SUM;   //Word 11 (31:24) AND Word 12 (31:24) AND Word 13 (31:24)
  
  unsigned int EARLY_PRE_RISE_ENERGY;  //Word 12 (23:0)
  
  bool SECOND_THRESH_DISC_FLAG; //Word 13(10)
  bool PARST_TSM;               //Word 13(11)
  bool COARSE_FIRED;            //Word 13(13)
  unsigned short TS_OF_COARSE;  //Word 13(23:14)
  
  //LED only 
  unsigned short TRIG_MON_XTRA_DATA; //Word 7 (15:0), LED mode only.  
  
  
  //CFD mode only  
  bool TIMESTAMP_MATCH_FLAG;  //Word 4 (bit 7, only when in CFD mode)
  bool CFD_VALID_FLAG;        //Word 4 (bit 11, only when in CFD mode) 
  
  short int CFD_SAMPLE_0;  //Word 5 (29:16), CFD mode only  
  short int CFD_SAMPLE_1;  //Word 7,(13:0,CFD mode only) (Invalid in LED mode)
  short int CFD_SAMPLE_2;  //Word 7,(29:16,CFD mode only) (Invalid in LED mode)

  bool PREVIOUS_CFD_VALID;      //Word 13(12)
  
  // shared by LED and CFD, but they are different in raw data
  uint64_t LAST_DISC_TIMESTAMP; // LED = 48 bit value, CFD = 30 bit value.
  unsigned short TRIG_MON_DET_DATA; 

  //---- type 1  or 2
  bool SYNC_ERROR_FLAG;
  bool GENERAL_ERROR_FLAG;
  unsigned short M1_BEGIN_SAMPLE; 
  unsigned short M1_END_SAMPLE;   
  unsigned short M2_BEGIN_SAMPLE; 
  unsigned short M2_END_SAMPLE;   
  unsigned short PEAK_SAMPLE;     
  unsigned short BASE_SAMPLE;     
  
  //---- type 3 oe 4
  unsigned short TS_of_Trigger;         
  unsigned short LAST_POST_RISE_SAMPLE2;
  unsigned short POST_RISE_SAMPLE;      
  unsigned short LAST_POST_RISE_SAMPLE; 
  unsigned short PRE_RISE_ENTER_SAMPLE; 
  unsigned short PRE_RISE_LEAVE_SAMPLE; 

  //---- type 5 or 6
  bool PU_TIME_ERROR; //Word 4 (bit 4)
  unsigned short POST_RISE_ENTER_SAMPLE; //Word 12 (13:0)
  unsigned short POST_RISE_LEAVE_SAMPLE; //Word 12 (29:16)

    void Print(){

    printf("############################################ DIG_Hit:\n");
    printf("  CH_ID                       : %u\n", CH_ID);
    printf("  USER_DEF                    : %u\n", USER_DEF);
    printf("  PACKET_LENGTH               : %u\n", PACKET_LENGTH);
    printf("  GEO_ADDR                    : %u\n", GEO_ADDR);
    printf("  HEADER_TYPE                 : %u\n", HEADER_TYPE);
    printf("  EVENT_TYPE                  : %u\n", EVENT_TYPE);
    printf("  HEADER_LENGTH (Byte)        : %u\n", HEADER_LENGTH);

    printf("-------------------------------------------\n");
    printf("  EVENT_TIMESTAMP             : %lu\n", EVENT_TIMESTAMP);
    printf("  TS_OF_TRIGGER (16bit)       : %hu\n", TS_OF_TRIGGER);
    printf("  PEAK_TIMESTAMP (16bit)      : %hu\n", PEAK_TIMESTAMP);

    uint64_t peak_ts = EVENT_TIMESTAMP & 0xFFFFFFFF0000 + PEAK_TIMESTAMP;
    if( peak_ts < EVENT_TIMESTAMP ) peak_ts += 0x10000; // handle roll over
    printf("                              : %lu\n", peak_ts);
    printf("  TS_OF_COARSE (10bit)        : %hu\n", TS_OF_COARSE);
    printf("  LAST_DISC_TIMESTAMP (%dbit) : %lu\n", HEADER_TYPE == 7 ? 48 : 30, LAST_DISC_TIMESTAMP);
    printf("\n");
    printf("  SAMPLED_BASELINE            : %u\n", SAMPLED_BASELINE);
    printf("  PRE_RISE_ENERGY             : %u\n", PRE_RISE_ENERGY);
    printf("  POST_RISE_ENERGY            : %u\n", POST_RISE_ENERGY);
    if( HEADER_TYPE == 8 ){
      printf("  TIMESTAMP_MATCH_FLAG    : %u\n", TIMESTAMP_MATCH_FLAG);
      printf("  CFD_VALID_FLAG          : %u\n", CFD_VALID_FLAG);
      printf("  PREVIOUS_CFD_VALID      : %hu\n", PREVIOUS_CFD_VALID);
      printf("  CFD_SAMPLE_0            : %d\n", CFD_SAMPLE_0);
      printf("  CFD_SAMPLE_1            : %d\n", CFD_SAMPLE_1);
      printf("  CFD_SAMPLE_2            : %d\n", CFD_SAMPLE_2);
    }
    printf("-------------------------------------------\n");
    
    printf("  PEQ_BYPASS                  : %u\n", PEQ_BYPASS);
    printf("  TRIG_TS_MODE                : %u\n", TRIG_TS_MODE);
    printf("  CFD_ESUM_MODE               : %u\n", CFD_ESUM_MODE);
    printf("  EARLY_PRE_RISE_SELECT       : %u\n", EARLY_PRE_RISE_SELECT);
    printf("  WRITE_FLAGS                 : %u\n", WRITE_FLAGS);
    printf("  VETO_FLAG                   : %u\n", VETO_FLAG);
    printf("  EXTERNAL_DISC_FLAG          : %u\n", EXTERNAL_DISC_FLAG);
    printf("  PEAK_VALID_FLAG             : %u\n", PEAK_VALID_FLAG);
    printf("  OFFSET_FLAG                 : %u\n", OFFSET_FLAG);
    printf("  PILEUP_ONLY_FLAG            : %u\n", PILEUP_ONLY_FLAG);
    printf("  PILEUP_FLAG                 : %u\n", PILEUP_FLAG);
    printf("  PILEUP_COUNT                : %u\n", PILEUP_COUNT);
    printf("  SECOND_THRESH_DISC_FLAG     : %u\n", SECOND_THRESH_DISC_FLAG);
    printf("  PARST_TSM                   : %u\n", PARST_TSM);
    printf("  COARSE_FIRED                : %u\n", COARSE_FIRED);
    
    printf("-------------------------------------------\n");
    if( HEADER_TYPE == 7 ){
      printf("  TRIG_MON_XTRA_DATA          : %hu\n", TRIG_MON_XTRA_DATA);
    }
    printf("  TRIG_MON_DET_DATA           : %u\n", TRIG_MON_DET_DATA);
    printf("  P2_MODE                     : %u\n", P2_MODE);
    printf("  P2_SUM                      : %u\n", P2_SUM);
    printf("  CAPTURE_PARST_TS            : %u\n", CAPTURE_PARST_TS);    
    printf("  MULTIPLEX_DATA              : %u\n", MULTIPLEX_DATA);
    printf("  LAST_POST_RISE_M_SUM        : %u\n", LAST_POST_RISE_M_SUM);
    printf("  EARLY_PRE_RISE_ENERGY       : %u\n", EARLY_PRE_RISE_ENERGY);
    
    printf("============================================\n");

  }

  void DecodeData(std::vector<uint32_t> payload){

    unsigned short header_type   = (ntohl(payload[2]) >> 16) & 0xF; // for data type

    unsigned int raw_header[14];
    for( int i = 0; i < 13; i++) raw_header[i+1] = ntohl(payload[i]);

    switch(header_type) {
      case 1: // Old LED header
      case 2: // Old CFD header
        DecodeOldHeader_1_2(raw_header);
        break;
      case 3: // New LED header
      case 4: // New CFD header
        DecodeHeader_3_4(raw_header);
        break;
      case 5: // LED with Pileup
      case 6: // CFD with Pileup
        DecodeHeader_5_6(raw_header);
        break;
      case 7: // DIG LED header
      case 8: // DIG CFD header
        DecodeHeader_7_8(raw_header);
        break;
      default:
        throw std::runtime_error("\033[31mUnknown header type: " + std::to_string(header_type) + "\033[0m");
    }


  }

private:
  bool ExtractBit(unsigned int value, unsigned int bitPosition) {
    if (bitPosition > 31) {
      return false; // Invalid bit position
    }
    return (value >> bitPosition) & 0x1; // Extract the bit at the specified position
  }

  void DecodeOldHeader_1_2(unsigned int Raw_Header[14]){

    // First decode the generic part of the header.
    //FIXED_AAAAAAAA  = (Raw_Header[0] & 0xFFFFFFFF) >> 0;            // Word 0: 31..0
    CH_ID            = (Raw_Header[1] & 0x0000000F) >> 0;             // Word 1: 3..0
    USER_DEF         = (Raw_Header[1] & 0x0000FFF0) >> 4;             // Word 1: 15..4
    PACKET_LENGTH    = (Raw_Header[1] & 0x07FF0000) >> 16;            // Word 1: 26..16
    GEO_ADDR         = (Raw_Header[1] & 0xF8000000) >> 27;            // Word 1: 31..27
    EVENT_TIMESTAMP  = ((uint64_t)(Raw_Header[2] & 0xFFFFFFFF)) >> 0 |   // Word 2: 31..0 & 
                          ((uint64_t)(Raw_Header[3] & 0x0000FFFF)) << 32;  // Word 3: 15..0 
    HEADER_TYPE      = (Raw_Header[3] & 0x000F0000) >> 16;            // Word 3: 19..16
    EVENT_TYPE       = (Raw_Header[3] & 0x03800000) >> 23;            // Word 3: 25..23
    HEADER_LENGTH    = (Raw_Header[3] & 0xFC000000) >> 26;            // Word 3: 31..26

    //Header type 1 is the old LED format.  Header type 2 is the old CFD format.
    switch(HEADER_TYPE){  
      case 1:    // "Standard" LED Header used from 2013 to July 2015
        WRITE_FLAGS           = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG             = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG  = 0xFFFF;  //bit invalid for LED mode
        EXTERNAL_DISC_FLAG    = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG       = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG           = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG        = 0xFFFF;  //bit invalid for LED mode
        SYNC_ERROR_FLAG       = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG    = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG      = (Raw_Header[4] & 0x00004000) >> 14;              // Word 4: 14
        PILEUP_FLAG           = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP   = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                ((uint64_t)(Raw_Header[5] & 0xFFFFFFFF)) << 16;  // Word 5: 13..0       
        CFD_SAMPLE_0          = 0;  //value invalid for LED mode
        SAMPLED_BASELINE      = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0
        CFD_SAMPLE_1          = 0;  //value invalid for LED mode
        CFD_SAMPLE_2          = 0;  //value invalid for LED mode
        PRE_RISE_ENERGY       = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY      = (Raw_Header[8] & 0xFF000000) >> 28 |            // Word 8: 31..24 & 
                                  (Raw_Header[9] & 0x0000FFFF) << 8;            // Word 9: 15..0  
        PEAK_TIMESTAMP        = ((uint64_t)(Raw_Header[ 9] & 0xFFFF0000)) >> 16 |  // Word 9: 31..16 & 
                                ((uint64_t)(Raw_Header[10] & 0xFFFFFFFF)) << 16;  // Word 10: 31..0 
        M1_BEGIN_SAMPLE  = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        M1_END_SAMPLE    = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        M2_BEGIN_SAMPLE  = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        M2_END_SAMPLE    = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE      = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE      = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16      
      break;

      case 2:    // "Standard" CFD Header used from 2013 to July 2015
        WRITE_FLAGS       = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG      = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG  = (Raw_Header[4] & 0x00000080) >> 7;               // Word 4: 7
        EXTERNAL_DISC_FLAG  = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG    = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG      = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG    = (Raw_Header[4] & 0x00000800) >> 11;              // Word 4: 11
        SYNC_ERROR_FLAG    = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG  = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG    = (Raw_Header[4] & 0x00000800) >> 14;              // Word 4: 11
        PILEUP_FLAG      = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP  = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                ((uint64_t)(Raw_Header[5] & 0x00003FFF)) << 16;  // Word 5: 13..0    
        CFD_SAMPLE_0      = (Raw_Header[5] & 0x3FFF0000) >> 16;              // Word 5: 29..16     
        SAMPLED_BASELINE    = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0
        CFD_SAMPLE_1      = (Raw_Header[7] & 0x00003FFF) >> 0;              // Word 7: 13..0
        CFD_SAMPLE_2      = (Raw_Header[7] & 0x3FFF0000) >> 16;              // Word 7: 29..16
        PRE_RISE_ENERGY    = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY    = (Raw_Header[8] & 0xFF000000) >> 28 |            // Word 8: 31..24 & 
                                  (Raw_Header[9] & 0x0000FFFF) << 8;            // Word 9: 15..0 
        PEAK_TIMESTAMP    = ((uint64_t)(Raw_Header[ 9] & 0xFFFF0000)) >> 16 |  // Word 9: 31..16 & 
                                ((uint64_t)(Raw_Header[10] & 0xFFFFFFFF)) << 16;  // Word 10: 31..0  
        M1_BEGIN_SAMPLE    = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        M1_END_SAMPLE    = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        M2_BEGIN_SAMPLE    = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        M2_END_SAMPLE    = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE      = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE      = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16      
      break;
      default: break;
    }
    return;
  }  

  // this function decodes the header types used up until May 2018.
  void DecodeHeader_3_4(unsigned int Raw_Header[14]){
    // FIXED_AAAAAAAA  = (Raw_Header[0] & 0xFFFFFFFF) >> 0;            // Word 0: 31..0
    CH_ID         = (Raw_Header[1] & 0x0000000F) >> 0;            // Word 1: 3..0
    USER_DEF       = (Raw_Header[1] & 0x0000FFF0) >> 4;            // Word 1: 15..4
    PACKET_LENGTH  = (Raw_Header[1] & 0x07FF0000) >> 16;            // Word 1: 26..16
    GEO_ADDR      = (Raw_Header[1] & 0xF8000000) >> 27;              // Word 1: 31..27
    EVENT_TIMESTAMP  = ((uint64_t)(Raw_Header[2] & 0xFFFFFFFF)) >> 0 |   // Word 2: 31..0 & 
                  ((uint64_t)(Raw_Header[3] & 0x0000FFFF)) << 32;        // Word 3: 15..0 
    HEADER_TYPE    = (Raw_Header[3] & 0x000F0000) >> 16;            // Word 3: 19..16
    EVENT_TYPE    = (Raw_Header[3] & 0x03800000) >> 23;            // Word 3: 25..23
    HEADER_LENGTH  = (Raw_Header[3] & 0xFC000000) >> 26;            // Word 3: 31..26

    //Header type 3 is the LED format.  Header type 4 is the CFD format.
    switch(HEADER_TYPE){  
      case 3:    // Updated LED Header as in firmware of 20161222.
        
        WRITE_FLAGS         = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG        = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG    = 0xFFFF;  //bit invalid for LED mode
        EXTERNAL_DISC_FLAG    = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG      = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG        = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG      = 0xFFFF;  //bit invalid for LED mode
        SYNC_ERROR_FLAG      = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG    = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG      = (Raw_Header[4] & 0x00004000) >> 14;              // Word 4: 14
        PILEUP_FLAG        = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP    = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                  ((uint64_t)(Raw_Header[5] & 0xFFFFFFFF)) << 16;  // Word 5: 31..0       
        CFD_SAMPLE_0        = 0;  //value invalid for LED mode
        SAMPLED_BASELINE      = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0
        CFD_SAMPLE_1        = 0;  //value invalid for LED mode
        CFD_SAMPLE_2        = 0;  //value invalid for LED mode
        
        PRE_RISE_ENERGY      = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY      = (Raw_Header[8] & 0xFF000000) >> 28 |            // Word 8: 31..24 & 
                                    (Raw_Header[9] & 0x0000FFFF) << 8;            // Word 9: 15..0 
        PEAK_TIMESTAMP          = (Raw_Header[9] & 0xFFFF0000) >> 16;            // Word 9: 31..16
        TS_of_Trigger           = (Raw_Header[10] & 0xFFFF0000) >> 16;            // Word 10  31:16
        LAST_POST_RISE_SAMPLE2  = (unsigned short)((Raw_Header[10] & 0x00003FFF) << 0);    // Word 10: 15..0  
        POST_RISE_SAMPLE        = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        LAST_POST_RISE_SAMPLE   = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        PRE_RISE_ENTER_SAMPLE   = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        PRE_RISE_LEAVE_SAMPLE   = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE             = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE             = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16      
        break;

      case 4:    // CFD Header updated to match firmware as of release 20161222
        WRITE_FLAGS         = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG        = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG    = (Raw_Header[4] & 0x00000080) >> 7;               // Word 4: 7
        EXTERNAL_DISC_FLAG    = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG      = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG        = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG      = (Raw_Header[4] & 0x00000800) >> 11;              // Word 4: 11
        SYNC_ERROR_FLAG      = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG    = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG      = (Raw_Header[4] & 0x00000800) >> 14;              // Word 4: 11
        PILEUP_FLAG        = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP    = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                  ((uint64_t)(Raw_Header[5] & 0x00003FFF)) << 16;  // Word 5: 13..0    
        CFD_SAMPLE_0        = (Raw_Header[5] & 0x3FFF0000) >> 16;              // Word 5: 29..16     
        SAMPLED_BASELINE      = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0
        CFD_SAMPLE_1        = (Raw_Header[7] & 0x00003FFF) >> 0;              // Word 7: 13..0
        CFD_SAMPLE_2        = (Raw_Header[7] & 0x3FFF0000) >> 16;              // Word 7: 29..16

        PRE_RISE_ENERGY      = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY      = (Raw_Header[8] & 0xFF000000) >> 28 |            // Word 8: 31..24 & 
                                    (Raw_Header[9] & 0x0000FFFF) << 8;                // Word 9: 15..0 
        PEAK_TIMESTAMP      = (Raw_Header[9] & 0xFFFF0000) >> 16;            // Word 9: 31..16
        TS_of_Trigger      = (Raw_Header[10] & 0xFFFF0000) >> 16;            // Word 10  31:16
        LAST_POST_RISE_SAMPLE2  = (unsigned short)((Raw_Header[10] & 0x00003FFF) << 0);    // Word 10: 15..0  
        POST_RISE_SAMPLE      = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        LAST_POST_RISE_SAMPLE  = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        PRE_RISE_ENTER_SAMPLE  = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        PRE_RISE_LEAVE_SAMPLE  = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE        = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE        = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16      

        break;
      default: break;  
    }
    return;
  }

  // this function decodes the header types used after May 1 2018.
  void DecodeHeader_5_6(unsigned int Raw_Header[14]){
    //FIXED_AAAAAAAA  = (Raw_Header[0] & 0xFFFFFFFF) >> 0;            // Word 0: 31..0
    CH_ID            = (Raw_Header[1] & 0x0000000F) >> 0;            // Word 1: 3..0
    USER_DEF         = (Raw_Header[1] & 0x0000FFF0) >> 4;            // Word 1: 15..4
    PACKET_LENGTH    = (Raw_Header[1] & 0x07FF0000) >> 16;            // Word 1: 26..16
    GEO_ADDR         = (Raw_Header[1] & 0xF8000000) >> 27;            // Word 1: 31..27
    EVENT_TIMESTAMP  = ((uint64_t)(Raw_Header[2] & 0xFFFFFFFF)) >> 0 |   // Word 2: 31..0 & 
                          ((uint64_t)(Raw_Header[3] & 0x0000FFFF)) << 32;  // Word 3: 15..0 
    HEADER_TYPE    = (Raw_Header[3] & 0x000F0000) >> 16;            // Word 3: 19..16
    EVENT_TYPE     = (Raw_Header[3] & 0x03800000) >> 23;            // Word 3: 25..23
    TRIG_TS_MODE   = ExtractBit(Raw_Header[3], 21);                // Word 3 (21)
    PEQ_BYPASS     = ExtractBit(Raw_Header[3], 20);                // Word 3 (20)  
    
    
    HEADER_LENGTH  = (Raw_Header[3] & 0xFC000000) >> 26;            // Word 3: 31..26

    //Header type 5 is the LED format.  Header type 6 is the CFD format.
    switch(HEADER_TYPE){  
      case 5:    // Updated LED Header as in firmware of 20180501.
        
        PU_TIME_ERROR        = ExtractBit(Raw_Header[4], 4);                 // Word 4: 4
        WRITE_FLAGS          = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG            = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG = 0xFFFF;              //only valid for CFD     Word 4: 7
        EXTERNAL_DISC_FLAG   = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG      = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG          = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG       = 0xFFFF;              //only valid for CFD  // Word 4: 11
        SYNC_ERROR_FLAG      = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG   = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG     = (Raw_Header[4] & 0x00004000) >> 14;              // Word 4: 14
        PILEUP_FLAG          = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP  = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                  ((uint64_t)(Raw_Header[5] & 0xFFFFFFFF)) << 16;  // Word 5: 31..0       
        CFD_SAMPLE_0        =  0;                //only valid for CFD  // Word 5: 29:16
        
        SAMPLED_BASELINE    = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0

        TRIG_MON_XTRA_DATA  = (Raw_Header[7] & 0x0000FFFF);                // Word 7: 15..0
        CFD_SAMPLE_1        =  0;                //only valid for CFD  // Word 7: 13:0
        TRIG_MON_DET_DATA   = (Raw_Header[7] & 0xFFFF0000) >> 16;            // Word 7: 31..16
        CFD_SAMPLE_2        =  0;                //only valid for CFD  // Word 7: 29:16
        
        PRE_RISE_ENERGY      = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY     = (Raw_Header[8] & 0xFF000000) >> 24 |            // Word 8: 31..24 & 
                                    (Raw_Header[9] & 0x0000FFFF) << 8;            // Word 9: 15..0 
        PEAK_TIMESTAMP          = (Raw_Header[9] & 0xFFFF0000) >> 16;            // Word 9: 31..16
        TS_OF_TRIGGER           = (Raw_Header[10] & 0xFFFF0000) >> 16;            // Word 10  31:16
        POST_RISE_LEAVE_SAMPLE  = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        POST_RISE_ENTER_SAMPLE  = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        PRE_RISE_ENTER_SAMPLE   = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        PRE_RISE_LEAVE_SAMPLE   = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE             = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE             = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16      
        break;

      case 6:    // CFD Header updated to match firmware 20180501
        
        PU_TIME_ERROR        = ExtractBit(Raw_Header[4], 4);                 // Word 4: 4
        WRITE_FLAGS          = (Raw_Header[4] & 0x00000020) >> 5;               // Word 4: 5
        VETO_FLAG            = (Raw_Header[4] & 0x00000040) >> 6;               // Word 4: 6
        TIMESTAMP_MATCH_FLAG = ExtractBit(Raw_Header[4], 7);                    // Word 4: 7
        EXTERNAL_DISC_FLAG   = (Raw_Header[4] & 0x00000100) >> 8;              // Word 4: 8
        PEAK_VALID_FLAG      = (Raw_Header[4] & 0x00000200) >> 9;              // Word 4: 9
        OFFSET_FLAG          = (Raw_Header[4] & 0x00000400) >> 10;              // Word 4: 10
        CFD_VALID_FLAG       = ExtractBit(Raw_Header[4], 11);                 // Word 4: 11
        SYNC_ERROR_FLAG      = (Raw_Header[4] & 0x00001000) >> 12;              // Word 4: 12
        GENERAL_ERROR_FLAG   = (Raw_Header[4] & 0x00002000) >> 13;              // Word 4: 13
        PILEUP_ONLY_FLAG     = (Raw_Header[4] & 0x00004000) >> 14;              // Word 4: 14
        PILEUP_FLAG          = (Raw_Header[4] & 0x00008000) >> 15;              // Word 4: 15
        LAST_DISC_TIMESTAMP  = ((uint64_t)(Raw_Header[4] & 0xFFFF0000)) >> 16 |  // Word 4: 31..16 & 
                                  ((uint64_t)(Raw_Header[5] & 0x00003FFF)) << 16;  // Word 5: 13..0    

        TRIG_MON_DET_DATA    = (unsigned short)
                                (    ((Raw_Header[4] & 0xF) << 12)                //Word 4: 3:0 => bits 15:12
                                  | ((Raw_Header[5] & 0xC0000000) >> 20)            //Word 5: 31:30 => bits 11:10
                                  | ((Raw_Header[5] & 0x0000C000) >> 6)              //Word 5: 15:14 => bits 09:08
                                  | ((Raw_Header[6] & 0xFF000000) >> 24)            //Word 6: 31:24 => bits 07:00
                                );

        CFD_SAMPLE_0        = (Raw_Header[5] & 0x3FFF0000) >> 16;              // Word 5: 29..16     
                            
        SAMPLED_BASELINE    = (Raw_Header[6] & 0x00FFFFFF) >> 0;              // Word 6: 23..0

        TRIG_MON_XTRA_DATA  = 0xFFFF;  //TRIG_MON_XTRA_DATA not available in CFD header
        CFD_SAMPLE_1        = (Raw_Header[7] & 0x00003FFF) >> 0;              // Word 7: 13..0
        CFD_SAMPLE_2        = (Raw_Header[7] & 0x3FFF0000) >> 16;              // Word 7: 29..16
        
        PRE_RISE_ENERGY      = (Raw_Header[8] & 0x00FFFFFF) >> 0;              // Word 8: 23..0
        POST_RISE_ENERGY     = (Raw_Header[8] & 0xFF000000) >> 24 |            // Word 8: 31..24 & 
                                    (Raw_Header[9] & 0x0000FFFF) << 8;            // Word 9: 15..0 
        PEAK_TIMESTAMP          = (Raw_Header[9] & 0xFFFF0000) >> 16;            // Word 9: 31..16
        TS_OF_TRIGGER           = (Raw_Header[10] & 0xFFFF0000) >> 16;            // Word 10  31:16
        POST_RISE_LEAVE_SAMPLE  = (Raw_Header[11] & 0x00003FFF) >> 0;            // Word 11: 13..0
        POST_RISE_ENTER_SAMPLE  = (Raw_Header[11] & 0x3FFF0000) >> 16;            // Word 11: 29..16
        PRE_RISE_ENTER_SAMPLE   = (Raw_Header[12] & 0x00003FFF) >> 0;            // Word 12: 13..0
        PRE_RISE_LEAVE_SAMPLE   = (Raw_Header[12] & 0x3FFF0000) >> 16;            // Word 12: 29..16
        PEAK_SAMPLE             = (Raw_Header[13] & 0x00003FFF) >> 0;            // Word 13: 13..0
        BASE_SAMPLE             = (Raw_Header[13] & 0x3FFF0000) >> 16;            // Word 13: 29..16  
        
      break;
      default: break;  
    }
    return;
  }

  void DecodeHeader_7_8(unsigned int Raw_Header[14]){

    // FIXED_AAAAAAAA  = (Raw_Header[0] & 0xFFFFFFFF) >> 0;// Word 0: 31..0
    CH_ID           = (Raw_Header[1] & 0x0000000F) >> 0;// Word 1: 3..0
    USER_DEF        = (Raw_Header[1] & 0x0000FFF0) >> 4;// Word 1: 15..4
    PACKET_LENGTH   = (Raw_Header[1] & 0x07FF0000) >> 16;// Word 1: 26..16
    GEO_ADDR        = (Raw_Header[1] & 0xF8000000) >> 27;// Word 1: 31..27

    EVENT_TIMESTAMP = ((uint64_t)(Raw_Header[2] & 0xFFFFFFFF)) >> 0  // Word 2: 31..0 & 
                    | ((uint64_t)(Raw_Header[3] & 0x0000FFFF)) << 32;// Word 3: 15..0 


    PEQ_BYPASS      = ExtractBit(Raw_Header[3], 20);//Word 3 (20)
    TRIG_TS_MODE    = ExtractBit(Raw_Header[3], 21);//Word 3 (21)
    CFD_ESUM_MODE   = ExtractBit(Raw_Header[3], 22);//Word 3, bit 22; new item added 20210817.  Should always be zero in LED mode.
    HEADER_TYPE     = (Raw_Header[3] & 0x000F0000) >> 16;// Word 3: 19..16
    PEQ_BYPASS      = ExtractBit(Raw_Header[3], 20);// Word 3 (20)
    TRIG_TS_MODE    = ExtractBit(Raw_Header[3], 21);// Word 3 (21)
    //Bit 22 of word 3 is not used in LED mode, but is used in CFD mode as of 20210817.
    EVENT_TYPE      = (Raw_Header[3] & 0x03800000) >> 23;// Word 3: 25..23
    HEADER_LENGTH   = (Raw_Header[3] & 0xFC000000) >> 26;// Word 3: 31..26

    //Word 4
    EARLY_PRE_RISE_SELECT = ExtractBit(Raw_Header[4], 4);//Word 4 (bit 4)
    WRITE_FLAGS           = ExtractBit(Raw_Header[4], 5);//Word 4 (bit 5)
    VETO_FLAG             = ExtractBit(Raw_Header[4], 6);//Word 4 (bit 6)
    EXTERNAL_DISC_FLAG    = ExtractBit(Raw_Header[4], 8);//Word 4 (bit 8)
    PEAK_VALID_FLAG       = ExtractBit(Raw_Header[4], 9);//Word 4 (bit 9)
    OFFSET_FLAG           = ExtractBit(Raw_Header[4], 10);//Word 4 (bit 10)
    PILEUP_ONLY_FLAG      = ExtractBit(Raw_Header[4], 14);//Word 4 (bit 14)
    PILEUP_FLAG           = ExtractBit(Raw_Header[4], 15);//Word 4 (bit 15)


    if( HEADER_TYPE < 7 || HEADER_TYPE > 8 ){
      printf("\033[31m ERROR. Invalid header type %d. \033[0m\n", HEADER_TYPE);
      return;
    }

    //Extraction of LED (mode 7)
    if (HEADER_TYPE == 7){

      TIMESTAMP_MATCH_FLAG  = 0;  
      CFD_VALID_FLAG        = 0;

      //Word 5
      LAST_DISC_TIMESTAMP   = ((uint64_t)Raw_Header[5] << 16)                //Bits 47:16 from Word 5 (31:00)
                            | (((uint64_t)Raw_Header[4] & 0xFFFF0000) >> 16);//Bits 15:00 from Word 4 (31:16)

      //Word 6
      PILEUP_COUNT          = (unsigned short) ((Raw_Header[6] & 0x0F000000) >> 24);//Word 6 (27:24)

      //Word 7
      TRIG_MON_XTRA_DATA    = (Raw_Header[7] & 0x0000FFFF);//Word 7 (15:0)
      TRIG_MON_DET_DATA     = ((Raw_Header[7] & 0xFFFF0000) >> 16);//Word 7 (31:16)

    }

    //Extraction of CFD (mode 8)
    if (HEADER_TYPE == 8){

      TIMESTAMP_MATCH_FLAG  = ExtractBit(Raw_Header[4], 7);//Word 4 (bit 7)
      CFD_VALID_FLAG        = ExtractBit(Raw_Header[4], 11);//Word 4 (bit 11)

      TRIG_MON_DET_DATA     = (unsigned short)((Raw_Header[4] & 0x0000000F) << 12)//Word 4 (03:00) => bits 15:12 of header value.
                            | ((Raw_Header[5] & 0x0000C000) >> 6)                 //Word 5 (15:14) ==> bits (09:08) of value.
                            | ((Raw_Header[5] & 0xC0000000) >> 20)                //Word 5 (31:30) ==> bits (11:10) of value.
                            | ((Raw_Header[6] & 0xFF000000) >> 24);               //Word 6 (31:24) are bits (07:00) of TRIG_MON_DET_DATA.

      //Word 5
      LAST_DISC_TIMESTAMP = ((uint64_t)(Raw_Header[5] & 0x000003FFF) << 16 ) 
                          | (((uint64_t)Raw_Header[4] & 0xFFFF0000) >> 16);//Bits 15:00 from Word 4 (31:16)

      //rest of word 5
      CFD_SAMPLE_0       = ((Raw_Header[5] & 0x3FFFF0000) >> 16);//Word 5 (29:16) are CFD_SAMPLE_0.

      //Word 7
      TRIG_MON_DET_DATA = 0;
      CFD_SAMPLE_1   = (Raw_Header[7] & 0x00003FFF);//Word 7 (13:00) are CFD_SAMPLE_1.
      CFD_SAMPLE_2   = ((Raw_Header[7] & 0x3FFF0000) >> 16);//Word 7 (29:16) are CFD_SAMPLE_2.
      PILEUP_COUNT   = ((Raw_Header[7] & 0xC0000000) >> 28)//Word 7 (31:30) => (03:02) of value; 
                     | ((Raw_Header[7] & 0x0000C000) >> 14);//Word 7 (13:14) => (01:00) of value; 
      
    }

    //Word 6
    SAMPLED_BASELINE= (Raw_Header[6] & 0x00FFFFFF);//Word 6 (23:0)
    
    //Word 8
    POST_RISE_ENERGY = ((Raw_Header[8] & 0xFF000000) >> 24)//Word 8 (31:24) ==> (07:00) of value
                      | ((Raw_Header[9] & 0x0000FFFF) << 8);// Word 9 (15:00) ==> (23:08) of value
    PRE_RISE_ENERGY  = (Raw_Header[8] & 0x00FFFFFF);//Word 8 (23:0)
    
    //Word 9
    PEAK_TIMESTAMP        = (unsigned short)((Raw_Header[9] & 0xFFFF0000) >> 16);//Word 9(31:16)

    //Word 10
    TS_OF_TRIGGER         = (unsigned short) ((Raw_Header[10] & 0xFFFF0000) >> 16);//Word 10 (31:16)
    P2_MODE               = (unsigned short)(ExtractBit(Raw_Header[10], 14));//Word 10 (14)
    CAPTURE_PARST_TS      = (unsigned short)(ExtractBit(Raw_Header[10], 15));//Word 10 (15)

    P2_SUM                 = (unsigned int) ((Raw_Header[13] & 0x000003FF) << 14)//Word 13 (9:0) is bits 23:14
                          | (unsigned int) (Raw_Header[10] & 0x00003FFF);//Word 10 (13:0)

    //LAST_POST_RISE_M_SUM spans words 11, 12 and 13.
    LAST_POST_RISE_M_SUM  = ((Raw_Header[11] & 0xFF000000) >> 8)//Word 11 (31:24) is bits (23:16)
                          | ((Raw_Header[12] & 0xFF000000) >> 16)//Word 12 (31:24) is bits (15:08)
                          | ((Raw_Header[13] & 0xFF000000) >> 24);//Word 13 (31:24) is bits (07:00)

    //Word 11
    MULTIPLEX_DATA        = (Raw_Header[11] & 0x00FFFFFF);//Word 11 (23:0) can be energy or time

    //Word 12
    EARLY_PRE_RISE_ENERGY = (Raw_Header[12] & 0x00FFFFFF);//Word 12 (23:0)

    //Word 13
    SECOND_THRESH_DISC_FLAG = (unsigned short int)ExtractBit(Raw_Header[13], 10);//Word 13 (10)
    PARST_TSM               = (unsigned short int)ExtractBit(Raw_Header[13], 11);//Word 13 (11)
    PREVIOUS_CFD_VALID      = (unsigned short int)ExtractBit(Raw_Header[13], 12);//Word 13 (12)
    COARSE_FIRED            = (unsigned short int)ExtractBit(Raw_Header[13], 13);//Word 13, bit 13 is COARSE_FIRED
    TS_OF_COARSE            = ((Raw_Header[4] & 0x00003000) >> 2) + ((Raw_Header[13] & 0x00FFC000) >> 14);//Word 4 (13:12) & Word 13 (23:14)

  }

};

#endif
