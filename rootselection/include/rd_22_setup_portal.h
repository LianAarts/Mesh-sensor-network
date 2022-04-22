#ifndef rd_22_setup_portal
#define rd_22_setup_portal
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>

void nextDnsRequest();
void setupWebserver();
void endWebserver();

void writeSpiffs(String data);
String readSpiffs();

void setup_API();
#endif