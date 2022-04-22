#ifndef rd_22_json
#define rd_22_json
#include <Arduino.h>
#include <Zanshin_BME680.h>
#include <ArduinoJson.h>

void setupSensor();

String makeSensorMessage(bool isRoot, String nodeName, String IP, String nodeID);

#endif