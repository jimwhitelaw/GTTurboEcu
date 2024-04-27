#ifndef ELMulator_ELMulator_h
#define ELMulator_ELMulator_h

#include <Arduino.h>
#include "definitions.h"

#if defined (OBD_TYPE_WIFI)
#include "OBDWiFiComm.h"
#elif defined (OBD_TYPE_BLE)
#include "OBDBLEComm.h"
#else 
#include "OBDSerialComm.h"
#endif

#include "ATCommands.h"
#include "PidProcessor.h"
#include "definitions.h"

class ELMulator
{

public:
    /**
     * When creating a new instance of ELMulator with an attached
     * BT module, we need to set the Bluetooth serial parameters
     *
     * @param baudRate - rate for bluetooth serial port (normally 9600)
     * @param rxPin - the Arduino RX pin for the Bluetooth module (serial port)
     * @param txPin - the Arduino TX pin for the Bluetooth module (serial port)
     * @return
     */
    ELMulator(uint32_t baudRate, uint8_t rxPin, uint8_t txPin);

    ELMulator();

    ~ELMulator();

    void init(const String &deviceName, const bool registerPids = false);

    /**
     * Waits and Reads a request from OBD client software (example: torque)
     *
     * It will only return if the OBD client software
     * request's one of the previous registred PID (see: registerMode01Pid())
     *
     * Any other request will be automaticly handle or it will be rejected.
     *
     * ELMulator lib will automaticly handle:
     * - any AT command
     * - PID's support request like 0100, 0120, 0140, 0160, ...
     *
     * @return the request PID (Vehicle speed ex:  010D )
     */
    bool readELMRequest();
    void sendELMResponse();
    void begin();

    /**
     * Registry the PID's (sensors) your arduino will support.
     * Currently ELMulator lib only supports MODE 01 PID's
     *
     * In OBDII MODE 01 PID's, are request
     * to Show current data for a given sensor identified by a PID
     *
     * List of MODE 01 PID's: https://en.wikipedia.org/wiki/OBD-II_PIDs#Mode_01
     *
     * Example, for Vehicle speed, where OBDII PID code is 0D:
     *
     * registerMode01Pid(0x0D)
     *
     * @param pidHexId
     * @return returns true if the entered ID is in the valide range [0x00, 0xFF]
     */
    bool registerMode01Pid(uint32_t pidHexId);

    bool registerMode01MILResponse(const String &response);

    bool registerMode03Response(const String &response);

    uint8_t getPidCodeOnly(uint16_t hexCommand);

    /**
     * Used to informe the OBD client that the PID,
     * it has requested is not supported
     *
     * In normal conditions, you will only receive,
     * request for the PID you have registred
     * All other request is automaticly handle by GTTuboEcu lib
     */
    void writePidNotSupported();

    /**
     * Respond to a given PID request from OBDII client (ex: torque)
     *
     * @param requestPid - the received pid
     * @param numberOfBytes - number of bytes this PID response VALUE is supposed to have
     * @param value - sensor value for this PID,
     *                  lib will convert it to HEX
     *                  with the appropriated number of HEX chars (numberOfChars)
     */
    void writePidResponse(const String &requestPid, uint8_t numberOfBytes, uint32_t value);

    // Write the response back to the requestor without PID formatting, etc
    // Just pass the response string on. Useful for testing with a specific response
    // that has been pre-configured.
    void writeResponse(const String &response);

    bool isMode01(const String &command);
    bool isMode03(const String &command);
    bool isMode01MIL(const String &command);
    bool isMode22(const String &command);

    uint8_t getPidCode(const String &request);
    void registerAllMode01Pids();
    uint32_t getMockSensorValue();

    String elmRequest;

private:
#if defined(OBD_TYPE_WIFI)
    OBDWiFiComm *_connection;
#elif defined(OBD_TYPE_BLE)
    OBDBLEComm *_connection;
#else
    OBDSerialComm *_connection;
#endif
    ATCommands *_atProcessor;
    PidProcessor *_pidProcessor;
    String _lastCommand;
    bool isCycleUp = true;
    uint32_t cycle = 0;
    bool processRequest(String &command);
    bool isValidHex(const char *pid);
};

#endif
