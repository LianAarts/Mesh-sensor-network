#ifndef rd_22_280sensor
#define rd_22_280sensor
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

void setupSensor280();

String makeSensorMessage280(bool isRoot, String nodeName, String IP,
                            String nodeID);

#endif