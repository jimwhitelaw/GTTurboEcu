#ifndef ELMulator_ATCommands_h
#define ELMulator_ATCommands_h

#include <Arduino.h>
#include "definitions.h"
#include "OBDWiFiComm.h"
#include "OBDBLEComm.h"
#include "OBDSerialComm.h"

class ATCommands
{

public:
    ATCommands(OBDWiFiComm *connection);
    ATCommands(OBDBLEComm *connection);
    ATCommands(OBDSerialComm *connection);
   
    ~ATCommands();

    bool process(const String &string);

private:
#if defined(OBD_TYPE_WIFI)
    OBDWiFiComm *connection;
#elif defined(OBD_TYPE_BLE)
    OBDBLEComm *connection;
#else // default is OBD_TYPE_BTC (Bluetooth Classic)
    OBDSerialComm *connection;
#endif
    void ATD();

    void ATZ();

    void ATI();

    void ATEx(String &x);

    void ATMx(String &x);

    void ATLx(String &x);

    void ATSx(String &x);

    void ATHx(String &x);

    void ATSPx(String &x);

    void ATATx(String &cmd);

    void ATPC();

    void ATDPN();

    void ATDESC();

    void ATRV();

    void processCommand(const String &command);

    bool isATCommand(const String &command);
};

#endif