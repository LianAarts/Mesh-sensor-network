#ifndef functions
#define functions
#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>

#include <secret.h>
#define ssidScan ssidScan_secr
#define passScan passScan_secr

//! doxygen

//! const String& sensorVal
void post(String sensorVal, String name, String unit);
void writeSpiffs(String data);
void postJson(String json);
void setupWebserver();
void setupSensor();
void endWebserver();
void nextDnsRequest();
void setupNetwork();

String readSpiffs();
String makeSensorMessage(bool isRoot);

int networkScan();

extern String inputName;
extern painlessMesh mesh; // for the mesh network
extern WiFiClient client;

#endif