#ifndef rd_22_280sensor
#define rd_22_280sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

void setupSensor280();

String makeSensorMessage280(bool isRoot, String nodeName, String IP, String nodeID);

#endif