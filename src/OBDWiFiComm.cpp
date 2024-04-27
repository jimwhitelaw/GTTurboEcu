#include "OBDWiFiComm.h"
#include "definitions.h"

WiFiServer server(35000);
IPAddress localIP = IPAddress(192, 168, 0, 10);
IPAddress gateway = IPAddress(192, 168, 0, 10);
IPAddress subnet = IPAddress(255,255,255,0);

OBDWiFiComm::OBDWiFiComm() {
    
}

OBDWiFiComm::~OBDWiFiComm() {
}

void OBDWiFiComm::init(const String& deviceName) {
    Serial.println("Starting AP . . .");
   
    WiFi.softAPConfig(localIP, gateway, subnet);
    WiFi.softAP(deviceName);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.print("AP SSID: ");
    Serial.println(deviceName);
    server.begin();
    setToDefaults();
}

void OBDWiFiComm::writeEnd() {

    // 1 - write carriage return
    writeTo("\r");

    // 2- (optional ) write linefeed
    if (lineFeedEnable) {
        writeTo("\n");
    }

    // 3 - Write prompt
    writeTo(">");
};


void OBDWiFiComm::writeEndOK() {
    writeTo("OK");
    writeEnd();
}

void OBDWiFiComm::writeEndERROR() {
    writeTo("ERROR");
    writeEnd();
}

void OBDWiFiComm::writeEndNoData() {
    writeTo("NO DATA");
    writeEnd();
}

void OBDWiFiComm::writeEndUnknown() {
    writeTo("?");
    writeEnd();
}

void OBDWiFiComm::setToDefaults() {
    setEcho(true);
    setStatus(READY);
    setWhiteSpaces(true);
    setHeaders(false);
    setLineFeeds(true);
    setMemory(false);
}

void OBDWiFiComm::writeTo(char const *response) {
    client.print(response);
}


void OBDWiFiComm::writeTo(uint8_t cChar) {
    client.print(cChar);
}

void OBDWiFiComm::writeEndPidTo(char const *response) {
    if (this->whiteSpacesEnabled) {
        uint8_t len = strlen(response);
        char spacedResponse[len + len / 2 + 1];
        addSpacesToResponse(response, spacedResponse);
        writeTo(spacedResponse);
    } else {
        writeTo(response);
    }
    writeEnd();
}

void OBDWiFiComm::readData(String& rxData) {
    if (!client.connected()) {
        client = server.available();
    }

    if(client)
    {
        rxData = client.readStringUntil(SERIAL_END_CHAR);
        if (isEchoEnable()) 
        {
            writeTo(rxData.c_str());
        }
    }
}

bool OBDWiFiComm::isEchoEnable() {
    return this->echoEnable;
}

void OBDWiFiComm::setEcho(bool echo) {
    this->echoEnable = echo;
}

void OBDWiFiComm::setStatus(STATUS status) {
    this->status = status;
}

void OBDWiFiComm::setLineFeeds(bool status) {
    this->lineFeedEnable = status;
}

void OBDWiFiComm::setMemory(bool status) {
    this->memoryEnabled = status;
}

void OBDWiFiComm::setWhiteSpaces(bool status) {
    this->whiteSpacesEnabled = status;
}

void OBDWiFiComm::setHeaders(bool status) {
    this->headersEnabled = status;
}

void OBDWiFiComm::addSpacesToResponse(const char *response, char spacedRes[]) {
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
