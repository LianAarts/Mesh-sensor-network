#ifndef rd_22_wifi
#define rd_22_wifi
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

void post(String sensorVal, String name, String unit);
void splitJson(String json);
void setupNetwork();

int networkScan(); 

String getIp();

#endif