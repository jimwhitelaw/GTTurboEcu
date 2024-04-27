#ifndef ELMulator_OBDBLEComm_h
#define ELMulator_OBDBLEComm_h

#include <Arduino.h>
#include "definitions.h"
#include <BleSerial.h>


class OBDBLEComm
{
public:
    enum STATUS
    {
        IDLE = 0,
        READY = 1
    };

// // Bluetooth is not built in, we are using a BT module via GPIO
//     OBDSerialComm(uint32_t baudRate, uint8_t rxPin, uint8_t txPin);

    OBDBLEComm();

    ~OBDBLEComm();

    void init(const String &deviceName);

    void writeEndOK();

    void writeEndERROR();

    /**
     * Response for unsupported PID command or missing sensor, respond "NO DATA".
     */
    void writeEndNoData();

    /**
     * Response for invalid AT command, or invalid PID or other bad input, respond "?"
     */
    void writeEndUnknown();

    void setToDefaults();

    void readData(String &rxData);

    void writeTo(uint8_t cChar);

    void writeTo(char const *string);

    void setEcho(bool echo);

    void writeEnd();

    bool isEchoEnable();

    void setLineFeeds(bool status);

    void setMemory(bool status);

    void setWhiteSpaces(bool status);

    void setHeaders(bool status);

    void setStatus(STATUS status);

    void writeEndPidTo(char const *string);

private:
    uint32_t baudRate; // Serial Baud Rate
    STATUS status;     // Operation status
    bool echoEnable;   // echoEnable command after received
    bool lineFeedEnable;
    bool memoryEnabled;
    bool whiteSpacesEnabled;
    bool headersEnabled;

    void setBaudRate(uint32_t rate);

    long getBaudRate();

    void addSpacesToResponse(const char *response, char string[]);

#ifndef BLUETOOTH_BUILTIN
    HardwareSerial *serial; // lib to communicate with bluetooth
#else
    BleSerial *serialBLE;
#endif
};

#endif