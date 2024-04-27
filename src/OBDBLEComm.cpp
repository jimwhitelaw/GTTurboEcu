#include "OBDBLEComm.h"
#include "definitions.h"


OBDBLEComm::OBDBLEComm() {
    
}

OBDBLEComm::~OBDBLEComm() {
    operator delete(serialBLE);
}

void OBDBLEComm::init(const String& deviceName) {
    Serial.println("Starting BLE server");
    serialBLE = new BleSerial();
    //delay(2000);
    serialBLE->begin(deviceName.c_str());
    serialBLE->setTimeout(1000);
    setToDefaults();

}

void OBDBLEComm::writeEnd() {

    // 1 - write carriage return
    writeTo("\r");

    // 2- (optional ) write linefeed
    if (lineFeedEnable) {
        writeTo("\n");
    }

    // 3 - Write prompt
    writeTo(">");

    serialBLE->flush();
};

void OBDBLEComm::writeEndOK() {
    writeTo("OK");
    writeEnd();
}

void OBDBLEComm::writeEndERROR() {
    writeTo("ERROR");
    writeEnd();
}

void OBDBLEComm::writeEndNoData() {
    writeTo("NO DATA");
    writeEnd();
}

void OBDBLEComm::writeEndUnknown() {
    writeTo("?");
    writeEnd();
}

void OBDBLEComm::setToDefaults() {
    setEcho(true);
    setStatus(READY);
    setWhiteSpaces(true);
    setHeaders(false);
    setLineFeeds(true);
    setMemory(false);
}

void OBDBLEComm::writeTo(char const *response) {
    serialBLE->print(response);
}

// void OBDBLEComm::writeTo(uint8_t cChar) {
//     serial->print((char *)cChar);
// }

void OBDBLEComm::writeEndPidTo(char const *response) {
    if (whiteSpacesEnabled) {
        uint8_t len = strlen(response);
        char spacedResponse[len + len / 2 + 1];
        addSpacesToResponse(response, spacedResponse);
        writeTo(spacedResponse);
    } else {
        writeTo(response);
    }
    writeEnd();
}

void OBDBLEComm::readData(String& rxData) {
    while (serialBLE->available())
    {
        uint8_t rec = serialBLE->read();
        if (rec != 0x0D) 
        {
            rxData += (char)rec;
        }
    }            
       
    if (isEchoEnable()) {
        writeTo(rxData.c_str());
    }
} 

void OBDBLEComm::setBaudRate(uint32_t rate) {
    this->baudRate = rate;
}

long OBDBLEComm::getBaudRate() {
    return this->baudRate;
}

bool OBDBLEComm::isEchoEnable() {
    return this->echoEnable;
}

void OBDBLEComm::setEcho(bool echo) {
    this->echoEnable = echo;
}

void OBDBLEComm::setStatus(STATUS status) {
    this->status = status;
}

void OBDBLEComm::setLineFeeds(bool status) {
    this->lineFeedEnable = status;
}

void OBDBLEComm::setMemory(bool status) {
    this->memoryEnabled = status;
}

void OBDBLEComm::setWhiteSpaces(bool status) {
    this->whiteSpacesEnabled = status;
}

void OBDBLEComm::setHeaders(bool status) {
    this->headersEnabled = status;
}

void OBDBLEComm::addSpacesToResponse(const char *response, char spacedRes[]) {
    uint8_t len = strlen(response);
    int j = 0;
    for (int i = 0; i < len;) {
        *(spacedRes + j++) = *(response + i++);
        *(spacedRes + j++) = *(response + i++);
        if (i < len) {
            *(spacedRes + j++) = 0x20;
        }
    }
    *(spacedRes + j) = '\0';
}
