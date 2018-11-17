
#include "DustSensor_v2.h"
// #include "Particle.h"
#include <ThingSpeak.h>

namespace DustSensor {
    
    bool DustSensor_Reader::pms7003_read(PMS7003_framestruct *thisFrame, char * printbuf, int printbufLen) {
    // Particle.publish("PMS7003", printbuf, 60, PRIVATE);
    // send data only when you receive data:
    //sprintf(thisFrame->deviceType, "PMS7003");

    uint16_t calcChecksum = 0;
    const int MAX_FRAME_LEN = 64;
    int incomingByte = 0;  // for incoming serial data
    char frameBuf[MAX_FRAME_LEN];
    int detectOff = 0;
    int frameLen = MAX_FRAME_LEN;
    bool inFrame = false;
    char buff[50];
    thisFrame->frameLen = MAX_FRAME_LEN;

    strcpy(thisFrame->deviceType, "PMS7003");
    Serial.println("-- Reading PMS7003");
    Serial1.begin(9600);
    bool packetReceived = false;
    while (!packetReceived) {
        if (Serial1.available() > 32) {
            int drain = Serial1.available();
            if (DEBUG) {
                Serial.print("-- Draining buffer: ");
                Serial.println(Serial1.available(), DEC);
            }
            for (int i = drain; i > 0; i--) {
                Serial1.read();
            }
        }
        if (Serial1.available() > 0) {
            if (DEBUG) {
                Serial.print("-- Available: ");
                Serial.println(Serial1.available(), DEC);
            }
            incomingByte = Serial1.read();
            if (DEBUG) {
                Serial.print("-- READ: ");
                Serial.println(incomingByte, HEX);
            }
            if (!inFrame) {
                if (incomingByte == 0x42 && detectOff == 0) {
                    frameBuf[detectOff] = incomingByte;
                    thisFrame->frameHeader[0] = incomingByte;
                    calcChecksum = incomingByte; // Checksum init!
                    detectOff++;
                }
                else if (incomingByte == 0x4D && detectOff == 1) {
                    frameBuf[detectOff] = incomingByte;
                    thisFrame->frameHeader[1] = incomingByte;
                    calcChecksum += incomingByte;
                    inFrame = true;
                    detectOff++;
                }
                else {
                    Serial.print("-- Frame syncing... ");
                    Serial.print(incomingByte, HEX);
                    if (DEBUG) {
                    }
                    Serial.println();
                }
            }
            else {
                frameBuf[detectOff] = incomingByte;
                calcChecksum += incomingByte;
                detectOff++;
                uint16_t val = frameBuf[detectOff-1]+(frameBuf[detectOff-2]<<8);
                switch (detectOff) {
                    case 4:
                        thisFrame->frameLen = val;
                        frameLen = val + detectOff;
                        break;
                    case 6:
                        thisFrame->concPM1_0_CF1 = val;
                        break;
                    case 8:
                        thisFrame->concPM2_5_CF1 = val;
                        break;
                    case 10:
                        thisFrame->concPM10_0_CF1 = val;
                        break;
                    case 12:
                        thisFrame->concPM1_0_amb = val;
                        break;
                    case 14:
                        thisFrame->concPM2_5_amb = val;
                        break;
                    case 16:
                        thisFrame->concPM10_0_amb = val;
                        break;
                    case 18:
                        thisFrame->rawGt0_3um = val;
                        break;
                    case 20:
                        thisFrame->rawGt0_5um = val;
                        break;
                    case 22:
                        thisFrame->rawGt1_0um = val;
                        break;
                    case 24:
                        thisFrame->rawGt2_5um = val;
                        break;
                    case 26:
                        thisFrame->rawGt5_0um = val;
                        break;
                    case 28:
                        thisFrame->rawGt10_0um = val;
                        break;
                    case 29:
                        val = frameBuf[detectOff-1];
                        thisFrame->version = val;
                        break;
                    case 30:
                        val = frameBuf[detectOff-1];
                        thisFrame->errorCode = val;
                        break;
                    case 32:
                        thisFrame->checksum = val;
                        calcChecksum -= ((val>>8)+(val&0xFF));
                        break;
                    default:
                        break;
                }
    
                if (detectOff >= frameLen) {
                    snprintf(printbuf, printbufLen, "PMS7003 ");
                    snprintf(printbuf, printbufLen, "%s[%02x %02x] (%04x) ", printbuf,
                        thisFrame->frameHeader[0], thisFrame->frameHeader[1], thisFrame->frameLen);
                    snprintf(printbuf, printbufLen, "%sCF1=[%04x %04x %04x] ", printbuf,
                        thisFrame->concPM1_0_CF1, thisFrame->concPM2_5_CF1, thisFrame->concPM10_0_CF1);
                    snprintf(printbuf, printbufLen, "%samb=[%04x %04x %04x] ", printbuf,
                        thisFrame->concPM1_0_amb, thisFrame->concPM2_5_amb, thisFrame->concPM10_0_amb);
                    snprintf(printbuf, printbufLen, "%sraw=[%04x %04x %04x %04x %04x %04x] ", printbuf,
                        thisFrame->rawGt0_3um, thisFrame->rawGt0_5um, thisFrame->rawGt1_0um,
                        thisFrame->rawGt2_5um, thisFrame->rawGt5_0um, thisFrame->rawGt10_0um);
                    snprintf(printbuf, printbufLen, "%sver=%02x err=%02x ", printbuf,
                        thisFrame->version, thisFrame->errorCode);
                    snprintf(printbuf, printbufLen, "%scsum=%04x %s xsum=%04x", printbuf,
                        thisFrame->checksum, (calcChecksum == thisFrame->checksum ? "==" : "!="), calcChecksum);
                    // Serial.println(printbuf);
                    // Particle.publish("Data1", printbuf, 60, PRIVATE);
                    
                    sprintf(buff, "%d", thisFrame->concPM1_0_amb);
                    Particle.publish("PM 1.0 (μg/m3)", buff, 40, PRIVATE);
                    ThingSpeak.setField(6, buff);
                    sprintf(buff, "%d", thisFrame->concPM2_5_amb);
                    Particle.publish("PM 2.5 (μg/m3) ", buff, 40, PRIVATE);
                    ThingSpeak.setField(7, buff);
                    sprintf(buff, "%d", thisFrame->concPM10_0_amb);
                    Particle.publish("PM 10.0 (μg/m3)", buff, 40, PRIVATE);
                    ThingSpeak.setField(8, buff);
                    packetReceived = true;
                    detectOff = 0;
                    inFrame = false;
                }
            }
        }
    }
    Serial1.end();
    thisFrame->dataGood = calcChecksum == thisFrame->checksum;
    thisFrame->readCount++;
    return thisFrame->dataGood;
    }
}