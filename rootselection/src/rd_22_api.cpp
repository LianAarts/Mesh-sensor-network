#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "rd_22_mesh.h"

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"


StaticJsonDocument<250> jsonDocument;
char buffer[250];

WebServer APIserver(80);
// make our webserver object

void handlePost() {
  if (APIserver.hasArg("plain") == false) {
  }
  String body = APIserver.arg("plain");
  deserializeJson(jsonDocument, body);
  
  debug3("Message has been send to: ");
  debugln3(int(jsonDocument["chipID"]));

  sendSingleMessage("[{\"bn\": \"nameNotAllowed\"}]", int(jsonDocument["chipID"]));
  // we send a mesh message to the node that is in the JSON

  APIserver.send(200, "application/json", "{}");
}

void setup_API(){
  // APIserver.on("/temperature", getTemperature); // example of GET request
  APIserver.on("/usedNameID", HTTP_POST, handlePost);
  // when we receive a POST request with /usedNameID we got to the handlePost routine
  APIserver.begin();
  // start our server when needed (only when root)
  debugln2("API is ready");
}

void apiCheck(){
  APIserver.handleClient();
}

// void getTemperature() {
//   debugln3("Get temperature");
//   APIserver.send(200, "application/json", buffer);
// }