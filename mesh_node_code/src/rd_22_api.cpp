#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#include "rd_22_configuration.h"
#include "rd_22_mesh.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

/**
 * @brief API server object.
 * 
 */
WebServer APIserver(80);
// make our webserver object

/**
 * @brief JSON object with a size of 200 bytes.
 * 
 */
StaticJsonDocument<250> jsonDocument;

/**
 * @brief When we receive a Post request from Home Assistant this function is
 * called
 *
 * This means we have two nodes that are configured with the same name.
 * We will send a single message to the node that needs to clear its name.
 *
 */
void handlePost() {
  if (APIserver.hasArg("plain") == false) {
  }
  String body = APIserver.arg("plain");
  deserializeJson(jsonDocument, body);

  debug3("Message has been send to: ");
  debugln3(int(jsonDocument["chipID"]));

  sendSingleMessage("[{\"bn\": \"nameNotAllowed\"}]",
                    int(jsonDocument["chipID"]));
  // we send a mesh message to the node that is in the JSON

  APIserver.send(200, "application/json", "{}");
}

/**
 * @brief Set up the REST API server we use for the messages from Home Assistant
 * to the mesh network.
 *
 */
void setupAPI() {
  // APIserver.on("/temperature", getTemperature); // example of GET request
  APIserver.on("/usedNameID", HTTP_POST, handlePost);
  // when we receive a POST request with /usedNameID we got to the handlePost
  // routine
  APIserver.begin();
  // start our server when needed (only when root)
  debugln2("API is ready");
}

/**
 * @brief Check the API server
 *
 * @warning This should be done as often as possible
 *
 */
void apiCheck() { APIserver.handleClient(); }

// void getTemperature() {
// debugln3("Get temperature");
// APIserver.send(200, "application/json", buffer);
// }