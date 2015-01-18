#ifndef _Rf24PacketDefine_h
#define _Rf24PacketDefine_h

#define RF24_CHANNEL 90

#define RF24_TYPE_TIME 0
#define RF24_TYPE_TEMP 1
#define RF24_TYPE_ID 2

typedef struct {
  uint8_t Raw [8];    // Raw payload, just 32 bytes of something
} PayloadRaw_t;

typedef struct {
  uint8_t BattLevel;    // Battery voltage over 2V in 10s of mV - e.g. 3.3V is reported as 130
} PayloadDefault_t;

typedef struct {
  uint8_t BattLevel; 
  uint32_t Time;        // Time in standard Unix format
} PayloadTime_t;

typedef struct {
  uint8_t BattLevel;
  uint8_t Temperature[4];    // Up to four temperatures, reported in half degrees. 255 means 'no temperature'
} PayloadTemperature_t;

typedef struct {
  uint8_t BattLevel;
  char Id[8];          // Node identification
  uint8_t Version;
  uint16_t Flags;
  uint8_t _padding[4]; // To 16 bytes
} PayloadId_t;

typedef union {
  PayloadDefault_t Default;
  PayloadTime_t Time;
  PayloadTemperature_t Temperature;
  PayloadRaw_t Raw;
} Payload_t;

#endif