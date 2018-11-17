#include "Particle.h"

const bool DEBUG = false;
const int printbufLen = 256;

typedef struct {
    char     deviceType[32];
    uint8_t  frameHeader[2];
    uint16_t frameLen;
    uint16_t concPM1_0_CF1;
    uint16_t concPM2_5_CF1;
    uint16_t concPM10_0_CF1;
    uint16_t concPM1_0_amb;
    uint16_t concPM2_5_amb;
    uint16_t concPM10_0_amb;
    uint16_t rawGt0_3um;
    uint16_t rawGt0_5um;
    uint16_t rawGt1_0um;
    uint16_t rawGt2_5um;
    uint16_t rawGt5_0um;
    uint16_t rawGt10_0um;
    uint8_t  version;
    uint8_t  errorCode;
    uint16_t checksum;
    bool     dataGood;
    uint32_t readCount;
} PMS7003_framestruct;


extern char * print_buff;
extern PMS7003_framestruct * currFrame;


namespace DustSensor {
    class DustSensor_Reader
    {
     public:
     // returns bool 
     static bool pms7003_read(PMS7003_framestruct *thisFrame, char * print_buff, int printbufLen);
    };
    }