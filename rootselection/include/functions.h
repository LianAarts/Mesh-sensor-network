#ifndef functions
#define functions
#include <Arduino.h>
#include <painlessMesh.h>

void post(String sensorVal, String name, String unit);
void setupWebserver();
void setupSensor();
void endWebserver();
void nextDnsRequest();
void setupNetwork();

String makeSensorMessage();

int networkScan();

extern String inputName;
extern painlessMesh mesh; // for the mesh network

#define ssidScan "home-assistant-AP"
#define passScan "networkubdx"

#endif