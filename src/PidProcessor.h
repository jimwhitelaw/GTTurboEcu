#ifndef ELMulator_PIDPROCESSOR_H
#define ELMulator_PIDPROCESSOR_H

#include "definitions.h"
#include <Arduino.h>
#include <WString.h>
#include <Print.h>
#include "OBDWiFiComm.h"
#include "OBDSerialComm.h"
#include "OBDBLEComm.h"

class PidProcessor
{

public:
    PidProcessor(OBDWiFiComm *connection);
    PidProcessor(OBDBLEComm *connection);
    PidProcessor(OBDSerialComm *connection);
    
    bool process(String &string);

    bool registerMode01Pid(uint32_t pid);

    bool registerMode01MILResponse(const String &response);

    bool registerMode03Response(const String &response);

    void writePidResponse(const String &requestPid, uint8_t numberOfBytes, uint32_t value);

    uint8_t getPidCodeFromHex(uint16_t hexCommand);
    uint8_t getPidCodeFromRequest(const String &command);

    bool isMode01(const String &command);
    bool isMode01MIL(const String &command);
    bool isMode03(const String &command);
    bool isMode22(const String &command);

private:
#if defined(OBD_TYPE_WIFI)
    OBDWiFiComm *_connection;
#elif defined(OBD_TYPE_BLE)
    OBDBLEComm *_connection;
#else
    OBDSerialComm *_connection;
#endif
    uint32_t pidMode01Supported[N_MODE01_INTERVALS];

    bool isSupportedPidRequest(uint8_t pid);

    uint32_t getSupportedPids(uint8_t pidcode);

    uint8_t getPidIntervalId(uint8_t pidcode);

    uint8_t getPidIntervalIndex(uint8_t pidcode);

    void setPidBit(uint8_t pid);

    uint8_t getPidBitPosition(uint8_t pidcode);

    String convertToPidResponse(String pid);

    void getFormattedResponse(char *response, uint8_t totalNumberOfChars, const String &pid, uint32_t value);

    void resetPidMode01Array();
};

#endif // ELMulator_PIDPROCESSOR_H
