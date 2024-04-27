#pragma once
#define OBD_TYPE_BLE
#include <Arduino.h>
#include <ELMulator.h>
#include <BleSerial.h>
#include <definitions.h>

void handlePIDRequest(const String& pidRequest);