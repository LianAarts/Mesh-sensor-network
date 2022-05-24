#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

#define token TOKEN_SECR
#define ssidScan SSID_SCAN_SECR
#define passScan PASS_SCAN_SECR
// get the token from the secret.h

/**
 * @brief Http client object.
 * 
 */
HTTPClient http;
// String serverName = "";

/**
 * @brief Wifi client object.
 * 
 */
WiFiClient client;

/**
 * @brief Ip address object
 * 
 */
IPAddress ip;

/**
 * @brief variable will store the measurement data of our POST request to Home Assistant.
 * 
 */
String httpRequestData = "";
/**
 * @brief Variable will store the attributes of our POST request.
 * 
 */
String httpAtributes = "";

/**
 * @brief The response code of our POST request to Home Assistant.
 * Response codes of 200 or 201 are successful.
 * 
 */
int httpResponseCode = 0;

/**
 * @brief Get the IP of the node.
 * 
 * @return String The IP address in String format.
 */
String getIp() { return (ip.toString()); }

//***********************************************************************
//**************************** Post function ****************************

/**
 * @brief Post the measurement to the Home Assistant REST API
 *
 * This will be an individual entity that can display one value.
 * One sensor will need multiple Posts if it measures more than one value.
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

  httpRequestData = "{\"state\":\"" + String(sensorVal);
  httpAtributes = "\", \"attributes\": {\"unit_of_measurement\": \"" + unit;
  // make the json we have to send

  httpResponseCode = http.POST(httpRequestData + httpAtributes + "\"}}");
  // POST the json
  // debug3(sensorVal);
  // debug3("\t");
  debug3("HTTP Response code: ");
  debugln3(httpResponseCode);
  if (httpResponseCode == -1) {
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
 * @brief Measure the RSSI of the Home Assistant access point.
 * 
 * We search the index of the desired network and we measure the RSSI.
 *
 * Received Signal Strength Indicator (RSSI) is the measurement of how strong
 * the signal of a router or access point is to a device. The RSSI is a value
 * between -100 and 0. An RSSI closer to 0 means a better connection.
 * 
 * When there is no connection to the access point we set the value to -100.
 *
 * @return int RSSI, a value between -100 and 0.
 */
int networkScan() {
  int RSSI = -100;

  int numberOfNetworks = WiFi.scanNetworks();
  // Scan all the networks that are nearby
  for (int i = 0; i < numberOfNetworks; i++) {
    // run trough all the networks
    if (strcmp(WiFi.SSID(i).c_str(), ssidScan) == 0) {
      //If we found the desired network we measure the SSID
      // debug2(i);
      debug2("Network name: ");
      debug2(WiFi.SSID(i));
      RSSI = WiFi.RSSI(i);
    }
  }

  debug2("RSSI: ");
  debugln2(RSSI);
  debug2("-----------------------");

  return (RSSI);
  // return the RSSI
}

//***********************************************************************
//*************************** Network connect ***************************
/**
 * @brief Connect to the Home Assistant access point
 *
 * When we are the root we need to send Post requests.
 * We need to connect to the access point to be able to do this.
 *
 */
void setupNetwork() {
  // connect to the AP if we are the host
  // it is difficult to connect to the AP while the mesh is running
  // we cannot block the mesh.update but the wifi library needs it's delays
  WiFi.begin(ssidScan, passScan);
  debug2("Connecting to WiFi..");
  for (int i = 0 ; i < 20 and WiFi.status() != WL_CONNECTED ; i++){
    // we wait for the connection for 10 seconds
    // if we don't have a connection after 10 seconds we are probably not in range
    debug2('.');
    delay(500);  //! why delay (do some research)
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
 * @brief Split the received JSON in individual measurements so we can POST them
 * to Home Assistant
 *
 * @param json JSON input we want to split up
 */
void splitJson(String json) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonArray arr = doc.as<JsonArray>();
  // deserialize the string as a JSON object

  if (arr[0].containsKey("bn")) {
    const char *baseName = arr[0]["bn"];
    for (int i = 0; i < arr.size(); i++) {
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
  } else {
    debug1("Wrong JSON format received, no message will be send");
  }
}