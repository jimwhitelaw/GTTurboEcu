#include "PidProcessor.h"

#if defined(OBD_TYPE_WIFI)
PidProcessor::PidProcessor(OBDWiFiComm *connection) {
    _connection = connection;
    resetPidMode01Array();
};
#elif defined(OBD_TYPE_BLE)
PidProcessor::PidProcessor(OBDBLEComm *connection) {
    _connection = connection;
    resetPidMode01Array();
};
#else // default is OBD_TYPE_BTC (Bluetooth Classic)
PidProcessor::PidProcessor(OBDSerialComm *connection) {
    _connection = connection;
    resetPidMode01Array();
};
#endif

bool PidProcessor::process(String& command) {
    bool processed = false;
    
    if (command.length() > 4)
    {
        command = command.substring(0, 4); // remove num_responses value if present; lib only handles single responses
    }

    if (!isMode01(command) & !isMode22(command))
    {
        _connection->writeEndNoData();
        return false;
    }

    uint16_t hexCommand = strtoul(command.c_str(), NULL, HEX);
    uint8_t pid = getPidCodeFromHex(hexCommand);
    if (isSupportedPidRequest(pid)) {   //reqeust to return a list of valid PIDs we can respond to 
        processed = true;
        uint32_t supportedPids = getSupportedPids(pid);
        writePidResponse(command, 4, supportedPids);
    }
    return processed;
}

void PidProcessor::writePidResponse(const String& requestPid, uint8_t numberOfBytes, uint32_t value) {
    uint8_t  nHexChars = PID_N_BYTES * N_CHARS_IN_BYTE +  numberOfBytes * N_CHARS_IN_BYTE ;
    char responseArray[nHexChars + 1]; // one more for termination char
    getFormattedResponse(responseArray, nHexChars, requestPid, value);
    _connection->writeEndPidTo(responseArray);
}

/**
 * adds a supported pid, so it can answer to pid support request, ex 0100, 0120, ...
 */
bool PidProcessor::registerMode01Pid(uint32_t pid) {
    // pid must be off type 01
    if (pid > 0x00 && pid < 0x0200) {
        // remove PidMode, only use pid code
        pid = getPidCodeFromHex(pid);
        setPidBit(pid);

        char buffer[4];
        sprintf(buffer, "%02X", pid);
        DEBUG("Registered PID: " + String(buffer ));
        return true;
    }
    return false;
}

bool PidProcessor::isMode01(const String& command) 
{
    return command.startsWith("01") ? true : false;
}

bool PidProcessor::isMode01MIL(const String& command)
{
    return command.startsWith("0101") ? true : false;
}

bool PidProcessor::isMode03(const String& command)
{
    return command.startsWith("03") ? true : false;
}

bool PidProcessor::isMode22(const String& command)
{
    return command.startsWith("22") ? true : false;
}


/**
 *  return true if:
 *  - 1000
 *  - 1020
 *  - 1040
 *  - 1060
 *  ....
 */
bool PidProcessor::isSupportedPidRequest(uint8_t pid) {
    return pid == getPidIntervalId(pid);
}

/**
 * returns the interval RANGEID for the given pid (ex: 00, 20, 40, ...)
 */
uint8_t PidProcessor::getPidIntervalId(uint8_t pidcode) {
    return getPidIntervalIndex(pidcode) * PID_INTERVAL_OFFSET;
}

/**
 * returns the interval INDEX for the given pid (0,1,2,3,4,...)
 */
uint8_t PidProcessor::getPidIntervalIndex(uint8_t pidcode) {
    return pidcode / PID_INTERVAL_OFFSET;
}

/**
 * Receives a mode pid ex 010C ,
 * returns only the pid ex 0C
 */
uint8_t PidProcessor::getPidCodeFromHex(uint16_t hexCommand) {

    if (hexCommand >= 0x0100) {
        hexCommand -= 0x100;
    }
    return (uint8_t) hexCommand; // return only the lower 8 bits (trim off num_messages or other extra bytes if present)
}

/**
 * Receives a complete PID request, 
 * returns only the pid ex 0C
 */
uint8_t PidProcessor::getPidCodeFromRequest(const String& request) {
    uint16_t hexCommand = strtoul(request.c_str(), NULL, HEX);
    return getPidCodeFromHex(hexCommand);
}


void PidProcessor::setPidBit(uint8_t pid) {
    uint8_t arrayIndex = getPidIntervalIndex(pid);
    if(isSupportedPidRequest(pid)) {
        arrayIndex -= 1;
    }

    uint8_t bitPosition = getPidBitPosition(pid);
    bitWrite(pidMode01Supported[arrayIndex], bitPosition, 1);

    // mark available intervals
    for (int i = 0; i < arrayIndex; i++) {
        bitWrite(pidMode01Supported[i], 0, 1);
    }
}

uint8_t PidProcessor::getPidBitPosition(uint8_t pid) {
    uint8_t intervalId = getPidIntervalId(pid) ;
    if(isSupportedPidRequest(pid)) {
        intervalId -= PID_INTERVAL_OFFSET;
    }

    uint16_t pos = pid - intervalId;
    pos = PID_INTERVAL_OFFSET - pos;
    return pos;
}


uint32_t PidProcessor:: getSupportedPids(uint8_t pid) {
    uint8_t index = getPidIntervalIndex(pid);
    return pidMode01Supported[index];
}

void PidProcessor::getFormattedResponse(char *response, uint8_t totalNumberOfChars, const String& pid, uint32_t value) {
    uint8_t nValueChars = totalNumberOfChars - PID_N_BYTES * N_CHARS_IN_BYTE;
    char cValue[3] = {0};
    itoa(nValueChars, cValue, DEC);
    String mask = "%s%0";
    mask.concat(cValue);
    mask.concat("lX\0");
    String pidResponse = convertToPidResponse(pid);
    sprintf(response, mask.c_str(), pidResponse.c_str(), value);

    DEBUG("RX PID: " + pid + " - TX: " + String(response));
}

String PidProcessor::convertToPidResponse(String pid) {
    String x = "4";
    x.concat(pid.substring(1, pid.length()));
    return x;
}

void PidProcessor::resetPidMode01Array() {
    for(uint8_t i = 0; i < N_MODE01_INTERVALS; i++ ) {
        pidMode01Supported[i] = 0x0;
    }
}






