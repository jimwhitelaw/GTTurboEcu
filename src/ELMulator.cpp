#include "ELMulator.h"

#if defined(OBD_TYPE_WIFI)
ELMulator::ELMulator()
{
    _connection = new OBDWiFiComm();
    _atProcessor = new ATCommands(_connection);
    _pidProcessor = new PidProcessor(_connection);
    _lastCommand = "";
    elmRequest.reserve(MAX_REQUEST_SIZE);
    elmRequest = "";
}
#elif defined(OBD_TYPE_BLE)
ELMulator::ELMulator()
{
    _connection = new OBDBLEComm();
    _atProcessor = new ATCommands(_connection);
    _pidProcessor = new PidProcessor(_connection);
    _lastCommand = "";
    elmRequest.reserve(MAX_REQUEST_SIZE);
    elmRequest = "";
}
#else // default is OBD_TYPE_BTC (Bluetooth Classic)
ELMulator::ELMulator()
{
    _connection = new OBDSerialComm();
    _atProcessor = new ATCommands(_connection);
    _pidProcessor = new PidProcessor(_connection);
    _lastCommand = "";
    elmRequest.reserve(MAX_REQUEST_SIZE);
    elmRequest = "";
}
#endif

ELMulator::~ELMulator() {}

void ELMulator::init(const String &deviceName, bool registerPids)
{
    _connection->init(deviceName);
    if (registerPids)
    {
        registerAllMode01Pids();
    }
}

void ELMulator::registerAllMode01Pids()
{
    // Set up PIDs 0x00 to 0x65
    // Will be provided a response with mock data value if no special handling is performed.
    // Requests for PID > 0x65 are not currently supported - will return "NO DATA"
    for (int i = 0; i < 0x66; i++)
    {
        _pidProcessor->registerMode01Pid(i);
    }
}

bool ELMulator::readELMRequest()
{
    do
    {
        elmRequest.clear(); // clear buffer from previous requests
        _connection->readData(elmRequest);
        elmRequest.toUpperCase();
        // TODO ignore spaces, and all control chars (tab, etc)
        // TODO accept single carriage return as repeat last command at or pid
        // ignore null i.e 00
    } while (processRequest(elmRequest)); // processRequest handles all non PID requests (AT commands, errors etc)
    return true;                          // We have received a valid PID request we need to respond to
}

void ELMulator::sendELMResponse()
{
    // Respond to any mode 01 request with a mock sensor value    
    if (isMode01(elmRequest)) 
    {
        uint8_t pidCode = getPidCode(elmRequest);
        writePidResponse(elmRequest, responseBytes[pidCode], getMockSensorValue());
        return;
    }
    // Not a mode 01 PID request. Report it as not supported (ie, "NO DATA");
    else
    {   
        writePidNotSupported();
    }
}

void ELMulator::begin()
{
    while (readELMRequest())
    {
        sendELMResponse();
    }
}

uint8_t ELMulator::getPidCode(const String &request)
{
    return _pidProcessor->getPidCodeFromRequest(request);
}

bool ELMulator::registerMode01Pid(uint32_t pid)
{
    return _pidProcessor->registerMode01Pid(pid);
}

bool ELMulator::registerMode01MILResponse(const String &response)
{
    return _pidProcessor->registerMode01MILResponse(response);
}

bool ELMulator::registerMode03Response(const String &response)
{
    return _pidProcessor->registerMode03Response(response);
}

void ELMulator::writePidNotSupported()
{
    _connection->writeEndNoData();
}

void ELMulator::writePidResponse(const String &requestPid, uint8_t numberOfBytes, uint32_t value)
{
    _pidProcessor->writePidResponse(requestPid, numberOfBytes, value);
}

void ELMulator::writeResponse(const String &response)
{
    _connection->writeTo(response.c_str());
    _connection->writeEnd();
}

bool ELMulator::processRequest(String &command)
{

    // TODO: check for no 0X0D char in command, return ? and >
    // TODO: receive only <cr>, repeat last command

    // carriage return, means repeat last command
    if (command == "\n")
    {
        command = _lastCommand;
    }

    // empty command, do not send to user
    if (command.length() == 0)
    {
        return true;
    }

    // Check for AT command
    if (_atProcessor->process(command))
    {
        _lastCommand = command;
        return true;
    }

    // Check for a valid hex string, return error if false
    if (!isValidHex(command.c_str()))
    {
        _connection->writeEndUnknown();
        DEBUG("Invalid HEX command: " + command);
        return true;
    }

    // Check for a valid PID request
    if (_pidProcessor->process(command))
    {
        _lastCommand = command;
        return true;
    }

    return false;
}

bool ELMulator::isValidHex(const char *pid)
{
    return (pid[strspn(pid, "0123456789abcdefABCDEF")] == 0);
}

bool ELMulator::isMode01(const String &command)
{
    return _pidProcessor->isMode01(command);
}

bool ELMulator::isMode22(const String &command)
{
    return _pidProcessor->isMode22(command);
}

bool ELMulator::isMode03(const String &command)
{
    return _pidProcessor->isMode03(command);
}

bool ELMulator::isMode01MIL(const String &command)
{
    return _pidProcessor->isMode01MIL(command);
}

/**
 * Fake some data
 * start counter = 0
 * every call to getFakeSensorData, will increment the counter to a max of 255 (0xff)
 * When 0xff is reached the cycle is reverted
 * and the counter will decrement by one until 0 is reached
 */
uint32_t ELMulator::getMockSensorValue()
{
    if (isCycleUp)
    {
        cycle++;
        if (cycle == 0xff)
        {
            isCycleUp = false;
        }
    }
    else
    {
        cycle--;
        if (cycle == 0)
        {
            isCycleUp = true;
        }
    }

    return cycle;
}