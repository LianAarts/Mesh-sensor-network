#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

Adafruit_BME280 BME280;

void setupSensor280(){
  BME280.begin(); 
}

String makeSensorMessage280(bool isRoot, String nodeName, String IP, String nodeID){
  //! remove static
  // variables for the measurements
  // get the sensordata
  float tempBME280 = BME280.readTemperature();
  float humBME280 = BME280.readHumidity();
  float presBME280 = BME280.readPressure();
  // calculate all the measurements

  //! rapidjson maybe
  String header = "[{\"bn\": \"" + nodeName;
  String meas1 = "\", \"n\": \"temperature\", \"u\": \"°C\", \"v\":" + String(tempBME280);
  String meas2 = "}, {\"n\": \"humidity\", \"u\": \"%\", \"v\":" + String(humBME280);
  String meas3 = "}, {\"n\": \"pressure\", \"u\": \"Hpa\", \"v\":" + String(presBME280);
  String meas4 = "}, {\"n\": \"chipID\", \"u\": \"ID\", \"v\":" + String(nodeID);
  // make the JSON message
  if (isRoot){
    String meas5 = "}, {\"n\": \"rootIP\", \"u\": \"\", \"v\":\"" + IP;
    return(header + meas1 + meas2 + meas3 + meas4 + meas5 + "\"\"}]");
    // return the JSON string to the main loop
  }
  else{
    String meas5 = "}, {\"n\": \"rootIP\", \"u\": \"\", \"v\": \"0.0.0.0\"";
    return(header + meas1 + meas2 + meas3 + meas4 + meas5 + "\"}]");
    // return the JSON string to the main loop
  }
}