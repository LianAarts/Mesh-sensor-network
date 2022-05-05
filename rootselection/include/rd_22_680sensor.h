#ifndef rd_22_680sensor
#define rd_22_680sensor
#include <Arduino.h>
#include <Zanshin_BME680.h>

void setupSensor680();

String makeSensorMessage680(bool isRoot, String nodeName, String IP,
                            String nodeID);

#endif