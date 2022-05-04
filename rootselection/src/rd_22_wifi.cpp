#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

#define token token_secr
#define ssidScan ssidScan_secr
#define passScan passScan_secr
// get the token from the secret.h

// make a sensor object

HTTPClient http;
// String serverName = "";

WiFiClient client;

IPAddress ip;

String httpRequestData = "";
String httpAtributes = "";
int httpResponseCode = 0;

String getIp(){
  return(ip.toString());
}

//***********************************************************************
//**************************** Post function ****************************

/**
 * @brief Post the measurement to the Home Assistant REST API
 * 
 * This wil be a individual entity that can display one value
 * One sensor will need multiple Posts if it measures more than one value
 * 
 * We always check the HTTP response of the POST. 
 * If the response is -1 we don't have a connection to Home Assistant.
 * If this happens we reset the node.
 * 
 * @param sensorVal Value of the measurement
 * @param name Entity ID used in Home Assistant
 * @param unit Unit of measurement
 */
void post(String sensorVal, String name, String unit) {
  // serverName = SERVERNAME + name;
  // the name in the JSON is used as the entity ID
  
  http.begin(client, SERVERNAME + name);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", token);
  // headers we need to make the post request to 

  httpRequestData = "{\"state\":\""+ String(sensorVal);
  httpAtributes = "\", \"attributes\": {\"unit_of_measurement\": \"" + unit;
  // make the json we have to send

  httpResponseCode = http.POST(httpRequestData + httpAtributes + "\"}}");
  // POST the json
  // debug3(sensorVal);
  // debug3("\t");
  debug3("HTTP Response code: ");
  debugln3(httpResponseCode);
  if (httpResponseCode == -1){
    debugln1("ERROR response code is -1, restart the ESP");
    // if the response code is -1 we don't have a connection to Home Assistant
    // we reset the ESP
    ESP.restart();
  }
  http.end();
}

//***********************************************************************
//**************************** Network scan *****************************
/**
 * @brief Measure the RSSI of the Home Assistant access point
 * 
 * Received Signal Strength Indicator (RSSI) is the measurement of how strong the signal of a router of access point is to a device.
 * The RSSI is a value between -100 and 0. Close to 0 means a better connection
 * 
 * @return int RSSI, a value between -100 and 0
 */
int networkScan(){
  WiFi.mode(WIFI_STA);
  // we use STA mode because we have to connect to the home assistant AP
  WiFi.begin(ssidScan, passScan);
  // connect to the wifi
  debug2("Connecting to WiFi ..");
  for (int i = 0 ; i < 20 and WiFi.status() != WL_CONNECTED ; i++){
    // we wait for the connection for 10 seconds
    // if we don't have a connection after 10 seconds we are probably not in range
    debug2('.');
    delay(500);  //! why delay (do some research)
  }

  int RSSI = WiFi.RSSI();
  if (WiFi.localIP()[0] == 0){
    // if the IP is 0.0.0.0 we don't have a connection
    debugln2("no IP, no connection to Home Assistant access point");
    RSSI = -100;
    // make the RSSI -100
  }

  debugln2(WiFi.localIP());
  debug2("RSSI: ");
  debugln2(RSSI);

  WiFi.disconnect();
  // disconnect after we have our RSSI
  return(RSSI);
  // return the RSSI
}

//***********************************************************************
//*************************** Network connect ***************************
/**
 * @brief Connect to the Home Assistant access point
 * 
 * When we are the root we need to send Post requests.
 * We need to connect to access point to be able to do this.
 * 
 */
void setupNetwork(){
  // connect to the AP if we are the host
  // it is difficult to connect to the AP while the mesh is running
  // we cannot block the mesh.update but the wifi library needs it's delays
  WiFi.begin(ssidScan, passScan);
  debug2("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    for (int i=0; i<=5; i++){
      delay(500);
      debug2(".");
    }
  }

  debugln2("Connected to the WiFi network");
  debug2("SSID: ");
  debugln2(WiFi.SSID());
  ip = WiFi.localIP();
  debug2("IP Address: ");
  debugln2(ip);
}

//***********************************************************************
//************************ Split up json and post ***********************
/**
 * @brief Split the received JSON in individual measurement so we can POST them to Home Assistant
 * 
 * @param json JSON input we want to split up
 */
void splitJson(String json){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonArray arr = doc.as<JsonArray>();
  // deserialize the string as a JSON object

  if (arr[0].containsKey("bn")){
    const char* baseName = arr[0]["bn"];
    for (int i=0; i<arr.size(); i++) {
      // make a post for every measurement in the JSON
      JsonObject repo = arr[i];
      String unit = repo["u"];
      // units of the measurements
      String value = repo["v"];
      // value we want to post
      String name = repo["n"];
      String ID = String(baseName) + "_";
      // create a name from the sensorname and the name of the measurement
      // like: sensor5_temperature
      post(value, ID + name, unit);
      // post the values
    }
  }
  else{
    debug1("Wrong JSON format received, no message will be send");
  }
}