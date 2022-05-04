#include <Arduino.h>
#include <Zanshin_BME680.h>

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

BME680_Class BME680;
int32_t  temp, humidity, pressure, gas;

//***********************************************************************
//**************************** Setup sensor *****************************
/**
 * @brief setup the BME680 sensor
 * 
 */
void setupSensor680(){
  while (!BME680.begin(I2C_STANDARD_MODE)) {  //! ifdef for different sensors
    // Start BME680 using I2C, use first device found
    debugln1("-  Unable to find BME680. Trying again in 5 seconds.");
    delay(5000);
    // loop until the device is found
  }
  debugln3("- Setting 16x oversampling for all sensors");
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  debugln3("- Setting IIR filter to a value of 4 samples");
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  debugln3("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms");  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds
}

//***********************************************************************
//************************* Make sensor message *************************
/**
 * @brief Make the JSON that we send to the root node
 * 
 * This JSON is based on the SenML specification. This one JSON will include multiple measurements.
 * 
 * bn:  Base name or sensor name  (sensor1)
 * 
 * n:   Measurement name          (temperature)
 * 
 * u:   Unit of Measurement       (°C)
 * 
 * v:   Value                     (21.8)
 * 
 * @param isRoot If we are root we also send the IP address of the root so Home Assistant can send API requests to the root.
 * @param nodeName This is part of the entity ID we want to use in Home Assistant
 * @param IP IP address of the node.
 * @param nodeID NodeID, we use this to check duplicate names in Home Assistant
 * @return String String in JSON format with all the measurements
 */
String makeSensorMessage680(bool isRoot, String nodeName, String IP, String nodeID){
  //! remove static
  // variables for the measurements

  BME680.getSensorData(temp, humidity, pressure, gas);
  // get the sensordata
  float tempBME680 = (temp / 100) + float(temp % 100) / 100;
  float gasBME680 = (gas / 100) + float(gas % 100) /100;
  float humBME680 = (humidity / 1000)+ float(humidity % 1000) /1000;
  float presBME680 = (pressure / 100)+ float(pressure % 100) /100;
  // calculate all the measurements

  //! rapidjson maybe
  String header = "[{\"bn\": \"" + nodeName;
  String meas1 = "\", \"n\": \"temperature\", \"u\": \"°C\", \"v\":" + String(tempBME680);
  String meas2 = "}, {\"n\": \"gas\", \"u\": \"ohm\", \"v\":" + String(gasBME680);
  String meas3 = "}, {\"n\": \"humidity\", \"u\": \"%\", \"v\":" + String(humBME680);
  String meas4 = "}, {\"n\": \"pressure\", \"u\": \"Hpa\", \"v\":" + String(presBME680);
  String meas5 = "}, {\"n\": \"chipID\", \"u\": \"ID\", \"v\":" + String(nodeID);
  // make the JSON message
  if (isRoot){
    String meas6 = "}, {\"n\": \"rootIP\", \"u\": \"\", \"v\":\"" + IP;
    return(header + meas1 + meas2 + meas3 + meas4 + meas5 + meas6 + "\"\"}]");
    // return the JSON string to the main loop
  }
  else{
    String meas6 = "}, {\"n\": \"rootIP\", \"u\": \"\", \"v\": \"0.0.0.0\"";
    return(header + meas1 + meas2 + meas3 + meas4 + meas5 + meas6 + "\"}]");
    // return the JSON string to the main loop
  }
}